#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
	class Scene;
	class SceneRenderer;
}

namespace Engine
{
	class SceneScriptBase : public Reflect::Reflected
	{
	public:
		virtual void OnUpdate() {};

		virtual void OnScenePreLoad() {};
		virtual void OnSceneLoad() {};

		static Ref<SceneScriptBase> GetDefultSceneScriptInstance();

	protected:
		void SetSceneRenderer(Ref<SceneRenderer> renderer);

	protected:
		Scene* m_Scene;

		friend Scene;
	};

}
