#pragma once
#include "Engine/Core/Scene/SceneSystem.h"

namespace Game
{
	CLASS(GROUP = SceneSystem) InputSystem : public Engine::SceneSystem
	{
	public:
		REFLECTED_BODY(Game::InputSystem);
	};
}
