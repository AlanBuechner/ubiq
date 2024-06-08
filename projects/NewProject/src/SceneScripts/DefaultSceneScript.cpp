#include "DefaultSceneScript.h"
#include "Engine/Core/Scene/TransformComponent.h"
#include "Renderer/SceneRenderer/SceneRenderer.h"

namespace Game
{

	void DefaultSceneScript::OnUpdate()
	{

	}

	void DefaultSceneScript::OnScenePreLoad()
	{
		SetSceneRenderer(SceneRenderer::Create());

		m_Scene->AddUpdateEvent<Engine::ComponentUpdateEvent<Engine::TransformComponent>>("UpdateHierarchyGlobalTransform");
		m_Scene->AddUpdateEvent<Engine::SceneUpdateEvent>("OnPreRender");
	}

	void DefaultSceneScript::OnSceneLoad()
	{
	}

}

