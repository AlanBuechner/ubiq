#include "pch.h"
#include "SkyboxNode.h"
#include "Engine/Core/Application.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"
#include "Engine/Renderer/Shaders/Shader.h"
#include "Engine/Renderer/Camera.h"

#include "Engine/Core/MeshBuilder.h"

#include "RenderGraph.h"

namespace Game
{
	
	SkyboxNode::SkyboxNode(Engine::RenderGraph& graph) :
		RenderGraphNode(graph)
	{
		m_SkyboxShader = Engine::Application::Get().GetAssetManager().GetAsset<Engine::Shader>("Assets/Shaders/SkyboxShader.hlsl");

		struct Vertex
		{
			Math::Vector4 position;
		};

		Engine::TMeshBuilder<Vertex> meshBuilder;

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

		meshBuilder.Apply();

		m_SkyboxMesh = meshBuilder.mesh;

	}

	void SkyboxNode::BuildImpl()
	{
		const SceneData& scene = m_Graph.As<RenderGraph>().GetScene();

		Engine::GPUTimer::BeginEvent(m_CommandList, "Skybox Pass");
		m_CommandList->SetRenderTarget(m_RenderTarget);

		if (scene.m_Skybox)
		{
			Engine::Ref<Engine::GraphicsShaderPass> pass = m_SkyboxShader->GetGraphicsPass("main");
			m_CommandList->SetShader(pass);
			m_CommandList->SetRootConstant(pass->GetUniformLocation("RC_MainCameraIndex"), scene.m_MainCamera->GetCameraBuffer()->GetCBVDescriptor()->GetIndex());
			m_CommandList->SetTexture(pass->GetUniformLocation("texture"), scene.m_Skybox);
			m_CommandList->DrawMesh(m_SkyboxMesh);
		}

		Engine::GPUTimer::EndEvent(m_CommandList);
	}

}

