#include "pch.h"
#include "RenderGraph.h"
#include "MainPassNode.h"

namespace Engine
{

	RenderGraph::RenderGraph()
	{
		// create outputNode
		m_OutputNode = CreateRef<OutputNode>(*this);
		m_Nodes.push_back(m_OutputNode);

		Ref<ShaderPassNode> depthPass = CreateRef<ShaderPassNode>(*this, "depth");
		depthPass->SetRenderTarget({ m_OutputNode, m_OutputNode->m_Buffer });
		m_Nodes.push_back(depthPass);

		Ref<ShaderPassNode> mainPass = CreateRef<ShaderPassNode>(*this, "main");
		mainPass->SetRenderTarget({ depthPass, m_OutputNode->m_Buffer });
		m_Nodes.push_back(mainPass);
	}

	RenderGraph::~RenderGraph()
	{
		for (Ref<RenderGraphNode> node : m_Nodes)
		{
			for (Ref<CommandList> cmdList : node->GetCommandLists())
				Renderer::GetMainCommandQueue()->RemoveCommandLIst(cmdList);
		}
	}

	void RenderGraph::AddToCommandQueue()
	{
		for (auto& node : m_Nodes)
			node->AddToCommandQueue();
	}

	void RenderGraph::OnViewportResize(uint32 width, uint32 height)
	{
		for (auto& node : m_Nodes)
			node->OnViewportResize(width, height);
	}

	void RenderGraph::Build()
	{
		m_FrameBufferStates.clear();
		for (auto& node : m_Nodes)
			node->Invalidate();

		for (auto& node : m_Nodes)
			node->Build();
	}

	Engine::Ref<Engine::FrameBuffer> RenderGraph::GetRenderTarget()
	{
		return m_OutputNode->m_Buffer;
	}

	void RenderGraph::RecoardFrameBufferState(FrameBufferState state)
	{
		for (auto& fbState : m_FrameBufferStates)
		{
			if (fbState.buffer == state.buffer)
			{
				fbState.afterState = state.afterState;
				return;
			}
		}

		m_FrameBufferStates.push_back(state);
	}

	void RenderGraph::UpdateStates()
	{
		for (auto& fbState : m_FrameBufferStates)
			fbState.buffer->SetState(fbState.afterState);
	}

}

