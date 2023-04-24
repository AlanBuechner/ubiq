#include "pch.h"
#include "RenderGraph.h"

// nodes
#include "GBufferPassNode.h"
#include "MainPassNode.h"
#include "SkyboxNode.h"
#include "ShadowPassNode.h"
#include "PostProcessNode.h"

// post process
#include "Engine/Renderer/PostProcessing/Bloom.h"

namespace Engine
{

	RenderGraph::RenderGraph()
	{
		Window& window = Application::Get().GetWindow();

		FrameBufferSpecification fbSpec1;
		fbSpec1.Attachments = {
			{ FrameBufferTextureFormat::RGBA16, {0.1f,0.1f,0.1f,1} },
			{ FrameBufferTextureFormat::Depth, { 1,0,0,0 } }
		};
		fbSpec1.InitalState = FrameBufferState::Common;

		fbSpec1.Width = window.GetWidth();
		fbSpec1.Height = window.GetHeight();

		Ref<FrameBufferNode> renderTargetNode = CreateRef<FrameBufferNode>(*this, fbSpec1);
		m_Nodes.push_back(renderTargetNode);

		Ref<CommandList> commandList = CommandList::Create(CommandList::Direct);
		m_CommandLists.push_back(commandList);

		// set frame buffer to render target
		Ref<TransitionNode> t1 = CreateRef<TransitionNode>(*this);
		t1->SetCommandList(commandList);
		t1->AddBuffer({ renderTargetNode->m_Buffer, FrameBufferState::RenderTarget, FrameBufferState::Common });
		m_Nodes.push_back(t1);

		// shadow pass
		Ref<ShadowPassNode> shadowPass = CreateRef<ShadowPassNode>(*this);
		shadowPass->SetCommandList(commandList);
		m_Nodes.push_back(shadowPass);

		// gbuffer pass
		Ref<GBufferPassNode> gBufferPass = CreateRef<GBufferPassNode>(*this);
		gBufferPass->SetCommandList(commandList);
		gBufferPass->SetRenderTarget(renderTargetNode->m_Buffer);
		gBufferPass->AddDependincy(t1);
		m_Nodes.push_back(gBufferPass);

		// skybox pass
		Ref<SkyboxNode> skyboxPass = CreateRef<SkyboxNode>(*this);
		skyboxPass->SetCommandList(commandList);
		skyboxPass->SetRenderTarget(renderTargetNode->m_Buffer);
		skyboxPass->AddDependincy(gBufferPass);
		m_Nodes.push_back(skyboxPass);

		// main lit pass
		Ref<ShaderPassNode> mainPass = CreateRef<ShaderPassNode>(*this, "lit");
		mainPass->SetCommandList(commandList);
		mainPass->SetRenderTarget(renderTargetNode->m_Buffer);
		mainPass->AddDependincy(shadowPass);
		mainPass->AddDependincy(skyboxPass);
		m_Nodes.push_back(mainPass);

		// create post processing render target
		FrameBufferSpecification fbSpec2;
		fbSpec2.Attachments = {
			{ FrameBufferTextureFormat::RGBA16, {0.1f,0.1f,0.1f,1} },
			{ FrameBufferTextureFormat::Depth, { 1,0,0,0 } }
		};
		fbSpec2.InitalState = FrameBufferState::Common;
		fbSpec2.Width = window.GetWidth();
		fbSpec2.Height = window.GetHeight();

		Ref<FrameBufferNode> postRenderTargetNode = CreateRef<FrameBufferNode>(*this, fbSpec2);
		m_Nodes.push_back(postRenderTargetNode);

		// set frame buffer to srv for use in post processing
		Ref<TransitionNode> t2 = CreateRef<TransitionNode>(*this);
		t2->SetCommandList(commandList);
		t2->AddBuffer({ renderTargetNode->m_Buffer, FrameBufferState::Common, FrameBufferState::RenderTarget });
		t2->AddBuffer({ postRenderTargetNode->m_Buffer, FrameBufferState::RenderTarget, FrameBufferState::Common });
		t2->AddDependincy(mainPass);
		m_Nodes.push_back(t2);

		// post processing
		PostProcessInput input;
		input.m_TextureHandles["Color Buffer"] = renderTargetNode->m_Buffer->GetAttachmentShaderDescriptoLocation(0);
		input.m_TextureHandles["Depth Buffer"] = renderTargetNode->m_Buffer->GetAttachmentShaderDescriptoLocation(1);

		Ref<PostProcessNode> postPass = CreateRef<PostProcessNode>(*this);
		postPass->SetCommandList(commandList);
		postPass->SetRenderTarget(postRenderTargetNode->m_Buffer );
		postPass->SetInput(input);
		postPass->AddPostProcess(CreateRef<Bloom>());
		postPass->SetSrc(renderTargetNode->m_Buffer->GetAttachmentShaderDescriptoLocation(0));
		postPass->InitPostProcessStack();
		postPass->AddDependincy(t2);
		m_Nodes.push_back(postPass);

		// create outputNode
		m_OutputNode = CreateRef<OutputNode>(*this);
		m_OutputNode->m_Buffer = postRenderTargetNode->m_Buffer;
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

