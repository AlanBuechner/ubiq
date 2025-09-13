#include "pch.h"
#include "GBufferPassNode.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"
#include "Engine/Renderer/Shaders/Shader.h"
#include "Engine/Renderer/Camera.h"

#include "RenderGraph.h"

#include "Utils/Common.h"

namespace Game
{
	GBufferPassNode::GBufferPassNode(Engine::RenderGraph& graph) :
		RenderGraphNode(graph)
	{}

	void GBufferPassNode::BuildImpl()
	{
		CREATE_PROFILE_FUNCTIONI();
		const SceneData& scene = m_Graph.As<RenderGraph>().GetScene();

		// depth pre pass
		Engine::GPUTimer::BeginEvent(m_CommandList, "Depth pre pass");
		BEGIN_EVENT_TRACE_GPU(m_CommandList, "Depth pre pass");
		// set up render target
		m_CommandList->SetRenderTarget(m_RenderTarget);
		m_CommandList->ClearRenderTarget(m_RenderTarget);

		// render all objects
		for (auto& cmd : scene.m_MainPassDrawCommands)
		{
			Engine::Ref<Engine::GraphicsShaderPass> pass = cmd.m_Shader->GetGraphicsPass("depth");
			if (pass)
			{
				m_CommandList->SetShader(pass);
				m_CommandList->SetRootConstant("u_MainCameraIndex", scene.m_MainCamera->GetCameraBuffer()->GetCBVDescriptor()->GetIndex());
				m_CommandList->DrawMesh(cmd.m_Mesh, cmd.m_InstanceBuffer);
			}
		}
		END_EVENT_TRACE_GPU(m_CommandList);
		Engine::GPUTimer::EndEvent(m_CommandList); // end depth pre pass
	}
}

