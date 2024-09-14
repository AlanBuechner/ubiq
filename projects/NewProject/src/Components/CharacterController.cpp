#include "CharacterController.h"

namespace Game
{

	void CharacterController::OnComponentAdded()
	{
		m_Layer = Owner.GetScene()->GetSceneSystem<InputSystem>()->GetLayer("Game");
		m_Layer->BindFunc(BIND_EVENT_FN(&CharacterController::OnInput));
	}

	void CharacterController::OnComponentRemoved()
	{
		m_Layer->UnbindFunc(BIND_EVENT_FN(&CharacterController::OnInput));
	}

	void CharacterController::OnInput(InputState& state)
	{
		if(state.GetKeyPressed(Engine::KeyCode::W))
			DEBUG_INFO("hello world");
	}

}

