#include "pch.h"
#include "Renderer.h"
#include "Shader.h"
#include "Camera.h"
#include "Renderer2D.h"
#include "LineRenderer.h"
#include "ResourceManager.h"
#include "Engine/Core/MeshBuilder.h"
#include "GPUProfiler.h"

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
Engine::Ref<Engine::Mesh> Engine::Renderer::s_ScreenMesh;
Engine::Ref<Engine::ComputeShader> Engine::Renderer::s_BlitShader;

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

		// copy command queue
		s_MainCopyCommandQueue = CommandQueue::Create(CommandQueue::Type::Direct);
		s_MainCopyCommandList = CommandList::Create();
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

		s_ScreenMesh = meshBuilder.ToMesh();

		s_BlitShader = ComputeShader::Create("Assets/Shaders/Blit.compute");
	}

	void Renderer::Destroy()
	{
		s_WhiteTexture = nullptr;
		s_BlackTexture = nullptr;
		s_BlueTexture = nullptr;
		s_ScreenMesh = nullptr;

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
		LineRenderer::Build(commandList);
	}

	void Renderer::Render()
	{
		Instrumentor::Get().RegisterThread("Render", 1);
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
			s_MainCommandQueue->Execute();
			s_MainCommandQueue->ExecuteImmediate({s_MainCommandList});
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
