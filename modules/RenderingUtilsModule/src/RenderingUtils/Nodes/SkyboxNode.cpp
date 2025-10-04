#include "pch.h"
#include "SkyboxNode.h"
#include "Engine/Core/Application.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"
#include "Engine/Renderer/Shaders/Shader.h"
#include "Engine/Renderer/Camera.h"

#include "Engine/Core/MeshBuilder.h"

#include "Resource.h"

namespace Game
{
	
	SkyboxNode::SkyboxNode()
	{
		m_SkyboxShader = Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Shader>(SKYBOX);

		struct Vertex
		{
			Math::Vector4 position;
		};

		Engine::TMeshBuilder<Vertex> meshBuilder;

		meshBuilder.vertices.Push({ {-1,-1,1,1} });
		meshBuilder.vertices.Push({ { 1,-1,1,1} });
		meshBuilder.vertices.Push({ {-1, 1,1,1} });
		meshBuilder.vertices.Push({ { 1, 1,1,1} });

		meshBuilder.indices.Push(1);
		meshBuilder.indices.Push(2);
		meshBuilder.indices.Push(3);
		meshBuilder.indices.Push(0);
		meshBuilder.indices.Push(2);
		meshBuilder.indices.Push(1);

		meshBuilder.Apply();

		m_SkyboxMesh = meshBuilder.mesh;

	}

	void SkyboxNode::Build()
	{
		CREATE_PROFILE_FUNCTIONI();

		Engine::GPUTimer::BeginEvent(m_CommandList, "Skybox Pass");
		BEGIN_EVENT_TRACE_GPU(m_CommandList, "Skybox pass");
		m_CommandList->SetRenderTarget(m_RenderTarget);

		if (m_SkyboxTextureBind)
		{
			m_CommandList->SetShader(m_SkyboxShader->GetGraphicsPass("main"));
			m_CommandList->SetRootConstant("u_MainCameraIndex", (*m_CameraBind)->GetCameraBuffer()->GetCBVDescriptor()->GetIndex());
			m_CommandList->SetTexture("u_Texture", *m_SkyboxTextureBind);
			m_CommandList->DrawMesh(m_SkyboxMesh);
		}

		END_EVENT_TRACE_GPU(m_CommandList);
		Engine::GPUTimer::EndEvent(m_CommandList);
	}

}

