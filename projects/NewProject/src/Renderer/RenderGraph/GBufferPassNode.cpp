#include "pch.h"
#include "GBufferPassNode.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"
#include "Engine/Renderer/Abstractions/Shader.h"
#include "Engine/Renderer/Camera.h"

#include "RenderGraph.h"

namespace Game
{
	GBufferPassNode::GBufferPassNode(Engine::RenderGraph& graph) :
		RenderGraphNode(graph)
	{}

	void GBufferPassNode::BuildImpl()
	{
		const SceneData& scene = m_Graph.As<RenderGraph>().GetScene();

		// depth pre pass
		Engine::GPUTimer::BeginEvent(m_CommandList, "Depth pre pass");
		// set up render target
		m_CommandList->SetRenderTarget(m_RenderTarget);
		m_CommandList->ClearRenderTarget(m_RenderTarget);

		// render all objects
		for (auto& cmd : scene.m_DrawCommands)
		{
			Engine::Ref<Engine::ShaderPass> pass = cmd.m_Shader->GetPass("depth");
			if (pass)
			{
				m_CommandList->SetShader(pass);
				m_CommandList->SetRootConstant(pass->GetUniformLocation("RC_MainCameraIndex"), scene.m_MainCamera->GetCameraBuffer()->GetCBVDescriptor()->GetIndex());
				m_CommandList->DrawMesh(cmd.m_Mesh, cmd.m_InstanceBuffer);
			}
		}
		Engine::GPUTimer::EndEvent(m_CommandList); // end depth pre pass

		// begin gbuffer pass
		Engine::GPUTimer::BeginEvent(m_CommandList, "GBuffer pass");
		Engine::GPUTimer::EndEvent(m_CommandList); // end gbuffer pass
	}
}

