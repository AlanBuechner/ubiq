#include "pch.h"
#include "RenderGraph.h"

#include "Engine/Core/Window.h"
#include "Engine/Core/Application.h"

#include "Engine/Renderer/Resources/FrameBuffer.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Resources/InstanceBuffer.h"
#include "Engine/Renderer/Resources/ConstantBuffer.h"
#include "Engine/Renderer/Resources/Texture.h"
#include "Engine/Renderer/Light.h"
#include "Engine/Renderer/CommandList.h"
#include "Engine/Renderer/CommandQueue.h"
#include "Engine/Renderer/Camera.h"

// nodes
#include "GBufferPassNode.h"
#include "MainPassNode.h"
#include "SkyboxNode.h"
#include "ShadowPassNode.h"
#include "PostProcessNode.h"

// post process
#include "Engine/Renderer/PostProcessing/Bloom.h"
#include "Engine/Renderer/PostProcessing/DepthOfField.h"
#include "Engine/Renderer/PostProcessing/ToneMapping.h"

namespace Engine
{

	RenderGraph::RenderGraph()
	{
		Window& window = Application::Get().GetWindow();

		Ref<CommandList> commandList = CommandList::Create(CommandList::Direct);;
		m_CommandLists.push_back(commandList);

		float clearval = pow(0.2f, 2.2);
		Ref<FrameBufferNode> renderTargetNode = CreateRef<FrameBufferNode>(*this, std::vector<Ref<RenderTarget2D>>{
			RenderTarget2D::Create(window.GetWidth(), window.GetHeight(), 1, TextureFormat::RGBA16),
			RenderTarget2D::Create(window.GetWidth(), window.GetHeight(), 1, TextureFormat::Depth),
		});
		m_Nodes.push_back(renderTargetNode);

		// set frame buffer to render target
		Ref<TransitionNode> t1 = CreateRef<TransitionNode>(*this);
		t1->SetCommandList(commandList);
		t1->AddBuffer({ renderTargetNode->m_Buffer->GetAttachment(0)->GetResourceHandle(), ResourceState::RenderTarget });
		t1->AddBuffer({ renderTargetNode->m_Buffer->GetAttachment(1)->GetResourceHandle(), ResourceState::RenderTarget });
		m_Nodes.push_back(t1);

		// shadow pass
		Ref<ShadowPassNode> shadowPass = CreateRef<ShadowPassNode>(*this);
		shadowPass->SetCommandList(commandList);
		m_Nodes.push_back(shadowPass);

		//// gbuffer pass
		//Ref<GBufferPassNode> gBufferPass = CreateRef<GBufferPassNode>(*this);
		//gBufferPass->SetCommandList(commandList);
		//gBufferPass->SetRenderTarget(renderTargetNode->m_Buffer);
		//gBufferPass->AddDependincy(t1);
		//m_Nodes.push_back(gBufferPass);

		//// skybox pass
		//Ref<SkyboxNode> skyboxPass = CreateRef<SkyboxNode>(*this);
		//skyboxPass->SetCommandList(commandList);
		//skyboxPass->SetRenderTarget(renderTargetNode->m_Buffer);
		//skyboxPass->AddDependincy(gBufferPass);
		//m_Nodes.push_back(skyboxPass);

		//// main lit pass
		//Ref<ShaderPassNode> mainPass = CreateRef<ShaderPassNode>(*this, "lit");
		//mainPass->SetCommandList(commandList);
		//mainPass->SetRenderTarget(renderTargetNode->m_Buffer);
		//mainPass->AddDependincy(shadowPass);
		//mainPass->AddDependincy(skyboxPass);
		//m_Nodes.push_back(mainPass);

		//// create post processing render target
		//Ref<FrameBufferNode> postRenderTargetNode = CreateRef<FrameBufferNode>(*this, std::vector<Ref<RenderTarget2D>>{
		//	RenderTarget2D::Create(window.GetWidth(), window.GetHeight(), 1, TextureFormat::RGBA16),
		//});
		//m_Nodes.push_back(postRenderTargetNode);

		//// set frame buffer to srv for use in post processing
		//Ref<TransitionNode> t2 = CreateRef<TransitionNode>(*this);
		//t2->SetCommandList(commandList);
		//t2->AddBuffer({ postRenderTargetNode->m_Buffer->GetAttachment(0)->GetResourceHandle(), ResourceState::RenderTarget });
		//t2->AddBuffer({ renderTargetNode->m_Buffer->GetAttachment(0)->GetResourceHandle(), ResourceState::Common });
		//t2->AddBuffer({ renderTargetNode->m_Buffer->GetAttachment(1)->GetResourceHandle(), ResourceState::Common });
		//t2->AddDependincy(mainPass);
		//m_Nodes.push_back(t2);

		//// post processing
		//PostProcessInput input;
		//input.m_TextureHandles["Color Buffer"] = renderTargetNode->m_Buffer->GetAttachment(0);
		//input.m_TextureHandles["Depth Buffer"] = renderTargetNode->m_Buffer->GetAttachment(1);

		//Ref<PostProcessNode> postPass = CreateRef<PostProcessNode>(*this);
		//postPass->SetCommandList(commandList);
		//postPass->SetRenderTarget(postRenderTargetNode->m_Buffer->GetAttachment(0));
		//postPass->SetInput(input);
		//postPass->AddPostProcess(CreateRef<DepthOfField>());
		////postPass->AddPostProcess(CreateRef<Bloom>());
		//postPass->AddPostProcess(CreateRef<ToneMapping>());
		//postPass->SetSrc(renderTargetNode->m_Buffer->GetAttachment(0));
		//postPass->InitPostProcessStack();
		//postPass->AddDependincy(t2);
		//m_Nodes.push_back(postPass);

		// create outputNode
		m_OutputNode = CreateRef<OutputNode>(*this);
		m_OutputNode->SetCommandList(commandList);
		m_OutputNode->m_Buffer = FrameBuffer::Create({
			renderTargetNode->m_Buffer->GetAttachment(0),
			//postRenderTargetNode->m_Buffer->GetAttachment(0),
			renderTargetNode->m_Buffer->GetAttachment(1),
		});
		m_Nodes.push_back(m_OutputNode);

		m_Order = ExecutionOrder::Create();
		m_Order->Add(commandList);
	}

	RenderGraph::~RenderGraph()
	{
	}

	void RenderGraph::Submit(Ref<CommandQueue> queue)
	{
		queue->Submit(m_Order);
	}

	void RenderGraph::OnViewportResize(uint32 width, uint32 height)
	{
		for (auto& node : m_Nodes)
			node->OnViewportResize(width, height);
	}

	void RenderGraph::Build()
	{
		for (Ref<CommandList> list : m_CommandLists)
			list->StartRecording();

		for (auto& node : m_Nodes)
			node->Invalidate();

		for (auto& node : m_Nodes)
			node->Build();

		for (Ref<CommandList> list : m_CommandLists)
			list->Close();
	}

	Engine::Ref<Engine::FrameBuffer> RenderGraph::GetRenderTarget()
	{
		return m_OutputNode->m_Buffer;
	}

}

