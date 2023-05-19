#include "pch.h"
#include "RenderGraphNode.h"
#include "Engine/Core/Application.h"
#include "Engine/Core/Window.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/CommandList.h"

namespace Engine
{
	RenderGraphNode::RenderGraphNode(RenderGraph& graph) :
		m_Graph(graph)
	{}

	void RenderGraphNode::Build()
	{

		if (!m_Built)
		{
			for (auto de : m_Dependincys)
				de->Build();
			BuildImpl();
		}
		m_Built = true;
	}


	// output node
	OutputNode::OutputNode(RenderGraph& graph) :
		RenderGraphNode(graph)
	{}

	void OutputNode::Invalidate()
	{
		RenderGraphNode::Invalidate();
	}

	// frame buffer node
	FrameBufferNode::FrameBufferNode(RenderGraph& graph, const FrameBufferSpecification& fbSpec) :
		RenderGraphNode(graph)
	{
		m_Buffer = FrameBuffer::Create(fbSpec);
	}


	void FrameBufferNode::OnViewportResize(uint32 width, uint32 height)
	{
		m_Buffer->Resize(width, height);
	}

	TransitionNode::TransitionNode(RenderGraph& graph) :
		RenderGraphNode(graph)
	{}

	void TransitionNode::AddBuffer(const CommandList::FBTransitionObject& transition)
	{
		m_Transitions.push_back(transition);
	}

	void TransitionNode::BuildImpl()
	{
		m_CommandList->Transition(m_Transitions);
	}

}

