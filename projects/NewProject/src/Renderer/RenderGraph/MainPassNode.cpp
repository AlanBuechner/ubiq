#include "pch.h"
#include "MainPassNode.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/Light.h"
#include "Engine/Renderer/Abstractions/Shader.h"

#include "Engine/Renderer/Abstractions/GPUProfiler.h"

#include "RenderGraph.h"

namespace Game
{
	ShaderPassNode::ShaderPassNode(Engine::RenderGraph& graph, const std::string& passName) :
		RenderGraphNode(graph), m_PassName(passName)
	{}

	void ShaderPassNode::BuildImpl()
	{
		const SceneData& scene = m_Graph.As<RenderGraph>().GetScene();

		Engine::GPUTimer::BeginEvent(m_CommandList, "Shader Pass");
		m_CommandList->SetRenderTarget(m_RenderTarget);

		for (auto& cmd : scene.m_DrawCommands)
		{
			Engine::Ref<Engine::ShaderPass> pass = cmd.m_Shader->GetPass(m_PassName);
			if (pass)
			{
				m_CommandList->SetShader(pass);
				m_CommandList->SetRootConstant(pass->GetUniformLocation("RC_MainCameraIndex"), scene.m_MainCamera->GetCameraBuffer()->GetCBVDescriptor()->GetIndex());
				m_CommandList->SetConstantBuffer(pass->GetUniformLocation("DirLight"), scene.m_DirectinalLight->GetBuffer());
				m_CommandList->SetStructuredBuffer(pass->GetUniformLocation("Cascades"), scene.m_DirectinalLight->GetShadowMaps()[scene.m_MainCamera].m_CameraIndeces);
				m_CommandList->DrawMesh(cmd.m_Mesh, cmd.m_InstanceBuffer);
			}
		}

		Engine::GPUTimer::EndEvent(m_CommandList);
	}
}


