#include "GameBase.h"
#include "Scene/SceneSerializer.h"
#include "Application.h"
#include "Scene/SceneScriptBase.h"

Engine::GameBase* Engine::GameBase::s_Game = nullptr;

namespace Engine
{

	GameBase::GameBase()
	{
		if (s_Game) {
			CORE_ERROR("Atempting to create new game instance");
			return;
		}

		s_Game = this;
	}


	void GameBase::SwitchScene(Ref<Scene> scene)
	{
		CREATE_PROFILE_FUNCTIONI();
		OnSceneSwitch(scene);
		m_Scene = scene;
	}

}

