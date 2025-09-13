#include "pch.h"
#include "MainPassNode.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/Shaders/Shader.h"

#include "Engine/Renderer/Abstractions/GPUProfiler.h"

#include "RenderingUtils/Lighting/DirectionalLight.h"
#include "RenderGraph.h"

namespace Game
{
	ShaderPassNode::ShaderPassNode(Engine::RenderGraph& graph, const std::string& passName) :
		RenderGraphNode(graph), m_PassName(passName)
	{}

	void ShaderPassNode::BuildImpl()
	{
		CREATE_PROFILE_FUNCTIONI();
		const SceneData& scene = m_Graph.As<RenderGraph>().GetScene();

		Engine::GPUTimer::BeginEvent(m_CommandList, "Shader Pass");
		BEGIN_EVENT_TRACE_GPU(m_CommandList, "Shader pass");
		m_CommandList->SetRenderTarget(m_RenderTarget);

		for (auto& cmd : scene.m_MainPassDrawCommands)
		{
			Engine::Ref<Engine::GraphicsShaderPass> pass = cmd.m_Shader->GetGraphicsPass(m_PassName);
			if (pass)
			{
				m_CommandList->SetShader(pass);
				m_CommandList->SetRootConstant("u_MainCameraIndex", scene.m_MainCamera->GetCameraBuffer()->GetCBVDescriptor()->GetIndex());

				// set directional light
				bool useDirLight = scene.m_DirectinalLight != nullptr;
				m_CommandList->SetRootConstant("u_UseDirLight", useDirLight);
				if (useDirLight)
				{
					m_CommandList->SetConstantBuffer("u_DirLight", scene.m_DirectinalLight->GetBuffer());
					m_CommandList->SetStructuredBuffer("u_Cascades", scene.m_DirectinalLight->GetShadowMaps()[scene.m_MainCamera].m_CameraIndeces);
				}

				m_CommandList->DrawMesh(cmd.m_Mesh, cmd.m_InstanceBuffer);
			}
		}

		END_EVENT_TRACE_GPU(m_CommandList);
		Engine::GPUTimer::EndEvent(m_CommandList);
	}
}


