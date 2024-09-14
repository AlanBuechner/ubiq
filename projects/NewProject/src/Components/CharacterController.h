#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/Scene/Components.h"
#include "Engine/Core/Scene/SceneCamera.h"
#include "Input/InputSystem.h"


namespace Game
{
	CLASS(GROUP = Component) CharacterController : public Engine::Component
	{
	public:
		REFLECTED_BODY(Game::CharacterController);

	public:

		virtual void OnComponentAdded() override;
		virtual void OnComponentRemoved() override;

		void OnInput(InputState& state);

	private:
		InputLayer* m_Layer;
	};
}
