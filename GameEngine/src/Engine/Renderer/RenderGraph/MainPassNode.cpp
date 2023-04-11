#include "pch.h"
#include "MainPassNode.h"
#include "Engine/Renderer/GPUProfiler.h"

namespace Engine
{
	ShaderPassNode::ShaderPassNode(RenderGraph& graph, const std::string& passName) :
		RenderGraphNode(graph), m_PassName(passName)
	{
		m_CommandList = CommandList::Create(CommandList::Direct);
	}

	void ShaderPassNode::BuildImpl()
	{
		const SceneData& scene = m_Graph.GetScene();

		GPUTimer::BeginEvent(m_CommandList, "Shader Pass");
		m_CommandList->SetRenderTarget(m_RenderTarget);

		for (auto& cmd : scene.m_DrawCommands)
		{
			Ref<ShaderPass> pass = cmd.m_Shader->GetPass(m_PassName);
			if (pass)
			{
				m_CommandList->SetShader(pass);
				m_CommandList->SetRootConstant(pass->GetUniformLocation("RC_MainCameraIndex"), scene.m_MainCamera->GetCameraBuffer()->GetDescriptorLocation());
				m_CommandList->SetConstantBuffer(pass->GetUniformLocation("DirLight"), scene.m_DirectinalLight->GetBuffer());
				m_CommandList->SetConstantBuffer(pass->GetUniformLocation("Cascades"), scene.m_DirectinalLight->GetShadowMaps()[scene.m_MainCamera].m_CameraIndeces);
				m_CommandList->DrawMesh(cmd.m_Mesh, cmd.m_InstanceBuffer);
			}
		}

		GPUTimer::EndEvent(m_CommandList);
	}
}


