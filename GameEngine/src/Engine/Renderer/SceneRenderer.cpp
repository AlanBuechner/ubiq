#include "SceneRenderer.h"
#include "Renderer.h"

namespace Engine
{

	void SceneRenderer::OnViewportResize(uint32 width, uint32 height)
	{
		for (auto& node : m_Nodes)
			node->OnViewportResize(width, height);
	}

	void SceneRenderer::BuildCommands()
	{
		CREATE_PROFILE_SCOPEI("Build command lists");

		for (Ref<CPUCommandList> list : m_CommandLists)
			list->StartRecording();

		for (auto& node : m_Nodes)
			node->Build();

		for (Ref<CPUCommandList> list : m_CommandLists)
			list->StopRecording();
	}

	void SceneRenderer::Submit()
	{
		Renderer::SubmitCommandLists(m_CommandLists);
	}

}

