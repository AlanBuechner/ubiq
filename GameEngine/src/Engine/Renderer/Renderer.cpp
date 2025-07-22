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

#include "Engine/Core/Application.h"

Engine::Ref<Engine::GraphicsContext> Engine::Renderer::s_Context;
Engine::Ref<Engine::CommandQueue> Engine::Renderer::s_MainCommandQueue;
Engine::Ref<Engine::CPUCommandList> Engine::Renderer::s_MainCommandList;
Engine::Ref<Engine::CPUCommandList> Engine::Renderer::s_UploadCommandList;
Engine::RendererAPI Engine::Renderer::s_Api = Engine::RendererAPI::DirectX12;
Profiler::InstrumentationTimer Engine::Renderer::s_Timer = CREATE_PROFILEI();

Engine::Ref<Engine::Texture2D> Engine::Renderer::s_WhiteTexture;
Engine::Ref<Engine::Texture2D> Engine::Renderer::s_BlackTexture;
Engine::Ref<Engine::Texture2D> Engine::Renderer::s_NormalTexture;
Engine::Ref<Engine::Mesh> Engine::Renderer::s_ScreenMesh;
Engine::Ref<Engine::Shader> Engine::Renderer::s_BlitShader;

Engine::Ref<Engine::Material> Engine::Renderer::s_DefultMaterial;

std::atomic<Engine::Renderer::FrameContext*> Engine::Renderer::s_FrameContext = nullptr;
Engine::NamedJobThread* Engine::Renderer::s_RenderThread;
Engine::NamedJobThread* Engine::Renderer::s_GPUThread;

namespace Engine
{

	Renderer::FrameContext::FrameContext()
	{
		CREATE_PROFILE_FUNCTIONI();
		m_DeletionPool = new ResourceDeletionPool();
		m_UploadPool = new UploadPool();
	}

	Renderer::FrameContext::~FrameContext()
	{
		CREATE_PROFILE_FUNCTIONI();
		hasBeenDeleted = true;
		delete m_DeletionPool;
		delete m_UploadPool;

		m_DeletionPool = nullptr;
		m_UploadPool = nullptr;

		for (uint32 i = 0; i < m_Commands.Count(); i++)
			delete m_Commands[i];

	}

	void Renderer::Init()
	{
		CREATE_PROFILE_FUNCTIONI();

		s_FrameContext = new FrameContext();

		// Init GPUProfiling
		GPUProfiler::Init();

		s_Context = GraphicsContext::Create();
		s_Context->Init();
		
		// graphics command queue
		s_MainCommandQueue = CommandQueue::Create(CommandQueue::Type::Direct);
		s_MainCommandList = CPUCommandList::Create(CommandListType::Graphics);
		s_UploadCommandList = CPUCommandList::Create(CommandListType::Graphics);

		s_MainCommandList->SetName("Main Command List");
		s_UploadCommandList->SetName("Upload Command List");

		// copy command queue

		Renderer2D::Init();
		DebugRenderer::Init();

		s_RenderThread = JobSystem::AddNamedJob("Render Thread", &Renderer::RenderThread);
		s_GPUThread = JobSystem::AddNamedJob("GPU Thread", &Renderer::GPUThread);

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
		meshBuilder.vertices.Push({ { -1,-1,0 }, { 0, 1 } });
		meshBuilder.vertices.Push({ {  1,-1,0 }, { 1, 1 } });
		meshBuilder.vertices.Push({ { -1, 1,0 }, { 0, 0 } });
		meshBuilder.vertices.Push({ {  1, 1,0 }, { 1, 0 } });

		meshBuilder.indices.Push(0);
		meshBuilder.indices.Push(3);
		meshBuilder.indices.Push(1);
		meshBuilder.indices.Push(0);
		meshBuilder.indices.Push(2);
		meshBuilder.indices.Push(3);

		meshBuilder.Apply();

		s_ScreenMesh = meshBuilder.mesh;

		s_BlitShader = Application::Get().GetAssetManager().GetEmbededAsset<Shader>(BLITSHADER);
	}

	void Renderer::Destroy()
	{
		s_WhiteTexture.reset();
		s_BlackTexture.reset();
		s_NormalTexture.reset();
		s_ScreenMesh.reset();
		s_DefultMaterial.reset();

		WaitForRender();
		Renderer2D::Destroy();
		DebugRenderer::Destroy();

		s_MainCommandList.reset();
		s_MainCommandQueue.reset();

		s_Context.reset(); // destroy context before atexit
	}

	void Renderer::BeginFrame()
	{
		CREATE_PROFILE_FUNCTIONI();
		
		s_MainCommandList->StartRecording();
		GPUTimer::BeginEvent(s_MainCommandList, "MainCommandList");
	}

	void Renderer::EndFrame()
	{
		CREATE_PROFILE_FUNCTIONI();
		GPUTimer::EndEvent(s_MainCommandList);
		s_MainCommandList->StopRecording();
		SubmitCommandList(s_MainCommandList);
		FrameContext* context = s_FrameContext.exchange(new FrameContext());
		s_RenderThread->Invoke(context);
	}

	void Renderer::WaitForRender()
	{
		s_RenderThread->Wait();
		s_GPUThread->Wait();
	}

	void Renderer::Build(Ref<CPUCommandList> commandList)
	{
		Renderer2D::Build(commandList);
		DebugRenderer::Build(commandList);
	}

	void Renderer::RenderThread(void* data)
	{
		CREATE_PROFILE_SCOPEI("Render Frame");
		FrameContext* frameContext = (FrameContext*)data;

		// copy commands
		frameContext->m_UploadPool->RecoredUploadCommands(s_UploadCommandList);
		frameContext->m_Commands.Insert(0u, s_UploadCommandList->TakeAllocator()); // add upload command list

		{ // create commands for resource state tracking
			CREATE_PROFILE_SCOPEI("Prepare Command Lists");
			CPUCommandAllocator::ResourceStateMap stateMap;
			for (CPUCommandAllocator* commands : frameContext->m_Commands)
			{
				commands->PrependResourceStateCommands(stateMap);
				commands->MergeResourceStatesInto(stateMap);
			}
		}

		{ // record commands
			CREATE_PROFILE_SCOPEI("Recored GPU Command Lists");
			for (CPUCommandAllocator* commands : frameContext->m_Commands)
			{
				Ref<CommandList> commandList = CommandList::Create(CommandListType::Graphics);
				frameContext->m_CommandLists.Push(commandList);

				commandList->StartRecording();
				commandList->RecoredCommands(commands);
				commandList->Close();
			}
		}

		if (frameContext->m_IMGUICommandList)
			frameContext->m_CommandLists.Push(frameContext->m_IMGUICommandList);

		s_GPUThread->Invoke(frameContext);
	}

	void Renderer::GPUThread(void* data)
	{
		CREATE_PROFILE_SCOPEI("Render Frame");
		FrameContext* frameContext = (FrameContext*)data;

		// get command lists
		Utils::Vector<Ref<CommandList>>& commandLists = frameContext->m_CommandLists;

		// execute commands
		GPUProfiler::StartFrame();
		s_MainCommandQueue->Submit(commandLists);
		s_MainCommandQueue->Execute();
		s_MainCommandQueue->Await();
		GPUProfiler::EndFrame();

		// free command lists
		for (uint32 i = 0; i < commandLists.Count(); i++)
		{
			Ref<CommandList> commandList = commandLists[i];
			CommandList::Free(commandList);
			commandLists[i] = nullptr;
		}
		frameContext->m_IMGUICommandList = nullptr;

		// swap buffers
		WindowManager::UpdateWindows();

		// prepare for next frame
		delete frameContext;
	}

}
