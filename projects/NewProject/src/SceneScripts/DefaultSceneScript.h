#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/Scene/Scene.h"
#include "Engine/Core/Scene/SceneScriptBase.h"

namespace Game
{
	class InputSystem;
}

namespace Game
{
	CLASS(GROUP = SceneScript) DefaultSceneScript : public Engine::SceneScriptBase
	{
	public:
		REFLECTED_BODY(Game::DefaultSceneScript);

	private:
		InputSystem* m_InputSystem;
	
	public:
		virtual void OnUpdate() override;

		virtual void OnEvent(Engine::Event* e) override;

		virtual void OnScenePreLoad() override;
		virtual void OnSceneLoad() override;

		virtual void GenerateUpdateEvents() override;

	private:
	};
}
