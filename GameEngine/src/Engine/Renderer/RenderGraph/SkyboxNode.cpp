#include "pch.h"
#include "SkyboxNode.h"
#include "Engine/Core/Application.h"
#include "Engine/Renderer/GPUProfiler.h"
#include "Engine/Core/MeshBuilder.h"

namespace Engine
{
	
	SkyboxNode::SkyboxNode(RenderGraph& graph) :
		RenderGraphNode(graph)
	{
		m_CommandList = CommandList::Create(CommandList::Direct);

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

	void SkyboxNode::SetRenderTarget(FrameBufferVar var)
	{
		m_RenderTarget = var;
	}

	void SkyboxNode::AddToCommandQueue(Ref<ExecutionOrder> order)
	{
		std::vector<Ref<CommandList>> dependencies;
		for (auto& cmdList : m_RenderTarget.GetInput()->GetCommandLists())
			dependencies.push_back(cmdList);
		order->Add(
			m_CommandList, dependencies);
	}

	void SkyboxNode::BuildImpl()
	{
		Ref<FrameBuffer> renderTarget = m_RenderTarget.GetVarAndBuild();
		const SceneData& scene = m_Graph.GetScene();

		m_CommandList->StartRecording();

		GPUTimer::BeginEvent(m_CommandList, "Skybox Pass");
		m_CommandList->SetRenderTarget(renderTarget);

		if (scene.m_Skybox)
		{
			Ref<ShaderPass> pass = m_SkyboxShader->GetPass("main");
			m_CommandList->SetShader(pass);
			m_CommandList->SetRootConstant(pass->GetUniformLocation("RC_MainCameraIndex"), scene.m_MainCamera->GetCameraBuffer()->GetDescriptorLocation());
			m_CommandList->SetTexture(pass->GetUniformLocation("texture"), scene.m_Skybox);
			m_CommandList->DrawMesh(m_SkyboxMesh);
		}

		GPUTimer::EndEvent(m_CommandList);
		m_CommandList->Close();
	}

}

