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
		meshBuilder.Apply();

		m_ScreenMesh = meshBuilder.mesh;
	}

	void PostProcessNode::SetRenderTarget(Ref<RenderTarget2D> fb)
	{
		m_RenderTarget = fb;
		uint32 width = fb->GetResource()->GetWidth();
		uint32 height = fb->GetResource()->GetHeight();
		TextureFormat format = fb->GetResource()->GetFormat();
		m_BackBuffer = RenderTarget2D::Create(width, height, 1, format);
	}

	void PostProcessNode::InitPostProcessStack()
	{
		for (Ref<PostProcess> pp : m_PostProcessStack)
			pp->Init(m_Input, m_Graph.GetScene());
	}

	void PostProcessNode::OnViewportResize(uint32 width, uint32 height)
	{
		m_BackBuffer->Resize(width, height);

		for (uint32 i = 0; i < m_PostProcessStack.size(); i++)
		{
			Ref<PostProcess> post = m_PostProcessStack[i];
			post->OnViewportResize(width, height);
		}
	}

	void PostProcessNode::BuildImpl()
	{
		Ref<RenderTarget2D> curr = m_PostProcessStack.size() % 2 == 0 ? m_BackBuffer : m_RenderTarget;

		GPUTimer::BeginEvent(m_CommandList, "Post Processing");

		m_CommandList->ValidateStates({
			{curr->GetResource(), ResourceState::RenderTarget },
		});

		for (uint32 i = 0; i < m_PostProcessStack.size(); i++)
		{
			Ref<PostProcess> post = m_PostProcessStack[i];
			Ref<RenderTarget2D> lastPass = (curr == m_BackBuffer) ? m_RenderTarget : m_BackBuffer;
			Ref<Texture2D> src = lastPass;
			if (i == 0) src = m_Src;

			post->RecordCommands(m_CommandList, curr, src, m_Input, m_ScreenMesh);

			if (i + 1 < m_PostProcessStack.size())
			{
				m_CommandList->ValidateStates({
					{ lastPass->GetResource(), ResourceState::RenderTarget },
					{ curr->GetResource(), m_RenderTarget->GetResource()->GetDefultState() },
				});
			}

			curr = (curr == m_BackBuffer) ? m_RenderTarget : m_BackBuffer; // swap buffers
		}

		GPUTimer::EndEvent(m_CommandList);
	}

}

