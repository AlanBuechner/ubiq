#include "pch.h"
#include "RenderGraph.h"
#include "GBufferPassNode.h"
#include "MainPassNode.h"
#include "SkyboxNode.h"
#include "ShadowPassNode.h"
#include "PostProcessNode.h"

namespace Engine
{

	RenderGraph::RenderGraph()
	{
		Ref<FrameBufferNode> renderTargetNode = CreateRef<FrameBufferNode>(*this);
		m_Nodes.push_back(renderTargetNode);

		Ref<CommandList> commandList = CommandList::Create(CommandList::Direct);
		m_CommandLists.push_back(commandList);

		// gbuffer pass
		Ref<GBufferPassNode> gBufferPass = CreateRef<GBufferPassNode>(*this);
		gBufferPass->SetCommandList(commandList);
		gBufferPass->SetRenderTarget(renderTargetNode->m_Buffer);
		m_Nodes.push_back(gBufferPass);

		// shadow pass
		Ref<ShadowPassNode> shadowPass = CreateRef<ShadowPassNode>(*this);
		shadowPass->SetCommandList(commandList);
		m_Nodes.push_back(shadowPass);

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

		// post processing
		Ref<FrameBufferNode> postRenderTargetNode = CreateRef<FrameBufferNode>(*this);
		m_Nodes.push_back(renderTargetNode);

		PostProcessInput input;
		input.m_TextureHandles["Color Buffer"] = renderTargetNode->m_Buffer->GetAttachmentShaderHandle(0);
		input.m_TextureHandles["Depth Buffer"] = renderTargetNode->m_Buffer->GetAttachmentShaderHandle(1);

		Ref<PostProcessNode> postPass = CreateRef<PostProcessNode>(*this);
		postPass->SetCommandList(commandList);
		postPass->SetRenderTarget(postRenderTargetNode->m_Buffer );
		postPass->SetInput(input);
		postPass->AddDependincy(mainPass);
		m_Nodes.push_back(postPass);

		// create outputNode
		m_OutputNode = CreateRef<OutputNode>(*this);
		m_OutputNode->m_Buffer = renderTargetNode->m_Buffer;
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

