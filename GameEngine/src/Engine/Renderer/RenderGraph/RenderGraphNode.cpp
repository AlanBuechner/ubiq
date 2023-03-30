#include "pch.h"
#include "RenderGraphNode.h"
#include "Engine/Core/Application.h"
#include "Engine/Core/Window.h"

namespace Engine
{


	RenderGraphNode::RenderGraphNode(RenderGraph& graph) :
		m_Graph(graph)
	{}

	// frame buffer var
	FrameBufferVar::FrameBufferVar(Ref<RenderGraphNode> input, Ref<FrameBuffer> var) :
		m_Input(input), m_Var(var)
	{}

	Ref<FrameBuffer> FrameBufferVar::GetVar()
	{
		if (m_Input)
			m_Input->Build();
		return m_Var;
	}

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

	// frame buffer node
	FrameBufferNode::FrameBufferNode(RenderGraph& graph) :
		RenderGraphNode(graph)
	{
		FrameBufferSpecification fbSpec;
		fbSpec.Attachments = {
			{ FrameBufferTextureFormat::RGBA16, {0.1f,0.1f,0.1f,1} },
			{ FrameBufferTextureFormat::Depth, { 1,0,0,0 } }
		};
		fbSpec.InitalState = FrameBufferState::Common;

		Window& window = Application::Get().GetWindow();
		fbSpec.Width = window.GetWidth();
		fbSpec.Height = window.GetHeight();

		m_Buffer = FrameBuffer::Create(fbSpec);
	}


	void FrameBufferNode::OnViewportResize(uint32 width, uint32 height)
	{
		m_Buffer->Resize(width, height);
	}

}

