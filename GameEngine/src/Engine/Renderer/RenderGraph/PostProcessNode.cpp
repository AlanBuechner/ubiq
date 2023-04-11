#include "pch.h"
#include "PostProcessNode.h"

namespace Engine
{

	PostProcessNode::PostProcessNode(RenderGraph& graph) :
		RenderGraphNode(graph)
	{
		m_CommandList = CommandList::Create(CommandList::Direct);
	}

	void PostProcessNode::SetRenderTarget(FrameBufferVar var)
	{
		m_RenderTarget = var;
		m_BackBuffer = FrameBuffer::Create(m_RenderTarget.GetVar()->GetSpecification());
	}

	void PostProcessNode::AddToCommandQueue(Ref<ExecutionOrder> order)
	{
		std::vector<Ref<CommandList>> dependencies;
		for (auto& cmdList : m_RenderTarget.GetInput()->GetCommandLists())
			dependencies.push_back(cmdList);
		for (auto& node : m_Dependincys)
		{
			for (auto& cmdList : node->GetCommandLists())
				dependencies.push_back(cmdList);
		}
		order->Add(m_CommandList, dependencies);
	}

	void PostProcessNode::BuildImpl()
	{
		Ref<FrameBuffer> renderTarget = m_RenderTarget.GetVarAndBuild();

		Ref<FrameBuffer> curr = m_BackBuffer;

		m_CommandList->StartRecording();

		for (Ref<PostProcess> post : m_PostProcessStack)
		{
			post->RecordCommands(m_CommandList, curr);
			curr = (curr == m_BackBuffer) ? renderTarget : m_BackBuffer; // swap buffers
		}

		m_CommandList->Close();
	}

}

