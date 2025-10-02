#include "pch.h"
#include "RenderPiplineNode.h"
#include "Engine/Core/Application.h"
#include "Engine/Core/Window.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Abstractions/CommandList.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"

namespace Engine
{

	// output node
	void OutputNode::Build()
	{
		GPUTimer::BeginEvent(m_CommandList, "Output Node");

		Utils::Vector<ResourceStateObject> transitions(m_Buffer->GetAttachments().Count());
		for (uint32 i = 0; i < transitions.Count(); i++)
			transitions[i] = { m_Buffer->GetAttachment(i)->GetResource(), ResourceState::RenderTarget };

		m_CommandList->ValidateStates(transitions);
		GPUTimer::EndEvent(m_CommandList);
	}



	// frame buffer node
	FrameBufferNode::FrameBufferNode(const Utils::Vector<Ref<RenderTarget2D>>& attachments)
	{
		m_Buffer = FrameBuffer::Create(attachments);
	}


	void FrameBufferNode::OnViewportResize(uint32 width, uint32 height)
	{
		m_Buffer->Resize(width, height);
	}




	// transition node
	void TransitionNode::AddBuffer(const TransitionObject& transition)
	{
		m_Transitions.Push(transition);
	}

	void TransitionNode::Build()
	{
		Utils::Vector<ResourceStateObject> transitions(m_Transitions.Count());
		for (uint32 i = 0; i < transitions.Count(); i++)
		{
			transitions[i].resource = *m_Transitions[i].handle;
			transitions[i].state = m_Transitions[i].state;
		}

		m_CommandList->ValidateStates(transitions);
	}


	// msaa node
	void ResolveMSAANode::Build()
	{
		m_CommandList->ResolveMSAA(m_Dest, m_Src);
	}

}

