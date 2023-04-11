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

	void ShaderPassNode::AddToCommandQueue(Ref<ExecutionOrder> order)
	{
		std::vector<Ref<CommandList>> dependencies;
		for (auto& cmdList : m_RenderTarget.GetInput()->GetCommandLists())
			dependencies.push_back(cmdList);
		for (auto& node : m_Dependincys)
		{
			for(auto& cmdList : node->GetCommandLists())
				dependencies.push_back(cmdList);
		}
		order->Add(m_CommandList, dependencies);
	}

	void ShaderPassNode::BuildImpl()
	{
		Ref<FrameBuffer> renderTarget = m_RenderTarget.GetVarAndBuild();
		const SceneData& scene = m_Graph.GetScene();
		
		m_CommandList->StartRecording();

		GPUTimer::BeginEvent(m_CommandList, "Shader Pass");
		m_CommandList->SetRenderTarget(renderTarget);

		for (auto& cmd : scene.m_DrawCommands)
		{
			Ref<ShaderPass> pass = cmd.m_Shader->GetPass(m_PassName);
			if (pass)
			{
				m_CommandList->SetShader(pass);
				m_CommandList->SetRootConstant(pass->GetUniformLocation("RC_MainCameraIndex"), scene.m_MainCamera->GetCameraBuffer()->GetDescriptorLocation());
				m_CommandList->SetConstantBuffer(pass->GetUniformLocation("DirLight"), scene.m_DirectinalLight->GetBuffer());
				m_CommandList->SetConstantBuffer(pass->GetUniformLocation("Cascades"), scene.m_DirectinalLight->GetShadowMaps()[scene.m_MainCamera].m_CameraIndeces);
				m_CommandList->DrawMesh(cmd.m_Mesh, cmd.m_InstanceBuffer);
			}
		}

		GPUTimer::EndEvent(m_CommandList);
		m_CommandList->Close();
	}
}


