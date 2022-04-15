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

	bool Input::OnKeyPressed(KeyPressedEvent& e)
	{ 
		int keycode = e.GetKeyCode();
		SetKeyState(keycode, KeyPressed); 
		m_ToUpdate.push_back(keycode);
		m_KeysDown.push_back(keycode);
		return false; 
	}

	bool Input::OnKeyReleased(KeyReleasedEvent& e) 
	{
		int keycode = e.GetKeyCode();
		SetKeyState(keycode, KeyReleased); 
		m_ToUpdate.push_back(keycode);
		std::vector<int>::const_iterator toRemove;
		for (std::vector<int>::const_iterator it = m_KeysDown.begin(); it < m_KeysDown.end(); it++)
		{
			if (*it == keycode)
			{
				toRemove = it;
				break;
			}
		}
		m_KeysDown.erase(toRemove);
		return false; 
	}

	bool Input::OnMousePressed(MouseButtonPressedEvent& e)
	{
		int button = e.GetMouseButton();
		SetMouseButtonState(button, MousePressed);
		m_ToUpdateMouse.push_back(button);
		return false;
	}

	bool Input::OnMouseReleased(MouseButtonReleasedEvent& e)
	{
		int button = e.GetMouseButton();
		SetMouseButtonState(button, MouseReleased);
		m_ToUpdateMouse.push_back(button);
		return false;
	}

	bool Input::OnMouseMoved(MouseMovedEvent& e)
	{
		if(e.GetMouseBindMode() == MouseMoveBindMode::CurserPosition)
			m_MousePosition = { e.GetX(), e.GetY() };
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
		m_ToUpdate.clear();

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
		m_ToUpdateMouse.clear();
		m_PreviousMousePosition = m_MousePosition;
	}

	void Input::GetUpdatedEventList(std::vector<Event*>& events)
	{
		std::vector<Event*> newEventList;
		if (!s_Instance->m_SendAllEventData)
		{
			for (auto e : events)
			{
				if ((e->GetCategoryFlags() & (EventCategoryInput | EventCategoryKeyboard | EventCategoryMouse | EventCategoryMouseButton)) != 0)
				{
					EventType eventType = e->GetEventType();
					for (auto i : s_Instance->m_BindedKeys)
					{
						if ((int)eventType == *(i.Type))
						{
							bool send = false;

							int key = 0;
							switch (eventType)
							{
							case EventType::KeyPressed:
								if (s_Instance->m_SystemsNeedingAllPressedEvents > 0) { send = true; break; }
								key = ((KeyPressedEvent*)e)->GetKeyCode();
								break;
							case EventType::KeyReleased:
								if (s_Instance->m_SystemsNeedingAllRelesedEvents > 0) { send = true; break; }
								key = ((KeyReleasedEvent*)e)->GetKeyCode();
								break;
							case EventType::KeyTyped:
								if (s_Instance->m_SystemsNeedingAllTypedEvents > 0) { send = true; break; }
								key = ((KeyTypedEvent*)e)->GetKeyCode();
								break;
							case EventType::KeyRepeat:
								if (s_Instance->m_SystemsNeedingAllRepeatEvents > 0) { send = true; break; }
								key = ((KeyRepeatEvent*)e)->GetKeyCode();
								break;
							case EventType::MouseButtonPressed:
								if (s_Instance->m_SystemsNeedingAllPressedEvents > 0) { send = true; break; }
								key = ((MouseButtonPressedEvent*)e)->GetMouseButton();
								break;
							case EventType::MouseButtonReleased:
								if (s_Instance->m_SystemsNeedingAllRelesedEvents > 0) { send = true; break; }
								key = ((MouseButtonReleasedEvent*)e)->GetMouseButton();
								break;
							}

							if (send || key == *(i.Key))
							{
								newEventList.push_back(e);
							}
						}
					}
					if (eventType == EventType::MouseMoved && s_Instance->m_SystemsNeedingAllMouseMoveEvents > 0)
					{
						newEventList.push_back(e);
					}
				}
			}
		}
		else
		{
			newEventList = events;
		}

		for (auto i : s_Instance->m_KeysDown)
		{
			if (s_Instance->isKeyBinded(i, KeyDown))
			{
				newEventList.push_back(new KeyDownEvent(i));
			}
		}
		events = newEventList;
	}

	void Input::BindKey(int* key, int* type)
	{
		s_Instance->m_BindedKeys.push_back({ key, type });
	}

	void Input::UnbindKey(int key, int type)
	{
		int index = 0;
		for (BindedKeyData i : s_Instance->m_BindedKeys)
		{
			if (*(i.Key) == key && *(i.Type) == type)
			{
				s_Instance->m_BindedKeys.erase(s_Instance->m_BindedKeys.begin() + index);
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
		case KeyCode::ALT:
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
			break;
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
