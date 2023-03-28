#include "pch.h"
#include "RenderGraph.h"
#include "GBufferPassNode.h"
#include "MainPassNode.h"
#include "SkyboxNode.h"
#include "ShadowPassNode.h"

namespace Engine
{

	RenderGraph::RenderGraph()
	{
		Ref<FrameBufferNode> renderTargetNode = CreateRef<FrameBufferNode>(*this);
		m_Nodes.push_back(renderTargetNode);

		// gbuffer pass
		Ref<GBufferPassNode> gBufferPass = CreateRef<GBufferPassNode>(*this);
		gBufferPass->SetRenderTarget({ renderTargetNode, renderTargetNode->m_Buffer });
		m_Nodes.push_back(gBufferPass);

		// shadow pass
		Ref<ShadowPassNode> shadowPass = CreateRef<ShadowPassNode>(*this);
		m_Nodes.push_back(shadowPass);

		// skybox pass
		Ref<SkyboxNode> skyboxPass = CreateRef<SkyboxNode>(*this);
		skyboxPass->SetRenderTarget({ gBufferPass, renderTargetNode->m_Buffer });
		m_Nodes.push_back(skyboxPass);

		// main lit pass
		Ref<ShaderPassNode> mainPass = CreateRef<ShaderPassNode>(*this, "main");
		mainPass->SetRenderTarget({ skyboxPass, renderTargetNode->m_Buffer });
		mainPass->AddDependincy(shadowPass);
		m_Nodes.push_back(mainPass);

		// create outputNode
		m_OutputNode = CreateRef<OutputNode>(*this);
		m_OutputNode->m_Buffer = renderTargetNode->m_Buffer;
		m_Nodes.push_back(m_OutputNode);


		m_Order = ExecutionOrder::Create();
		for (auto& node : m_Nodes)
			node->AddToCommandQueue(m_Order);
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
		for (auto& node : m_Nodes)
			node->Invalidate();

		for (auto& node : m_Nodes)
			node->Build();
	}

	Engine::Ref<Engine::FrameBuffer> RenderGraph::GetRenderTarget()
	{
		return m_OutputNode->m_Buffer;
	}

}

