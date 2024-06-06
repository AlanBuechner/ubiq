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

	Ref<Scene> GameBase::LoadScene(const std::string& file)
	{
		Ref<Scene> scene = CreateRef<Scene>(CreateSceneScript());
		SceneSerializer serializer(scene);
		serializer.Deserialize(file);
		Window& window = Application::Get().GetWindow();
		scene->OnViewportResize(window.GetWidth(), window.GetHeight());
		scene->GetSceneScript()->OnSceneLoad();
		return scene;
	}

	void GameBase::SwitchScene(Ref<Scene> scene)
	{
		OnSceneSwitch(scene);
		m_Scene = scene;
	}

}

