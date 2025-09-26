#include "pch.h"
#include "RenderGraphNode.h"
#include "Engine/Core/Application.h"
#include "Engine/Core/Window.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Abstractions/CommandList.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"

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

	void OutputNode::BuildImpl()
	{
		GPUTimer::BeginEvent(m_CommandList, "Output Node");

		Utils::Vector<ResourceStateObject> transitions(m_Buffer->GetAttachments().Count());
		for (uint32 i = 0; i < transitions.Count(); i++)
			transitions[i] = { m_Buffer->GetAttachment(i)->GetResource(), ResourceState::RenderTarget };

		m_CommandList->ValidateStates(transitions);
		GPUTimer::EndEvent(m_CommandList);
	}

	// frame buffer node
	FrameBufferNode::FrameBufferNode(RenderGraph& graph, const Utils::Vector<Ref<RenderTarget2D>>& attachments) :
		RenderGraphNode(graph)
	{
		m_Buffer = FrameBuffer::Create(attachments);
	}


	void FrameBufferNode::OnViewportResize(uint32 width, uint32 height)
	{
		m_Buffer->Resize(width, height);
	}

	TransitionNode::TransitionNode(RenderGraph& graph) :
		RenderGraphNode(graph)
	{}

	void TransitionNode::AddBuffer(const TransitionObject& transition)
	{
		m_Transitions.Push(transition);
	}

	void TransitionNode::BuildImpl()
	{
		Utils::Vector<ResourceStateObject> transitions(m_Transitions.Count());
		for (uint32 i = 0; i < transitions.Count(); i++)
		{
			transitions[i].resource = *m_Transitions[i].handle;
			transitions[i].state = m_Transitions[i].state;
		}

		m_CommandList->ValidateStates(transitions);
	}



	ResolveMSAANode::ResolveMSAANode(RenderGraph& graph) :
		RenderGraphNode(graph)
	{}

	void ResolveMSAANode::BuildImpl()
	{
		m_CommandList->ResolveMSAA(m_Dest, m_Src);
	}

}

