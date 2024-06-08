#include "pch.h"
#include "Input.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "InputControlerManeger.h"

Engine::Input* Engine::Input::s_Instance = new Input;

namespace Engine
{
	KeyCode* KeyCode::s_Instance = new KeyCode();

	KeyCode::KeyCode()
	{
		if (s_Instance == nullptr)
			s_Instance = this;
	}

	bool Input::OnKeyPressed(KeyPressedEvent* e)
	{ 
		int keycode = e->GetKeyCode();
		if (m_KeyStates[keycode] != Down)
		{
			SetKeyState(keycode, KeyPressed);
			m_ToUpdate.Push(keycode);
			m_KeysDown.Push(keycode);
		}
		return false; 
	}

	bool Input::OnKeyReleased(KeyReleasedEvent* e) 
	{
		int keycode = e->GetKeyCode();
		SetKeyState(keycode, KeyReleased);
		m_ToUpdate.Push(keycode);
		int* toRemove = m_KeysDown.end();
		for (int* it = m_KeysDown.begin(); it < m_KeysDown.end(); it++)
		{
			if (*it == keycode)
			{
				toRemove = it;
				break;
			}
		}
		if(toRemove != m_KeysDown.end())
			m_KeysDown.Remove(toRemove);
		return false; 
	}

	bool Input::OnMousePressed(MouseButtonPressedEvent* e)
	{
		int button = e->GetMouseButton();
		SetMouseButtonState(button, MousePressed);
		m_ToUpdateMouse.Push(button);
		return false;
	}

	bool Input::OnMouseReleased(MouseButtonReleasedEvent* e)
	{
		int button = e->GetMouseButton();
		SetMouseButtonState(button, MouseReleased);
		m_ToUpdateMouse.Push(button);
		return false;
	}

	bool Input::OnMouseMoved(MouseMovedEvent* e)
	{
		if(e->GetMouseBindMode() == MouseMoveBindMode::CurserPosition)
			m_MousePosition = { e->GetX(), e->GetY() };
		return false;
	}

	void Input::UpdateKeyStateImpl()
	{
		// keyboard
		for (int i : m_ToUpdate)
		{
			int state = GetKeyState(i);
			if (state == Input::KeyPressed)
			{
				SetKeyState(i, Input::Down);
			}
			else if (state == Input::KeyReleased)
			{
				SetKeyState(i, Input::Up);
			}
		}
		m_ToUpdate.Clear();

		// mouse
		for (int i : m_ToUpdateMouse)
		{
			int state = GetMouseButtonState(i);
			if (state == Input::MousePressed)
			{
				SetMouseButtonState(i, Input::Down);
			}
			else if (state == Input::MouseReleased)
			{
				SetMouseButtonState(i, Input::Up);
			}
		}
		m_ToUpdateMouse.Clear();
		m_PreviousMousePosition = m_MousePosition;
	}

	void Input::GetUpdatedEventList(Utils::Vector<Event*>& events)
	{
		for (auto i : s_Instance->m_KeysDown)
		{
			if (s_Instance->isKeyBinded(i, KeyDown))
			{
				events.Push(new KeyDownEvent(i));
			}
		}
	}

	void Input::BindKey(int* key, int* type)
	{
		s_Instance->m_BindedKeys.Push({ key, type });
	}

	void Input::UnbindKey(int key, int type)
	{
		int index = 0;
		for (BindedKeyData i : s_Instance->m_BindedKeys)
		{
			if (*(i.Key) == key && *(i.Type) == type)
			{
				s_Instance->m_BindedKeys.Remove(index);
				return;
			}
			index++;
		}
	}
	
	bool Input::isKeyBinded(int key, int type)
	{
		for (BindedKeyData i : m_BindedKeys)
		{
			if (*(i.Key) == key && *(i.Type) == type)
			{
				return true;
			}
		}
		return false;
	}


	Input::KeyState Input::GetKeyStateImpl(int keycode)
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
		if (l == KeyPressed || r == KeyPressed)
			return KeyPressed;
		if (l == KeyReleased || r == KeyReleased)
			return KeyReleased;
		return Up;
	}
}
