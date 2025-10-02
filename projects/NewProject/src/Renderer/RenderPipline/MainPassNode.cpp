#include "pch.h"
#include "MainPassNode.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/Shaders/Shader.h"

#include "Engine/Renderer/Abstractions/GPUProfiler.h"

#include "RenderingUtils/Lighting/DirectionalLight.h"

namespace Game
{
	ShaderPassNode::ShaderPassNode(const std::string& passName) :
		m_PassName(passName)
	{}

	void ShaderPassNode::Build()
	{
		CREATE_PROFILE_FUNCTIONI();

		Engine::GPUTimer::BeginEvent(m_CommandList, "Shader Pass");
		BEGIN_EVENT_TRACE_GPU(m_CommandList, "Shader pass");
		m_CommandList->SetRenderTarget(m_RenderTarget);

		for (auto& cmd : *m_DrawCommandsBind)
		{
			Engine::Ref<Engine::GraphicsShaderPass> pass = cmd.m_Shader->GetGraphicsPass(m_PassName);
			if (pass)
			{
				m_CommandList->SetShader(pass);
				m_CommandList->SetRootConstant("u_MainCameraIndex", (*m_CameraBind)->GetCameraBuffer()->GetCBVDescriptor()->GetIndex());

				// set directional light
				bool useDirLight = (*m_DirLightBind) != nullptr;
				m_CommandList->SetRootConstant("u_UseDirLight", useDirLight);
				if (useDirLight)
				{
					m_CommandList->SetConstantBuffer("u_DirLight", (*m_DirLightBind)->GetBuffer());
					m_CommandList->SetStructuredBuffer("u_Cascades", (*m_DirLightBind)->GetShadowMaps()[(*m_CameraBind)].m_CameraIndeces);
				}

				m_CommandList->DrawMesh(cmd.m_Mesh, cmd.m_InstanceBuffer);
			}
		}

		END_EVENT_TRACE_GPU(m_CommandList);
		Engine::GPUTimer::EndEvent(m_CommandList);
	}
}


