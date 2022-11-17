#include "pch.h"
#include "GBufferPassNode.h"
#include "Engine/Renderer/GPUProfiler.h"

namespace Engine
{
	GBufferPassNode::GBufferPassNode(RenderGraph& graph) :
		RenderGraphNode(graph)
	{
		m_CommandList = CommandList::Create(CommandList::Direct);
	}

	void GBufferPassNode::SetRenderTarget(FrameBufferVar var)
	{
		m_RenderTarget = var;
	}

	void GBufferPassNode::AddToCommandQueue()
	{
		std::vector<Ref<CommandList>> dependencies;
		for (auto& cmdList : m_RenderTarget.GetInput()->GetCommandLists())
			dependencies.push_back(cmdList);
		Renderer::GetMainCommandQueue()->AddCommandList(m_CommandList, dependencies);
	}

	void GBufferPassNode::BuildImpl()
	{
		Ref<FrameBuffer> renderTarget = m_RenderTarget.GetVar();
		const SceneData& scene = m_Graph.GetScene();

		m_CommandList->StartRecording();

		// depth pre pass
		GPUTimer::BeginEvent(m_CommandList, "Depth pre pass");
		// set up render target
		m_CommandList->ToRenderTarget(renderTarget, FrameBufferState::Common);
		m_CommandList->SetRenderTarget(renderTarget);
		m_CommandList->ClearRenderTarget();

		// render all objects
		for (auto& cmd : scene.m_DrawCommands)
		{
			Ref<ShaderPass> pass = cmd.m_Shader->GetPass("depth");
			m_CommandList->SetShader(pass);
			m_CommandList->SetRootConstant(pass->GetUniformLocation("RC_MainCameraIndex"), scene.m_MainCamera->GetCameraBuffer()->GetDescriptorLocation());
			m_CommandList->DrawMesh(cmd.m_Mesh, cmd.m_InstanceBuffer);
		}
		GPUTimer::EndEvent(m_CommandList); // end depth pre pass

		// begin gbuffer pass
		GPUTimer::BeginEvent(m_CommandList, "GBuffer pass");
		GPUTimer::EndEvent(m_CommandList); // end gbuffer pass

		m_CommandList->Close();
	}
}

