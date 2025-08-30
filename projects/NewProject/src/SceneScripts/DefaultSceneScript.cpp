#include "DefaultSceneScript.h"
#include "Engine/Core/Scene/TransformComponent.h"
#include "Renderer/SceneRenderer/SceneRenderer.h"
#include "Input/InputSystem.h"

#if defined(EDITOR)
#include "Editor/EditorLayer.h"
#endif

#if defined(EDITOR)
#define IS_PLAYING if (Editor::EditorLayer::Get()->IsPlaying())
#else
#define IS_PLAYING
#endif

namespace Game
{

	void DefaultSceneScript::OnUpdate()
	{

	}

	void DefaultSceneScript::OnEvent(Engine::Event* e)
	{
		IS_PLAYING {
			m_InputSystem->OnEvent(e);
		}
	}

	void DefaultSceneScript::OnScenePreLoad()
	{
		SetSceneRenderer(SceneRenderer::Create());

		Utils::Vector<const Reflect::Class*> systems
		{
			&InputSystem::GetStaticClass()
		};

		m_Scene->CreateSceneSystems(systems);

		m_InputSystem = m_Scene->GetSceneSystem<InputSystem>();
	}

	void DefaultSceneScript::OnSceneLoad()
	{
	}

	void DefaultSceneScript::GenerateUpdateEvents()
	{
		IS_PLAYING{
			m_Scene->AddUpdateEvent<Engine::SceneSystemUpdateEvent<InputSystem>>("HandleEvents");
		}
		m_Scene->AddUpdateEvent<Engine::ComponentUpdateEvent<Engine::TransformComponent>>("UpdateTransformEvent");
		m_Scene->AddUpdateEvent<Engine::SceneUpdateEvent>("OnPreRender");
		m_Scene->AddUpdateEvent<Engine::ComponentUpdateEvent<Engine::TransformComponent>>("ClearDirtyFlag");
	}

}

