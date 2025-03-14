#include "pch.h"
#include "Input.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"

Engine::Input* Engine::Input::s_Instance = new Input;

namespace Engine
{

	Input::Input()
	{
		for (uint32 i = 0; i < KeyCode::SIZE; i++)
			m_KeyStates[i] = Up;
		for (uint32 i = 0; i < MouseCode::SIZE; i++)
			m_MouseStates[i] = Up;
	}


	bool Input::OnKeyPressed(KeyPressedEvent* e)
	{ 
		int keycode = e->GetKeyCode();
		if (m_KeyStates[keycode] != Down)
		{
			SetKeyState(keycode, Pressed);
			m_ToUpdate.Push(keycode);
		}
		return false; 
	}

	bool Input::OnKeyReleased(KeyReleasedEvent* e) 
	{
		int keycode = e->GetKeyCode();
		SetKeyState(keycode, Released);
		return false; 
	}

	bool Input::OnMousePressed(MouseButtonPressedEvent* e)
	{
		int button = e->GetMouseButton();
		SetMouseButtonState(button, Pressed);
		m_ToUpdateMouse.Push(button);
		return false;
	}

	bool Input::OnMouseReleased(MouseButtonReleasedEvent* e)
	{
		int button = e->GetMouseButton();
		SetMouseButtonState(button, Released);
		m_ToUpdateMouse.Push(button);
		return false;
	}

	bool Input::OnMouseMoved(MouseMovedEvent* e)
	{
		m_MousePosition = { e->GetX(), e->GetY() };
		return false;
	}

	void Input::UpdateKeyStateImpl()
	{
		// keyboard
		for (int i : m_ToUpdate)
		{
			int state = GetKeyState(i);
			if (state == KeyState::Pressed)
				SetKeyState(i, KeyState::Down);
			else if (state == KeyState::Released)
				SetKeyState(i, KeyState::Up);
		}
		m_ToUpdate.Clear();

		// mouse
		for (int i : m_ToUpdateMouse)
		{
			int state = GetMouseButtonState(i);
			if (state == KeyState::Pressed)
				SetMouseButtonState(i, KeyState::Down);
			else if (state == KeyState::Released)
				SetMouseButtonState(i, KeyState::Up);
		}
		m_ToUpdateMouse.Clear();
		m_PreviousMousePosition = m_MousePosition;
	}

	KeyState Input::GetKeyStateImpl(KeyCode keycode)
	{
		KeyState l;
		KeyState r;
		switch (keycode)
		{
			/*case KeyCode::ALT:
				l = m_KeyStates[KeyCode::LEFT_ALT];
				r = m_KeyStates[KeyCode::RIGHT_ALT];
				break;
			case KeyCode::CONTROL:
				l = m_KeyStates[KeyCode::LEFT_CONTROL];
				r = m_KeyStates[KeyCode::RIGHT_CONTROL];
				break;
			case KeyCode::SHIFT:
				l = m_KeyStates[KeyCode::LEFT_SHIFT];
				r = m_KeyStates[KeyCode::RIGHT_SHIFT];
				break;*/
		default:
			return m_KeyStates[keycode];
			break;
		}

		if (l == Down || r == Down)
			return Down;
		if (l == Pressed || r == Pressed)
			return Pressed;
		if (l == Released || r == Released)
			return Released;
		return Up;
	}
}
