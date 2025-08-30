#include "pch.h"
#include "SceneRenderer.h"
#include "Engine/Renderer/Renderer.h"

#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/Abstractions/Resources/ConstantBuffer.h"
#include "Engine/Renderer/Abstractions/CommandList.h"
#include "Engine/Renderer/Abstractions/Resources/FrameBuffer.h"
#include "Engine/Renderer/RenderGraph.h"
#include "Renderer/Lighting/DirectionalLight.h"
#include "Renderer/RenderGraph/RenderGraph.h"

namespace Game
{

	// scene renderer
	SceneRenderer::SceneRenderer()
	{
		m_RenderGraph = Engine::CreateRef<RenderGraph>();
	}

	void SceneRenderer::OnViewportResize(uint32 width, uint32 height)
	{
		m_RenderGraph->OnViewportResize(width, height);
	}

	void SceneRenderer::SetDirectionalLight(Engine::Ref<DirectionalLight> light)
	{
		SceneData& data = m_RenderGraph->As<RenderGraph>().GetScene();
		data.m_DirectinalLight = light;
	}

	void SceneRenderer::SetSkyBox(Engine::Ref<Engine::Texture2D> texture)
	{
		SceneData& data = m_RenderGraph->As<RenderGraph>().GetScene();
		data.m_Skybox = texture;
	}

	void SceneRenderer::UpdateBuffers()
	{
		m_MainPassObject.UpdateBuffers();
	}

	void SceneRenderer::Build()
	{

		CREATE_PROFILE_SCOPEI("Build command lists");

		SceneData& data = m_RenderGraph->As<RenderGraph>().GetScene();
		data.m_MainCamera = m_MainCamera;

		// compile commands
		m_MainPassObject.BuildDrawCommands(data.m_DrawCommands);

		m_RenderGraph->Build();
		m_RenderGraph->Submit();
	}

	Engine::Ref<SceneRenderer> SceneRenderer::Create()
	{
		return Engine::CreateRef<SceneRenderer>();
	}
}

