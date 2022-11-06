#include "pch.h"
#include "MainPassNode.h"
#include "Engine/Renderer/GPUProfiler.h"

namespace Engine
{
	ShaderPassNode::ShaderPassNode(RenderGraph& graph, const std::string& passName) :
		RenderGraphNode(graph), m_PassName(passName)
	{
		m_CommandList = CommandList::Create(CommandList::Direct);
	}

	void ShaderPassNode::SetRenderTarget(FrameBufferVar var)
	{
		m_RenderTarget = var;
	}

	void ShaderPassNode::AddToCommandQueue()
	{
		std::vector<Ref<CommandList>> dependencies;
		for (auto& cmdList : m_RenderTarget.GetInput()->GetCommandLists())
			dependencies.push_back(cmdList);
		Renderer::GetMainCommandQueue()->AddCommandList(m_CommandList, dependencies);
	}

	void ShaderPassNode::BuildImpl()
	{
		Ref<FrameBuffer> renderTarget = m_RenderTarget.GetVar();
		const SceneData& scene = m_Graph.GetScene();
		
		m_CommandList->StartRecording();
		GPUTimer::BeginEvent(m_CommandList, "Shader Pass");
		m_CommandList->SetRenderTarget(renderTarget);
		if(!renderTarget->Cleared())
			m_CommandList->ClearRenderTarget();

		for (auto& cmd : scene.m_DrawCommands)
		{
			Ref<ShaderPass> pass = cmd.m_Shader->GetPass(m_PassName);
			m_CommandList->SetShader(pass);
			m_CommandList->SetConstantBuffer(pass->GetUniformLocation("MainCameraIndex"), scene.m_MainCamera);
			m_CommandList->SetConstantBuffer(pass->GetUniformLocation("DirLight"), scene.m_DirectinalLight);
			m_CommandList->DrawMesh(cmd.m_Mesh, cmd.m_InstanceBuffer);
		}

		GPUTimer::EndEvent(m_CommandList);
		m_CommandList->Close();
		m_Graph.RecoardFrameBufferState({ FrameBuffer::State::RenderTarget, renderTarget });
	}
}


