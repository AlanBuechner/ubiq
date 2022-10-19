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
Engine::Ref<Engine::CommandQueue> Engine::Renderer::s_MainCopyCommandQueue;
Engine::Ref<Engine::CommandList> Engine::Renderer::s_MainCopyCommandList;
Engine::RendererAPI Engine::Renderer::s_Api = Engine::RendererAPI::DirectX12;
Engine::InstrumentationTimer Engine::Renderer::s_Timer = CREATE_PROFILEI();

Engine::Ref<Engine::Texture2D> Engine::Renderer::s_WhiteTexture;
Engine::Ref<Engine::Texture2D> Engine::Renderer::s_BlackTexture;
Engine::Ref<Engine::Texture2D> Engine::Renderer::s_BlueTexture;

std::thread Engine::Renderer::s_RenderThread;

Engine::Flag Engine::Renderer::s_RenderFlag;
Engine::Flag Engine::Renderer::s_SwapFlag;
Engine::Flag Engine::Renderer::s_CopyFlag;

namespace Engine
{

	void Renderer::Init()
	{
		CREATE_PROFILE_FUNCTIONI();

		s_Context = GraphicsContext::Create();
		s_Context->Init();
		
		// graphics command queue
		s_MainCommandQueue = CommandQueue::Create(CommandQueue::Type::Direct);
		s_MainCommandList = CommandList::Create();
		s_MainCommandQueue->AddCommandList(s_MainCommandList);

		// copy command queue
		s_MainCopyCommandQueue = CommandQueue::Create(CommandQueue::Type::Direct); // TODO : change to copy
		s_MainCopyCommandList = CommandList::Create(); // TODO : change to copy
		s_MainCopyCommandQueue->AddCommandList(s_MainCopyCommandList);

		Renderer2D::Init();
		LineRenderer::Init();

		s_CopyFlag.Clear();
		s_RenderFlag.Clear();
		s_SwapFlag.Signal();
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
		s_CopyFlag.Wait(false);
		s_CopyFlag.Signal();
		Renderer::GetMainCommandList()->StartRecording();
	}

	void Renderer::EndFrame()
	{
		CREATE_PROFILE_FUNCTIONI();
		WaitForRender();
		WaitForSwap();
		GetMainCommandList()->Close();
		s_RenderFlag.Signal();
		s_SwapFlag.Clear();
	}

	void Renderer::WaitForRender()
	{
		s_RenderFlag.Wait(false);
	}

	void Renderer::WaitForSwap()
	{
		s_SwapFlag.Wait();
	}

	void Renderer::Build()
	{
		Renderer2D::Build();
		LineRenderer::Build();
	}

	void Renderer::Render()
	{
		Instrumentor::Get().RegisterThread("Render", 2);
		InstrumentationTimer timer = CREATE_PROFILEI();
		Ref<ResourceManager> resourceManager = s_Context->GetResourceManager();
		Ref<ResourceDeletionPool> deletionPool = resourceManager->CreateNewDeletionPool();
		while (Application::Get().IsRunning())
		{
			// copy commands
			s_CopyFlag.Wait();
			timer.Start("Copy");
			s_Context->GetResourceManager()->RecordCommands(s_MainCopyCommandList);
			s_MainCopyCommandQueue->Execute();
			timer.End();
			s_CopyFlag.Clear();

			// rendering commands
			s_RenderFlag.Wait();
			timer.Start("Render");
			GetMainCommandQueue()->Execute();
			s_RenderFlag.Clear();
			timer.End();
			
			// swap buffers
			WindowManager::UpdateWindows();

			// prepare for next frame
			deletionPool->Clear();
			deletionPool = resourceManager->CreateNewDeletionPool();
			s_SwapFlag.Signal();
		}
	}

}
