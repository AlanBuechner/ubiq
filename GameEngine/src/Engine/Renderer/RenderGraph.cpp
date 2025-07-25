#include "pch.h"
#include "RenderGraph.h"

#include "Engine/Core/Window.h"
#include "Engine/Core/Application.h"

#include "Engine/Renderer/Abstractions/Resources/FrameBuffer.h"
#include "Engine/Renderer/Abstractions/Resources/InstanceBuffer.h"
#include "Engine/Renderer/Abstractions/Resources/ConstantBuffer.h"
#include "Engine/Renderer/Abstractions/Resources/Texture.h"
#include "Engine/Renderer/CPUCommandList.h"
#include "Engine/Renderer/Shaders/Shader.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Camera.h"
#include "RenderGraphNode.h"

namespace Engine
{

	RenderGraph::RenderGraph()
	{
	}

	RenderGraph::~RenderGraph()
	{
	}

	void RenderGraph::Submit()
	{
		Renderer::SubmitCommandLists(m_CommandLists);
	}

	void RenderGraph::OnViewportResize(uint32 width, uint32 height)
	{
		for (auto& node : m_Nodes)
			node->OnViewportResize(width, height);
	}

	void RenderGraph::Build()
	{
		for (Ref<CPUCommandList> list : m_CommandLists)
			list->StartRecording();

		for (auto& node : m_Nodes)
			node->Invalidate();

		for (auto& node : m_Nodes)
			node->Build();

		for (Ref<CPUCommandList> list : m_CommandLists)
			list->StopRecording();
	}

	Ref<FrameBuffer> RenderGraph::GetRenderTarget()
	{
		return m_OutputNode->m_Buffer;
	}

}

