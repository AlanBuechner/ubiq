#include "pch.h"
#include "MainPassNode.h"

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
		m_CommandList->SetRenderTarget(renderTarget);
		m_CommandList->ClearRenderTarget();

		for (auto& cmd : scene.m_DrawCommands)
		{
			m_CommandList->SetShader(cmd.m_Shader->GetPass(m_PassName));
			m_CommandList->SetConstantBuffer(0, scene.m_MainCamera);
			m_CommandList->DrawMesh(cmd.m_Mesh, cmd.m_InstanceBuffer);
		}

		m_CommandList->Present();
		m_CommandList->Close();
	}
}


