#include "pch.h"
#include "Input.h"
#include "InputControlerLayer.h"
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
			oglToUbiq[i] = i;
		}
		for (int i = (int)'0'; i <= (int)'9'; i++)
		{
			oglToUbiq[i] = i;
		}
		oglToUbiq[GLFW_KEY_SPACE] = KEYCODE_SPACE;
		oglToUbiq[GLFW_KEY_ENTER] = KEYCODE_ENTER;
		oglToUbiq[GLFW_KEY_LEFT_ALT] = KEYCODE_LEFT_ALT;
		oglToUbiq[GLFW_KEY_RIGHT_ALT] = KEYCODE_RIGHT_ALT;
		oglToUbiq[GLFW_KEY_LEFT_SHIFT] = KEYCODE_LEFT_SHIFT;
		oglToUbiq[GLFW_KEY_RIGHT_SHIFT] = KEYCODE_RIGHT_SHIFT;
		oglToUbiq[GLFW_KEY_LEFT_CONTROL] = KEYCODE_LEFT_CONTROL;
		oglToUbiq[GLFW_KEY_RIGHT_CONTROL] = KEYCODE_RIGHT_CONTROL;
		oglToUbiq[GLFW_KEY_LEFT_SUPER] = KEYCODE_LEFT_SUPER;
		oglToUbiq[GLFW_KEY_RIGHT_SUPER] = KEYCODE_RIGHT_SUPER;
		oglToUbiq[GLFW_KEY_RIGHT_SUPER] = KEYCODE_RIGHT_SUPER;
		oglToUbiq[GLFW_KEY_BACKSPACE] = KEYCODE_BACKSPACE;
		oglToUbiq[GLFW_KEY_TAB] = KEYCODE_TAB;
		oglToUbiq[GLFW_KEY_CAPS_LOCK] = KEYCODE_CAPS_LOCK;
		oglToUbiq[GLFW_KEY_APOSTROPHE] = KEYCODE_APOSTROPHE;
		oglToUbiq[GLFW_KEY_SEMICOLON] = KEYCODE_SEMICOLON;
		oglToUbiq[GLFW_KEY_LEFT_BRACKET] = KEYCODE_LEFT_BRACKET;
		oglToUbiq[GLFW_KEY_RIGHT_BRACKET] = KEYCODE_RIGHT_BRACKET;
		oglToUbiq[GLFW_KEY_BACKSLASH] = KEYCODE_BACKSLASH;
		oglToUbiq[GLFW_KEY_COMMA] = KEYCODE_COMMA;
		oglToUbiq[GLFW_KEY_PERIOD] = KEYCODE_PERIOD;
		oglToUbiq[GLFW_KEY_SLASH] = KEYCODE_SLASH;
		oglToUbiq[GLFW_KEY_MINUS] = KEYCODE_MINUS;
		oglToUbiq[GLFW_KEY_EQUAL] = KEYCODE_EQUAL;
		oglToUbiq[GLFW_KEY_GRAVE_ACCENT] = KEYCODE_TILDE;
		oglToUbiq[GLFW_KEY_PAGE_UP] = KEYCODE_PAGE_UP;
		oglToUbiq[GLFW_KEY_PAGE_DOWN] = KEYCODE_PAGE_DOWN;
		oglToUbiq[GLFW_KEY_HOME] = KEYCODE_HOME;
		oglToUbiq[GLFW_KEY_END] = KEYCODE_END;
		oglToUbiq[GLFW_KEY_INSERT] = KEYCODE_INSERT;
		oglToUbiq[GLFW_KEY_DELETE] = KEYCODE_DELETE;
		// numpad
		for (int i = GLFW_KEY_KP_0; i <= GLFW_KEY_KP_9; i++)
		{
			oglToUbiq[i] = i-224;
		}
		oglToUbiq[GLFW_KEY_NUM_LOCK] = KEYCODE_NUM_LOCK;
		oglToUbiq[GLFW_KEY_KP_DECIMAL] = KEYCODE_NUM_DECIMAL;
		oglToUbiq[GLFW_KEY_KP_DIVIDE] = KEYCODE_NUM_DIVIDE;
		oglToUbiq[GLFW_KEY_KP_MULTIPLY] = KEYCODE_NUM_MULTIPLY;
		oglToUbiq[GLFW_KEY_KP_SUBTRACT] = KEYCODE_NUM_SUBTRACT;
		oglToUbiq[GLFW_KEY_KP_ADD] = KEYCODE_NUM_ADD;
		oglToUbiq[GLFW_KEY_KP_ENTER] = KEYCODE_NUM_ENTER;
		for (int i = GLFW_KEY_F1; i <= GLFW_KEY_F25; i++)
		{
			oglToUbiq[i] = i-178;
		}
	}
	inline Input::KeyState Input::GetKeyState(int keycode) { return InputControlerLayer::GetKeyState(keycode); }
	inline bool Input::GetKeyDown(int keycode) { return InputControlerLayer::GetKeyDown(keycode); }
	inline bool Input::GetKeyUp(int keycode) { return InputControlerLayer::GetKeyUp(keycode); }
	inline bool Input::GetKeyPressed(int keycode) { return InputControlerLayer::GetKeyPressed(keycode); }
	inline bool Input::GetKeyReleased(int keycode) { return InputControlerLayer::GetKeyReleased(keycode); }
}