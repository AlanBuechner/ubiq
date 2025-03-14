#pragma once

#include "Engine/Core/Core.h"
#include "KeyCodes.h"
#include "Engine/Core/Application.h"
#include "Engine/Math/Math.h"

#include <map>

namespace Engine
{
	class KeyPressedEvent;
	class KeyReleasedEvent;
	class MouseButtonPressedEvent;
	class MouseButtonReleasedEvent;
}

namespace Engine
{

	enum KeyState
	{
		Up,
		Down,
		Pressed,
		Released,
	};

	class  Input
	{
		friend Application;
		friend class Cursor;
		friend class WindowsWindow;

	public:

		Input();

		// keyboard
		inline static KeyState GetKeyState(KeyCode keycode) { return s_Instance->GetKeyStateImpl(keycode); }
		inline static bool GetKeyDown(KeyCode keycode) { return (GetKeyState(keycode) == KeyState::Down); }
		inline static bool GetKeyUp(KeyCode keycode) { return (GetKeyState(keycode) == KeyState::Up); }
		inline static bool GetKeyPressed(KeyCode keycode) { return (GetKeyState(keycode) == KeyState::Pressed); }
		inline static bool GetKeyReleased(KeyCode keycode) { return (GetKeyState(keycode) == KeyState::Released); }

		// mouse
		inline static KeyState GetMouseButtonState(MouseCode button) { return s_Instance->GetMouseButtonStateImpl(button); }
		inline static bool GetMouseButtonDown(MouseCode button) { return (GetMouseButtonState(button) == KeyState::Down); }
		inline static bool GetMouseButtonUp(MouseCode button) { return (GetMouseButtonState(button) == KeyState::Up); }
		inline static bool GetMouseButtonPressed(MouseCode button) { return (GetMouseButtonState(button) == KeyState::Pressed); }
		inline static bool GetMouseButtonReleased(MouseCode button) { return (GetMouseButtonState(button) == KeyState::Released); }

		inline static Math::Vector2 GetMousePosition() { return s_Instance->m_MousePosition; }
		inline static Math::Vector2 GetPreviousMousePosition() { return s_Instance->m_PreviousMousePosition; }

	private:
		// keyboard
		KeyState GetKeyStateImpl(KeyCode keycode);

		// mouse
		KeyState GetMouseButtonStateImpl(MouseCode button) { return m_MouseStates[button]; }

		void SetKeyState(KeyCode key, KeyState state) { m_KeyStates[key] = state; }
		void SetMouseButtonState(MouseCode button, KeyState state) { m_MouseStates[button] = state; }
		bool OnKeyPressed(KeyPressedEvent* e);
		bool OnKeyReleased(KeyReleasedEvent* e);
		bool OnMousePressed(MouseButtonPressedEvent* e);
		bool OnMouseReleased(MouseButtonReleasedEvent* e);
		bool OnMouseMoved(MouseMovedEvent* e);
		static void UpdateKeyState() { s_Instance->UpdateKeyStateImpl(); }
		void UpdateKeyStateImpl();

		inline static void SetMousePosition(Math::Vector2 pos) { s_Instance->m_PreviousMousePosition = pos; s_Instance->m_MousePosition = pos; }
		inline static void SetPrevMousePosition(Math::Vector2 pos) { s_Instance->m_PreviousMousePosition = pos; }

	private:
		static Input* s_Instance;

		KeyState m_KeyStates[Engine::KeyCode::SIZE];
		KeyState m_MouseStates[Engine::MouseCode::SIZE];

		Utils::Vector<KeyCode> m_ToUpdate;
		Utils::Vector<MouseCode> m_ToUpdateMouse;

		Math::Vector2 m_MousePosition;
		Math::Vector2 m_PreviousMousePosition;
	};
}

typedef Engine::Input Input;
