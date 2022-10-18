#include "pch.h"
#include "Renderer.h"
#include "Shader.h"
#include "Camera.h"
#include "Renderer2D.h"
#include "LineRenderer.h"
#include "ResourceManager.h"

Engine::Ref<Engine::GraphicsContext> Engine::Renderer::s_Context;
Engine::Ref<Engine::CommandQueue> Engine::Renderer::s_MainCommandQueue;
Engine::Ref<Engine::CommandList> Engine::Renderer::s_MainCommandList;
Engine::Ref<Engine::CommandList> Engine::Renderer::s_MainCopyCommandList;
Engine::RendererAPI Engine::Renderer::s_Api = Engine::RendererAPI::DirectX12;
Engine::InstrumentationTimer Engine::Renderer::s_Timer = CREATE_PROFILEI();

Engine::Ref<Engine::Texture2D> Engine::Renderer::s_WhiteTexture;
Engine::Ref<Engine::Texture2D> Engine::Renderer::s_BlackTexture;
Engine::Ref<Engine::Texture2D> Engine::Renderer::s_BlueTexture;

std::thread Engine::Renderer::s_RenderThread;

Engine::Flag Engine::Renderer::s_RenderFlag;
Engine::Flag Engine::Renderer::s_SwapFlag;
Engine::Flag Engine::Renderer::s_RecordFlag;

namespace Engine
{
	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

	void Renderer::Init()
	{
		CREATE_PROFILE_FUNCTIONI();

		s_Context = GraphicsContext::Create();
		s_Context->Init();
		s_MainCommandQueue = CommandQueue::Create(CommandQueue::Type::Direct);
		s_MainCommandList = CommandList::Create();
		s_MainCopyCommandList = CommandList::Create();

		s_MainCommandQueue->AddCommandList(s_MainCopyCommandList);
		s_MainCommandQueue->AddCommandList(s_MainCommandList, { s_MainCopyCommandList });

		Renderer2D::Init();
		LineRenderer::Init();

		m_SceneData->PointLights.reserve(MAX_LIGHTS);

		s_RenderFlag.Clear();
		s_SwapFlag.Signal();
		s_RecordFlag.Signal();
		s_RenderThread = std::thread(&Renderer::Render);

		uint32 textureData;

		s_WhiteTexture = Texture2D::Create(1, 1);
		textureData = 0xffffffff; // 1,1,1,1
		s_WhiteTexture->SetData(&textureData);

		s_BlackTexture = Texture2D::Create(1, 1);
		textureData = 0x000000ff; // 0,0,0,1
		s_BlackTexture->SetData(&textureData);

		s_BlueTexture = Texture2D::Create(1, 1);
		textureData = 0x0000ffff; // 0,0,1,1
		s_BlueTexture->SetData(&textureData);
	}

	void Renderer::Destroy()
	{
		s_WhiteTexture = nullptr;
		s_BlackTexture = nullptr;
		s_BlueTexture = nullptr;

		s_RenderThread.join();
		Renderer2D::Destroy();
		LineRenderer::Destroy();
	}

	void Renderer::BeginFrame()
	{
		CREATE_PROFILE_FUNCTIONI();
		s_RecordFlag.Wait();
		Renderer::GetMainCommandList()->StartRecording();
	}

	void Renderer::EndFrame()
	{
		CREATE_PROFILE_FUNCTIONI();
		WaitForRender();
		s_Context->GetResourceManager()->RecordCommands(s_MainCopyCommandList);
		WaitForSwap();
		GetMainCommandList()->Close();
		s_RenderFlag.Signal();
		s_SwapFlag.Clear();
		s_RecordFlag.Clear();
	}

	void Renderer::WaitForRender()
	{
		s_RenderFlag.Wait(false);
	}

	void Renderer::WaitForSwap()
	{
		s_SwapFlag.Wait();
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		m_SceneData->ViewMatrix = camera.GetViewMatrix();
		m_SceneData->ProjectionMatrix = camera.GetProjectionMatrix();
	}

	void Renderer::BeginScene(const Camera& camera, const Math::Mat4& transform)
	{
		m_SceneData->ViewMatrix = transform;
		m_SceneData->ProjectionMatrix = camera.GetProjectionMatrix();
	}

	void Renderer::EndScene()
	{
		m_SceneData->PointLights.clear();
		m_SceneData->PointLights.reserve(MAX_LIGHTS);
	}

	void Renderer::Build()
	{
		Renderer2D::Build();
		LineRenderer::Build();
	}

	void Renderer::SubmitPointLight(const PointLight& light)
	{
		if (m_SceneData->PointLights.size() >= MAX_LIGHTS)
			return;
		m_SceneData->PointLights.push_back(light);
	}

	void Renderer::WaitForRenderFlag()
	{
		s_RenderFlag.Wait();
	}

	void Renderer::Render()
	{
		Instrumentor::Get().RegisterThread("Render", 2);
		InstrumentationTimer timer = CREATE_PROFILEI();
		Ref<ResourceManager> resourceManager = s_Context->GetResourceManager();
		Ref<ResourceDeletionPool> deletionPool = resourceManager->CreateNewDeletionPool();
		while (Application::Get().IsRunning())
		{
			WaitForRenderFlag();
			timer.Start("Render");
			GetMainCommandQueue()->Execute();
			s_RecordFlag.Signal();
			s_RenderFlag.Clear();
			timer.End();
			
			WindowManager::UpdateWindows();
			deletionPool->Clear();
			deletionPool = resourceManager->CreateNewDeletionPool();
			s_SwapFlag.Signal();
		}
	}

}
