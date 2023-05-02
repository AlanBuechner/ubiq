#include "pch.h"
#include "PostProcessNode.h"
#include "Engine/Core/MeshBuilder.h"
#include "EngineResource.h"
#include "Engine/Renderer/GPUProfiler.h"

namespace Engine
{

	PostProcessNode::PostProcessNode(RenderGraph& graph) :
		RenderGraphNode(graph)
	{
		struct Vertex
		{
			Math::Vector4 position;
		};

		TMeshBuilder<Vertex> meshBuilder;

		meshBuilder.vertices.push_back({ {-1,-1,1,1} });
		meshBuilder.vertices.push_back({ { 1,-1,1,1} });
		meshBuilder.vertices.push_back({ {-1, 1,1,1} });
		meshBuilder.vertices.push_back({ { 1, 1,1,1} });

		meshBuilder.indices.push_back(1);
		meshBuilder.indices.push_back(2);
		meshBuilder.indices.push_back(3);
		meshBuilder.indices.push_back(0);
		meshBuilder.indices.push_back(2);
		meshBuilder.indices.push_back(1);

		m_ScreenMesh = meshBuilder.ToMesh();
	}

	void PostProcessNode::SetRenderTarget(Ref<FrameBuffer> fb)
	{
		m_RenderTarget = fb;
		FrameBufferSpecification spec = m_RenderTarget->GetSpecification();
		m_BackBuffer = FrameBuffer::Create(spec);
	}

	void PostProcessNode::InitPostProcessStack()
	{
		for (Ref<PostProcess> pp : m_PostProcessStack)
			pp->Init(m_Input, m_Graph.GetScene());
	}

	void PostProcessNode::OnViewportResize(uint32 width, uint32 height)
	{
		m_BackBuffer->Resize(width, height);
	}

	void PostProcessNode::BuildImpl()
	{
		Ref<FrameBuffer> curr = m_PostProcessStack.size() % 2 == 0 ? m_BackBuffer : m_RenderTarget;

		GPUTimer::BeginEvent(m_CommandList, "Post Processing");

		for (uint32 i = 0; i < m_PostProcessStack.size(); i++)
		{
			Ref<PostProcess> post = m_PostProcessStack[i];
			Ref<FrameBuffer> lastPass = (curr == m_BackBuffer) ? m_RenderTarget : m_BackBuffer;
			uint64 srcLoc = lastPass->GetAttachmentShaderDescriptoLocation(0);
			if (i == 0) srcLoc = m_SrcDescriptorLocation;

			post->RecordCommands(m_CommandList, curr, srcLoc, m_Input, m_ScreenMesh);

			m_CommandList->Transition({ 
				{curr, m_RenderTarget->GetSpecification().InitalState, FrameBufferState::RenderTarget },
				{lastPass, FrameBufferState::RenderTarget, m_RenderTarget->GetSpecification().InitalState },
			});

			curr = (curr == m_BackBuffer) ? m_RenderTarget : m_BackBuffer; // swap buffers
		}

		GPUTimer::EndEvent(m_CommandList);
	}

}

