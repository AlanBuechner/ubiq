#include "pch.h"
#include "ShadowPassNode.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Light.h"

#include "Engine/Renderer/GPUProfiler.h"

namespace Engine
{

	ShadowPassNode::ShadowPassNode(RenderGraph& graph) :
		RenderGraphNode(graph)
	{}

	void ShadowPassNode::BuildImpl()
	{
		const SceneData& scene = m_Graph.GetScene();

		GPUTimer::BeginEvent(m_CommandList, "Shadow Pass");

		// convert all frame buffers to render targets
		std::vector<ResourceStateObject> transitions;

		// directional light
		if (scene.m_DirectinalLight)
		{
			// remove all cameras from directional light
			scene.m_DirectinalLight->ClearCameras();
			// re add cameras to directional light
			scene.m_DirectinalLight->AddCamera(scene.m_MainCamera);

			// for each camera on the directional light
			for (auto& cm : scene.m_DirectinalLight->GetShadowMaps())
			{
				const DirectionalLight::CascadedShadowMaps& maps = cm.second;
				for (Ref<FrameBuffer> fb : maps.m_ShadowMaps)
				{
					for (Ref<RenderTarget2D> rt : fb->GetAttachments())
						transitions.push_back({ rt->GetResource(), ResourceState::RenderTarget });
				}
			}
		}


		m_CommandList->ValidateStates(transitions);


		// directional light
		if(scene.m_DirectinalLight) 
		{
			GPUTimer::BeginEvent(m_CommandList, "Directional Light");

			// for each camera using the directional lights shadow maps
			for (auto& cm : scene.m_DirectinalLight->GetShadowMaps())
			{
				Ref<Camera> camera = cm.first;
				const DirectionalLight::CascadedShadowMaps& maps = cm.second;

				// for each cascade in the shadow map
				for(uint32 i = 0; i < DirectionalLight::s_NumShadowMaps; i++)
				{
					Ref<FrameBuffer> fb = maps.m_ShadowMaps[i];
					Ref<Camera> camera = maps.m_Cameras[i];

					GPUTimer::BeginEvent(m_CommandList, "Cascade " + std::to_string(i));

					m_CommandList->SetRenderTarget(fb);
					m_CommandList->ClearRenderTarget(fb);

					// for each object in the scene
					for (auto& cmd : scene.m_DrawCommands)
					{
						Ref<ShaderPass> pass = cmd.m_Shader->GetPass("depth");
						if (pass)
						{
							m_CommandList->SetShader(pass);
							m_CommandList->SetRootConstant(pass->GetUniformLocation("RC_MainCameraIndex"), camera->GetCameraBuffer()->GetCBVDescriptor()->GetIndex());
							m_CommandList->DrawMesh(cmd.m_Mesh, cmd.m_InstanceBuffer);
						}
					}

					GPUTimer::EndEvent(m_CommandList);
				}
			}

			GPUTimer::EndEvent(m_CommandList);
		}

		// convert all render targets to srv
		for (auto& transition : transitions)
			transition.state = ResourceState::ShaderResource;

		if(!transitions.empty())
			m_CommandList->ValidateStates(transitions);

		GPUTimer::EndEvent(m_CommandList);
	}

}

