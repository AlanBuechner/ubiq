#include "pch.h"
#include "PostProcessNode.h"

namespace Engine
{

	PostProcessNode::PostProcessNode(RenderGraph& graph) :
		RenderGraphNode(graph)
	{
		m_CommandList = CommandList::Create(CommandList::Direct);
	}

	void PostProcessNode::SetRenderTarget(Ref<FrameBuffer> fb)
	{
		m_RenderTarget = fb;
		m_BackBuffer = FrameBuffer::Create(m_RenderTarget->GetSpecification());
	}

	void PostProcessNode::BuildImpl()
	{
		Ref<FrameBuffer> curr = m_PostProcessStack.size() % 2 == 0 ? m_BackBuffer : m_RenderTarget;

		for (Ref<PostProcess> post : m_PostProcessStack)
		{
			post->RecordCommands(m_CommandList, curr, m_Input);
			curr = (curr == m_BackBuffer) ? m_RenderTarget : m_BackBuffer; // swap buffers
		}
	}

}

