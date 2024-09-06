#include "DefaultSceneScript.h"
#include "Engine/Core/Scene/TransformComponent.h"
#include "Renderer/SceneRenderer/SceneRenderer.h"
#include "Input/InputSystem.h"

namespace Game
{

	void DefaultSceneScript::OnUpdate()
	{

	}

	void DefaultSceneScript::OnEvent(Engine::Event* e)
	{
#if !defined(EDITOR)
		m_Scene->GetSceneSystem<InputSystem>()->OnEvent(e);
#endif
	}

	void DefaultSceneScript::OnScenePreLoad()
	{
		SetSceneRenderer(SceneRenderer::Create());

		std::vector<const Reflect::Class*> systems
		{
			&InputSystem::GetStaticClass()
		};

		m_Scene->CreateSceneSystems(systems);
	}

	void DefaultSceneScript::OnSceneLoad()
	{
	}

	void DefaultSceneScript::GenerateUpdateEvents()
	{
		m_Scene->AddUpdateEvent<Engine::ComponentUpdateEvent<Engine::TransformComponent>>("UpdateHierarchyGlobalTransform");
		m_Scene->AddUpdateEvent<Engine::SceneUpdateEvent>("OnPreRender");
	}

}

