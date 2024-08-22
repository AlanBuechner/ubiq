#include "pch.h"
#include "Renderer.h"
#include "Shaders/Shader.h"
#include "Camera.h"
#include "Renderer2D.h"
#include "DebugRenderer.h"
#include "Abstractions/Resources/ResourceManager.h"
#include "Engine/Core/MeshBuilder.h"
#include "Abstractions/GPUProfiler.h"

#include "EngineResource.h"

Engine::Ref<Engine::GraphicsContext> Engine::Renderer::s_Context;
Engine::Ref<Engine::CommandQueue> Engine::Renderer::s_MainCommandQueue;
Engine::Ref<Engine::CommandList> Engine::Renderer::s_MainCommandList;
Engine::RendererAPI Engine::Renderer::s_Api = Engine::RendererAPI::DirectX12;
Engine::InstrumentationTimer Engine::Renderer::s_Timer = CREATE_PROFILEI();

Engine::Ref<Engine::Texture2D> Engine::Renderer::s_WhiteTexture;
Engine::Ref<Engine::Texture2D> Engine::Renderer::s_BlackTexture;
Engine::Ref<Engine::Texture2D> Engine::Renderer::s_NormalTexture;
Engine::Ref<Engine::Mesh> Engine::Renderer::s_ScreenMesh;
Engine::Ref<Engine::Shader> Engine::Renderer::s_BlitShader;

Engine::Ref<Engine::Material> Engine::Renderer::s_DefultMaterial;

std::thread Engine::Renderer::s_RenderThread;

Engine::Flag Engine::Renderer::s_RenderFlag;
Engine::Flag Engine::Renderer::s_SwapFlag;
Engine::Flag Engine::Renderer::s_CopyFlag;

namespace Engine
{

	void Renderer::Init()
	{
		CREATE_PROFILE_FUNCTIONI();

		// Init GPUProfiling
		GPUProfiler::Init();

		s_Context = GraphicsContext::Create();
		s_Context->Init();
		
		// graphics command queue
		s_MainCommandQueue = CommandQueue::Create(CommandQueue::Type::Direct);
		s_MainCommandList = CommandList::Create(CommandList::Direct);

		// copy command queue

		Renderer2D::Init();
		DebugRenderer::Init();

		s_CopyFlag.Clear();
		s_RenderFlag.Clear();
		s_SwapFlag.Signal();
		s_RenderThread = std::thread(&Renderer::Render);

		uint32 textureData;

		s_WhiteTexture = Texture2D::Create(1, 1);
		textureData = 0xffffffff; // 1,1,1,1
		s_WhiteTexture->SetData(&textureData);

		s_BlackTexture = Texture2D::Create(1, 1);
		textureData = 0xff0000ff; // 1,0,0,1
		s_BlackTexture->SetData(&textureData);

		s_NormalTexture = Texture2D::Create(1, 1);
		textureData = 0xffff8080; // 0,0,1,1
		s_NormalTexture->SetData(&textureData);

		struct Vertex
		{
			Math::Vector3 position;
			Math::Vector2 UV;
		};

		TMeshBuilder<Vertex> meshBuilder;
		meshBuilder.vertices.push_back({ { -1,-1,0 }, { 0, 1 } });
		meshBuilder.vertices.push_back({ {  1,-1,0 }, { 1, 1 } });
		meshBuilder.vertices.push_back({ { -1, 1,0 }, { 0, 0 } });
		meshBuilder.vertices.push_back({ {  1, 1,0 }, { 1, 0 } });

		meshBuilder.indices.push_back(0);
		meshBuilder.indices.push_back(3);
		meshBuilder.indices.push_back(1);
		meshBuilder.indices.push_back(0);
		meshBuilder.indices.push_back(2);
		meshBuilder.indices.push_back(3);

		meshBuilder.Apply();

		s_ScreenMesh = meshBuilder.mesh;

		s_BlitShader = Shader::CreateFromEmbeded(BLIT, "Blit.hlsl");
	}

	void Renderer::Destroy()
	{
		s_WhiteTexture = nullptr;
		s_BlackTexture = nullptr;
		s_NormalTexture = nullptr;
		s_ScreenMesh = nullptr;
		s_DefultMaterial = nullptr;

		s_RenderThread.join();
		Renderer2D::Destroy();
		DebugRenderer::Destroy();

		s_MainCommandList = nullptr;
		s_MainCommandQueue = nullptr;

		s_Context.reset(); // destroy context before atexit
	}

	void Renderer::BeginFrame()
	{
		CREATE_PROFILE_FUNCTIONI();
		s_CopyFlag.Wait(false);
		s_CopyFlag.Signal();
		
		Renderer::GetMainCommandList()->StartRecording();
		GPUTimer::BeginEvent(s_MainCommandList, "MainCommandList");
	}

	void Renderer::EndFrame()
	{
		CREATE_PROFILE_FUNCTIONI();
		WaitForRender();
		WaitForSwap();
		GPUTimer::EndEvent(s_MainCommandList);
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

	void Renderer::Build(Ref<CommandList> commandList)
	{
		Renderer2D::Build(commandList);
		DebugRenderer::Build(commandList);
	}

	void Renderer::Render()
	{
		Instrumentor::Get().RegisterThread("Render", 1);
		InstrumentationTimer timer = CREATE_PROFILEI();
		Ref<ResourceManager> resourceManager = s_Context->GetResourceManager();
		ResourceDeletionPool* deletionPool = resourceManager->CreateNewDeletionPool();
		static int frame = 0;
		while (Application::Get().IsRunning())
		{
			// copy commands
			s_CopyFlag.Wait();
			timer.Start("Copy");
			GPUProfiler::StartFrame();
			resourceManager->UploadData();
			timer.End();
			s_CopyFlag.Clear();

			// rendering commands
			s_RenderFlag.Wait();
			//CORE_INFO("{0}", frame);
			timer.Start("Render");
			s_MainCommandQueue->PrependSubmit(resourceManager->GetUploadCommandLists());
			s_MainCommandQueue->Submit(s_MainCommandList);
			s_MainCommandQueue->Build();
#if defined(RELEASE)
			try 
			{
				s_MainCommandQueue->Execute();
			}
			catch (...)
			{
				CORE_ERROR("Failed To Render the Scene");
			}
#else
			s_MainCommandQueue->Execute();
#endif
			s_MainCommandQueue->Await();
			s_RenderFlag.Clear();
			GPUProfiler::EndFrame();
			timer.End();
			
			// swap buffers
			WindowManager::UpdateWindows();

			// prepare for next frame
			resourceManager->Clean();
			deletionPool->Clear();
			deletionPool = resourceManager->CreateNewDeletionPool();
			s_SwapFlag.Signal();

			frame++;
		}
	}

}
