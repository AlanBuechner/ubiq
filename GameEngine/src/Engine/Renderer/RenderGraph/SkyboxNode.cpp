#include "pch.h"
#include "SkyboxNode.h"
#include "Engine/Core/Application.h"
#include "Engine/Renderer/GPUProfiler.h"
#include "Engine/Core/MeshBuilder.h"

namespace Engine
{
	
	SkyboxNode::SkyboxNode(RenderGraph& graph, uint32 uSeg, uint32 vSeg) :
		RenderGraphNode(graph), m_USeg(uSeg), m_VSeg(vSeg)
	{
		m_CommandList = CommandList::Create(CommandList::Direct);

		m_SkyboxShader = Application::Get().GetAssetManager().GetAsset<Shader>("Assets/Shaders/SkyboxShader.hlsl");

		//m_SkyboxTexture = Application::Get().GetAssetManager().GetAsset<Texture2D>("Assets/Images/ogre_diffuse.bmp");
		m_SkyboxTexture = Application::Get().GetAssetManager().GetAsset<Texture2D>("Assets/Images/alps_field_4k.jpg");

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

	void SkyboxNode::AddToCommandQueue()
	{
		std::vector<Ref<CommandList>> dependencies;
		for (auto& cmdList : m_RenderTarget.GetInput()->GetCommandLists())
			dependencies.push_back(cmdList);
		Renderer::GetMainCommandQueue()->AddCommandList(m_CommandList, dependencies);
	}

	void SkyboxNode::BuildImpl()
	{
		Ref<FrameBuffer> renderTarget = m_RenderTarget.GetVar();
		const SceneData& scene = m_Graph.GetScene();

		m_CommandList->StartRecording();
		GPUTimer::BeginEvent(m_CommandList, "Skybox Pass");
		m_CommandList->SetRenderTarget(renderTarget);
		if (!renderTarget->Cleared())
			m_CommandList->ClearRenderTarget();

		m_CommandList->SetShader(m_SkyboxShader->GetPass("main"));
		m_CommandList->SetConstantBuffer(0, scene.m_MainCamera);
		m_CommandList->SetTexture(2, m_SkyboxTexture);
		m_CommandList->DrawMesh(m_SkyboxMesh);

		GPUTimer::EndEvent(m_CommandList);
		m_CommandList->Close();
		m_Graph.RecoardFrameBufferState({ FrameBuffer::State::RenderTarget, renderTarget });
	}

}

