#include "pch.h"
#include "SkyboxNode.h"
#include "Engine/Core/Application.h"
#include "Engine/Renderer/GPUProfiler.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/Shader.h"

#include "Engine/Core/MeshBuilder.h"

namespace Engine
{
	
	SkyboxNode::SkyboxNode(RenderGraph& graph) :
		RenderGraphNode(graph)
	{
		m_SkyboxShader = Application::Get().GetAssetManager().GetAsset<Shader>("Assets/Shaders/SkyboxShader.hlsl");

		struct Vertex
		{
			Math::Vector4 position;
		};

		TMeshBuilder<Vertex> meshBuilder;

		meshBuilder.vertices.push_back({ {-1,-1,1,1} });
		meshBuilder.vertices.push_back({ { 1,-1,1,1} });
		meshBuilder.vertices.push_back({ {-1, 1,1,1} });
		meshBuilder.vertices.push_back({ { 1, 1,1,1} });

		meshBuilder.indices.push_back(1);
		meshBuilder.indices.push_back(2);
		meshBuilder.indices.push_back(3);
		meshBuilder.indices.push_back(0);
		meshBuilder.indices.push_back(2);
		meshBuilder.indices.push_back(1);

		m_SkyboxMesh = meshBuilder.ToMesh();

	}

	void SkyboxNode::BuildImpl()
	{
		const SceneData& scene = m_Graph.GetScene();

		GPUTimer::BeginEvent(m_CommandList, "Skybox Pass");
		m_CommandList->SetRenderTarget(m_RenderTarget);

		if (scene.m_Skybox)
		{
			Ref<ShaderPass> pass = m_SkyboxShader->GetPass("main");
			m_CommandList->SetShader(pass);
			m_CommandList->SetRootConstant(pass->GetUniformLocation("RC_MainCameraIndex"), scene.m_MainCamera->GetCameraBuffer()->GetDescriptorLocation());
			m_CommandList->SetTexture(pass->GetUniformLocation("texture"), scene.m_Skybox);
			m_CommandList->DrawMesh(m_SkyboxMesh);
		}

		GPUTimer::EndEvent(m_CommandList);
	}

}

