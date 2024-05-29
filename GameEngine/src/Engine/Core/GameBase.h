#pragma once
#include "Core.h"
#include "Logging/Log.h"
#include "Scene/Scene.h"

namespace Engine
{
	class GameBase
	{
		static GameBase* s_Game;
	public:
		static GameBase* Get() { return s_Game; }
		template<class T>
		static T* GetAs() { return (T*)Get(); }

	protected:
		Ref<Scene> m_Scene;

	protected:
		GameBase();
	public:

		Ref<Scene> GetScene() { return m_Scene; }

		Ref<Scene> LoadScene(const std::string& file);
		void SwitchScene(Ref<Scene> scene);

		virtual void OnUpdate(Ref<Camera> overideCamera = nullptr) = 0;
		virtual void OnRender() = 0;

		virtual Ref<SceneRenderer> CreateSceneRenderer() = 0;
	protected:
		virtual void OnSceneLoad(Ref<Scene> scene) {};
		virtual void OnSceneSwitch(Ref<Scene> scene) {};

	};
}

extern Engine::GameBase* CreateGame();
