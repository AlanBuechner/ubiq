#include "pch.h"
#include "GBufferPassNode.h"
#include "Engine/Renderer/GPUProfiler.h"
#include "Engine/Renderer/Camera.h"

namespace Engine
{
	GBufferPassNode::GBufferPassNode(RenderGraph& graph) :
		RenderGraphNode(graph)
	{}

	void GBufferPassNode::BuildImpl()
	{
		const SceneData& scene = m_Graph.GetScene();

		// depth pre pass
		GPUTimer::BeginEvent(m_CommandList, "Depth pre pass");
		// set up render target
		m_CommandList->SetRenderTarget(m_RenderTarget);
		m_CommandList->ClearRenderTarget();

		// render all objects
		for (auto& cmd : scene.m_DrawCommands)
		{
			Ref<ShaderPass> pass = cmd.m_Shader->GetPass("depth");
			if (pass)
			{
				m_CommandList->SetShader(pass);
				m_CommandList->SetRootConstant(pass->GetUniformLocation("RC_MainCameraIndex"), scene.m_MainCamera->GetCameraBuffer()->GetDescriptorLocation());
				m_CommandList->DrawMesh(cmd.m_Mesh, cmd.m_InstanceBuffer);
			}
		}
		GPUTimer::EndEvent(m_CommandList); // end depth pre pass

		// begin gbuffer pass
		GPUTimer::BeginEvent(m_CommandList, "GBuffer pass");
		GPUTimer::EndEvent(m_CommandList); // end gbuffer pass
	}
}

