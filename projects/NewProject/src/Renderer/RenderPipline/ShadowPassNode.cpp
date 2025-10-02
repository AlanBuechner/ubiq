#include "pch.h"
#include "ShadowPassNode.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/Shaders/Shader.h"
#include "Engine/Renderer/Renderer.h"

#include "Engine/Renderer/Abstractions/GPUProfiler.h"

namespace Game
{
	void ShadowPassNode::Build()
	{
		CREATE_PROFILE_FUNCTIONI();

		Engine::GPUTimer::BeginEvent(m_CommandList, "Shadow Pass");
		BEGIN_EVENT_TRACE_GPU(m_CommandList, "Shadow pass");

		// convert all frame buffers to render targets
		Utils::Vector<Engine::ResourceStateObject> transitions;

		// directional light
		if (*m_DirLightBind)
		{
			// for each camera on the directional light
			for (auto& cm : (*m_DirLightBind)->GetShadowMaps())
			{
				const DirectionalLight::CascadedShadowMaps& maps = cm.second;
				for (Engine::Ref<Engine::FrameBuffer> fb : maps.m_ShadowMaps)
				{
					for (Engine::Ref<Engine::RenderTarget2D> rt : fb->GetAttachments())
						transitions.Push({ rt->GetResource(), Engine::ResourceState::RenderTarget });
				}
			}
		}


		m_CommandList->ValidateStates(transitions);


		// directional light
		if(*m_DirLightBind)
		{
			Engine::GPUTimer::BeginEvent(m_CommandList, "Directional Light");

			// for each camera using the directional lights shadow maps
			for (auto& cm : (*m_DirLightBind)->GetShadowMaps())
			{
				Engine::Ref<Engine::Camera> camera = cm.first;
				const DirectionalLight::CascadedShadowMaps& maps = cm.second;

				// for each cascade in the shadow map
				for(uint32 i = 0; i < DirectionalLight::s_NumShadowMaps; i++)
				{
					Engine::Ref<Engine::FrameBuffer> fb = maps.m_ShadowMaps[i];
					Engine::Ref<Engine::Camera> camera = maps.m_Cameras[i];

					Engine::GPUTimer::BeginEvent(m_CommandList, "Cascade " + std::to_string(i));

					m_CommandList->SetRenderTarget(fb);
					m_CommandList->ClearRenderTarget(fb);

					// for each object in the scene
					for (auto& cmd : *m_DrawCommandsBind)
					{
						Engine::Ref<Engine::GraphicsShaderPass> pass = cmd.m_Shader->GetGraphicsPass("directionalShadowMap");
						if (pass)
						{
							m_CommandList->SetShader(pass);
							m_CommandList->SetRootConstant("u_MainCameraIndex", camera->GetCameraBuffer()->GetCBVDescriptor()->GetIndex());
							m_CommandList->DrawMesh(cmd.m_Mesh, cmd.m_InstanceBuffer);
						}
					}

					Engine::GPUTimer::EndEvent(m_CommandList);
				}
			}

			Engine::GPUTimer::EndEvent(m_CommandList);
		}



		// convert all render targets to srv
		for (auto& transition : transitions)
			transition.state = Engine::ResourceState::ShaderResource;

		if(!transitions.Empty())
			m_CommandList->ValidateStates(transitions);

		END_EVENT_TRACE_GPU(m_CommandList);
		Engine::GPUTimer::EndEvent(m_CommandList);
	}

}

