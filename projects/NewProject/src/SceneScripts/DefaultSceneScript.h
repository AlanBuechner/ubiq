#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/Scene/Scene.h"
#include "Engine/Core/Scene/SceneScriptBase.h"

namespace Game
{
	CLASS(GROUP = SceneScript) DefaultSceneScript : public Engine::SceneScriptBase
	{
	public:
		REFLECTED_BODY(Game::DefaultSceneScript);
	
	public:
		virtual void OnUpdate() override;

		virtual void OnScenePreLoad() override;
		virtual void OnSceneLoad() override;

	private:
	};
}
