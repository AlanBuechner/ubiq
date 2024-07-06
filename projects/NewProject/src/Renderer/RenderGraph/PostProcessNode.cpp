#include "pch.h"
#include "PostProcessNode.h"
#include "Engine/Core/MeshBuilder.h"
#include "EngineResource.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"
#include "RenderGraph.h"

namespace Game
{

	PostProcessNode::PostProcessNode(Engine::RenderGraph& graph) :
		RenderGraphNode(graph)
	{
		struct Vertex
		{
			Math::Vector4 position;
		};

		Engine::TMeshBuilder<Vertex> meshBuilder;

		meshBuilder.vertices.Push({ {-1,-1,1,1} });
		meshBuilder.vertices.Push({ { 1,-1,1,1} });
		meshBuilder.vertices.Push({ {-1, 1,1,1} });
		meshBuilder.vertices.Push({ { 1, 1,1,1} });

		meshBuilder.indices.Push(1);
		meshBuilder.indices.Push(2);
		meshBuilder.indices.Push(3);
		meshBuilder.indices.Push(0);
		meshBuilder.indices.Push(2);
		meshBuilder.indices.Push(1);
		meshBuilder.Apply();

		m_ScreenMesh = meshBuilder.mesh;
	}

	void PostProcessNode::SetRenderTarget(Engine::Ref<Engine::RenderTarget2D> fb)
	{
		m_RenderTarget = fb;
		uint32 width = fb->GetResource()->GetWidth();
		uint32 height = fb->GetResource()->GetHeight();
		Engine::TextureFormat format = fb->GetResource()->GetFormat();
		m_BackBuffer = Engine::RenderTarget2D::Create(width, height, format, true);
	}

	void PostProcessNode::InitPostProcessStack()
	{
		for (Engine::Ref<PostProcess> pp : m_PostProcessStack)
			pp->Init(m_Input, m_Graph.As<RenderGraph>().GetScene());
	}

	void PostProcessNode::OnViewportResize(uint32 width, uint32 height)
	{
		m_BackBuffer->Resize(width, height);

		for (uint32 i = 0; i < m_PostProcessStack.size(); i++)
		{
			Engine::Ref<PostProcess> post = m_PostProcessStack[i];
			post->OnViewportResize(width, height);
		}
	}

	void PostProcessNode::BuildImpl()
	{
		Engine::Ref<Engine::RenderTarget2D> curr = m_PostProcessStack.size() % 2 == 0 ? m_BackBuffer : m_RenderTarget;

		Engine::GPUTimer::BeginEvent(m_CommandList, "Post Processing");

		for (uint32 i = 0; i < m_PostProcessStack.size(); i++)
		{
			Engine::Ref<PostProcess> post = m_PostProcessStack[i];
			Engine::Ref<Engine::RenderTarget2D> lastPass = (curr == m_BackBuffer) ? m_RenderTarget : m_BackBuffer;
			Engine::Ref<Engine::Texture2D> src = lastPass;
			if (i == 0) src = m_Src;

			m_CommandList->ValidateStates({
				{ curr->GetResource(), Engine::ResourceState::RenderTarget },
				{ src->GetResource(), Engine::ResourceState::ShaderResource },
			});

			post->RecordCommands(m_CommandList, curr, src, m_Input, m_ScreenMesh);

			curr = (curr == m_BackBuffer) ? m_RenderTarget : m_BackBuffer; // swap buffers
		}

		m_CommandList->ValidateStates({
			{ m_RenderTarget->GetResource(), Engine::ResourceState::RenderTarget },
		});

		Engine::GPUTimer::EndEvent(m_CommandList);
	}

}

