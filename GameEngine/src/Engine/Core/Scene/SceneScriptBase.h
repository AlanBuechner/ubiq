#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
	class Scene;
	class SceneRenderer;
	class Event;
}

namespace Engine
{
	class SceneScriptBase : public Reflect::Reflected
	{
	public:
		virtual void OnUpdate() {};

		virtual void OnEvent(Event* e) {};

		virtual void OnScenePreLoad() = 0;
		virtual void OnSceneLoad() {};
		virtual void GenerateUpdateEvents() = 0;

		static SceneScriptBase* GetDefultSceneScriptInstance();

	protected:
		void SetSceneRenderer(Ref<SceneRenderer> renderer);

	protected:
		Scene* m_Scene;

		friend Scene;
	};

}
