#include "pch.h"
#include "ShadowPassNode.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/Abstractions/Shader.h"
#include "Engine/Renderer/Light.h"
#include "Engine/Renderer/Renderer.h"

#include "Engine/Renderer/Abstractions/GPUProfiler.h"

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
						Ref<ShaderPass> pass = cmd.m_Shader->GetPass("directionalShadowMap");
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



		//// convert all render targets to uav
		//for (auto& transition : transitions)
		//	transition.state = ResourceState::UnorderedResource;
		//GPUTimer::BeginEvent(m_CommandList, "Filter ShadowMaps");
		//Ref<ShaderPass> blurXPass = Renderer::GetBlitShader()->GetPass("BlurX");
		//Ref<ShaderPass> blurYPass = Renderer::GetBlitShader()->GetPass("BlurY");

		//if (scene.m_DirectinalLight)
		//{
		//	for (auto& cm : scene.m_DirectinalLight->GetShadowMaps())
		//	{
		//		Ref<Camera> camera = cm.first;
		//		const DirectionalLight::CascadedShadowMaps& maps = cm.second;

		//		// for each cascade in the shadow map
		//		for (uint32 i = 0; i < DirectionalLight::s_NumShadowMaps; i++)
		//		{
		//			Ref<RWTexture2D> shadowMap = maps.m_ShadowMaps[i]->GetAttachment(0)->GetRWTexture2D();
		//			Ref<RWTexture2D> shadowMapTemp = maps.m_ShadowMapsTemp[i];

		//			GPUTimer::BeginEvent(m_CommandList, "Cascade " + std::to_string(i));

		//			GPUTimer::BeginEvent(m_CommandList, "Blur");

		//			const uint32 ThreadSize = 128;

		//			m_CommandList->SetShader(blurXPass);
		//			m_CommandList->SetRWTexture(blurXPass->GetUniformLocation("DstTexture"), shadowMapTemp, 0);
		//			m_CommandList->SetRWTexture(blurXPass->GetUniformLocation("SrcTexture"), shadowMap, 0);
		//			m_CommandList->Dispatch(std::ceil((float)shadowMap->GetWidth() / (float)ThreadSize), shadowMap->GetHeight(), 1);

		//			m_CommandList->SetShader(blurYPass);
		//			m_CommandList->SetRWTexture(blurYPass->GetUniformLocation("DstTexture"), shadowMap, 0);
		//			m_CommandList->SetRWTexture(blurYPass->GetUniformLocation("SrcTexture"), shadowMapTemp, 0);
		//			m_CommandList->Dispatch(shadowMap->GetWidth(), std::ceil((float)shadowMap->GetHeight() / (float)ThreadSize), 1);

		//			GPUTimer::EndEvent(m_CommandList);

		//			GPUTimer::BeginEvent(m_CommandList, "Create Mips");

		//			GPUTimer::EndEvent(m_CommandList);

		//			GPUTimer::EndEvent(m_CommandList);
		//		}
		//	}
		//}

		//GPUTimer::EndEvent(m_CommandList);



		// convert all render targets to srv
		for (auto& transition : transitions)
			transition.state = ResourceState::ShaderResource;

		if(!transitions.empty())
			m_CommandList->ValidateStates(transitions);

		GPUTimer::EndEvent(m_CommandList);
	}

}

