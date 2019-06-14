#include "pch.h"
#include "Input.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Log.h"
#include "InputControlerManeger.h"

#include <GLFW/glfw3.h>

namespace Engine
{
	KeyCode* KeyCode::s_Instance = new KeyCode();

	KeyCode::KeyCode()
	{
		if (s_Instance == nullptr)
		{
			s_Instance = this;
		}

		// init ogltoubiq keycodes
		for (int i = (int)'A'; i <= (int)'Z'; i++)
		{
			m_OglToUbiq[i] = i;
		}
		for (int i = (int)'0'; i <= (int)'9'; i++)
		{
			m_OglToUbiq[i] = i;
		}
		m_OglToUbiq[GLFW_KEY_SPACE] = KEYCODE_SPACE;
		m_OglToUbiq[GLFW_KEY_ENTER] = KEYCODE_ENTER;
		m_OglToUbiq[GLFW_KEY_LEFT_ALT] = KEYCODE_LEFT_ALT;
		m_OglToUbiq[GLFW_KEY_RIGHT_ALT] = KEYCODE_RIGHT_ALT;
		m_OglToUbiq[GLFW_KEY_LEFT_SHIFT] = KEYCODE_LEFT_SHIFT;
		m_OglToUbiq[GLFW_KEY_RIGHT_SHIFT] = KEYCODE_RIGHT_SHIFT;
		m_OglToUbiq[GLFW_KEY_LEFT_CONTROL] = KEYCODE_LEFT_CONTROL;
		m_OglToUbiq[GLFW_KEY_RIGHT_CONTROL] = KEYCODE_RIGHT_CONTROL;
		m_OglToUbiq[GLFW_KEY_LEFT_SUPER] = KEYCODE_LEFT_SUPER;
		m_OglToUbiq[GLFW_KEY_RIGHT_SUPER] = KEYCODE_RIGHT_SUPER;
		m_OglToUbiq[GLFW_KEY_RIGHT_SUPER] = KEYCODE_RIGHT_SUPER;
		m_OglToUbiq[GLFW_KEY_BACKSPACE] = KEYCODE_BACKSPACE;
		m_OglToUbiq[GLFW_KEY_TAB] = KEYCODE_TAB;
		m_OglToUbiq[GLFW_KEY_CAPS_LOCK] = KEYCODE_CAPS_LOCK;
		m_OglToUbiq[GLFW_KEY_APOSTROPHE] = KEYCODE_APOSTROPHE;
		m_OglToUbiq[GLFW_KEY_SEMICOLON] = KEYCODE_SEMICOLON;
		m_OglToUbiq[GLFW_KEY_LEFT_BRACKET] = KEYCODE_LEFT_BRACKET;
		m_OglToUbiq[GLFW_KEY_RIGHT_BRACKET] = KEYCODE_RIGHT_BRACKET;
		m_OglToUbiq[GLFW_KEY_BACKSLASH] = KEYCODE_BACKSLASH;
		m_OglToUbiq[GLFW_KEY_COMMA] = KEYCODE_COMMA;
		m_OglToUbiq[GLFW_KEY_PERIOD] = KEYCODE_PERIOD;
		m_OglToUbiq[GLFW_KEY_SLASH] = KEYCODE_SLASH;
		m_OglToUbiq[GLFW_KEY_MINUS] = KEYCODE_MINUS;
		m_OglToUbiq[GLFW_KEY_EQUAL] = KEYCODE_EQUAL;
		m_OglToUbiq[GLFW_KEY_GRAVE_ACCENT] = KEYCODE_TILDE;
		m_OglToUbiq[GLFW_KEY_PAGE_UP] = KEYCODE_PAGE_UP;
		m_OglToUbiq[GLFW_KEY_PAGE_DOWN] = KEYCODE_PAGE_DOWN;
		m_OglToUbiq[GLFW_KEY_HOME] = KEYCODE_HOME;
		m_OglToUbiq[GLFW_KEY_END] = KEYCODE_END;
		m_OglToUbiq[GLFW_KEY_INSERT] = KEYCODE_INSERT;
		m_OglToUbiq[GLFW_KEY_DELETE] = KEYCODE_DELETE;
		// numpad
		for (int i = GLFW_KEY_KP_0; i <= GLFW_KEY_KP_9; i++)
		{
			m_OglToUbiq[i] = i-224;
		}
		m_OglToUbiq[GLFW_KEY_NUM_LOCK] = KEYCODE_NUM_LOCK;
		m_OglToUbiq[GLFW_KEY_KP_DECIMAL] = KEYCODE_NUM_DECIMAL;
		m_OglToUbiq[GLFW_KEY_KP_DIVIDE] = KEYCODE_NUM_DIVIDE;
		m_OglToUbiq[GLFW_KEY_KP_MULTIPLY] = KEYCODE_NUM_MULTIPLY;
		m_OglToUbiq[GLFW_KEY_KP_SUBTRACT] = KEYCODE_NUM_SUBTRACT;
		m_OglToUbiq[GLFW_KEY_KP_ADD] = KEYCODE_NUM_ADD;
		m_OglToUbiq[GLFW_KEY_KP_ENTER] = KEYCODE_NUM_ENTER;
		for (int i = GLFW_KEY_F1; i <= GLFW_KEY_F25; i++)
		{
			m_OglToUbiq[i] = i-178;
		}
		// mouse
		m_OglToUbiqMouse[GLFW_MOUSE_BUTTON_LEFT] = MOUSE_LBUTTON;
		m_OglToUbiqMouse[GLFW_MOUSE_BUTTON_RIGHT] = MOUSE_RBUTTON;
		m_OglToUbiqMouse[GLFW_MOUSE_BUTTON_MIDDLE] = MOUSE_MBUTTON;
	}

	inline Input::KeyState Input::GetKeyStateImpl(int keycode) { return m_KeyStates[keycode]; }
	inline bool Input::GetKeyDown(int keycode) { return (GetKeyState(keycode) == Input::Down); }
	inline bool Input::GetKeyUp(int keycode) { return (GetKeyState(keycode) == Input::Up); }
	inline bool Input::GetKeyPressed(int keycode) { return (GetKeyState(keycode) == Input::KeyPressed); }
	inline bool Input::GetKeyReleased(int keycode) { return (GetKeyState(keycode) == Input::KeyReleased); }

	inline Input::KeyState Input::GetMouseButtonStateImpl(int button) { return m_MouseStates[button]; }
	inline bool Input::GetMouseButtonDown(int button) { return (GetMouseButtonState(button) == Input::Down); }
	inline bool Input::GetMouseButtonUp(int button) { return (GetMouseButtonState(button) == Input::Up); }
	inline bool Input::GetMouseButtonPressed(int button) { return (GetMouseButtonState(button) == Input::MousePressed); }
	inline bool Input::GetMouseButtonReleased(int button) { return (GetMouseButtonState(button) == Input::MouseReleased); }

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
	}

	void Input::GetUpdatedEventList(std::vector<Event*>& events)
	{
		for (auto i : s_Instance->m_KeysDown)
		{
			if (s_Instance->isKeyBinded(i))
			{
				events.push_back(new KeyDownEvent(i));
			}
		}
	}

	void Input::BindKey(int * key)
	{
		s_Instance->m_bindedKeys.push_back(key);
	}

	void Input::UnbindKey(int key)
	{
		int index = 0;
		for (int* i : s_Instance->m_bindedKeys)
		{
			if (*i == key)
			{
				s_Instance->m_bindedKeys.erase(s_Instance->m_bindedKeys.begin() + index);
				return;
			}
			index++;
		}
	}
	
	inline bool Input::isKeyBinded(int key)
	{
		for (int* i : m_bindedKeys)
		{
			if (*i == key)
			{
				return true;
			}
		}
		return false;
	}
}