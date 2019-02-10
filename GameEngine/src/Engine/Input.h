#pragma once

#include "core.h"

#ifdef PLATFORM_WINDOWS
	#include "Platform/Windows/WindowsKeyCodes.h"
#endif

namespace Engine
{
	class ENGINE_API KeyCode
	{
	public:
		static int GetKeyCode(char key) { return s_Instance->GetKeyCodeImpl(key); }

		enum
		{
			UNKNOWN = KEYCODE_UNKNOWN,
			SPACE = KEYCODE_SPACE,
			APOSTROPHE = KEYCODE_APOSTROPHE,
			COMMA = KEYCODE_COMMA,
			MINUS = KEYCODE_MINUS,
			PERIOD = KEYCODE_PERIOD,
			SLASH = KEYCODE_SLASH,
			NUM0 = KEYCODE_0,
			NUM1 = KEYCODE_1,
			NUM2 = KEYCODE_2,
			NUM3 = KEYCODE_3,
			NUM4 = KEYCODE_4,
			NUM5 = KEYCODE_5,
			NUM6 = KEYCODE_6,
			NUM7 = KEYCODE_7,
			NUM8 = KEYCODE_8,
			NUM9 = KEYCODE_9,
			SEMICOLON = KEYCODE_SEMICOLON,
			EQUAL = KEYCODE_EQUAL,
			A = KEYCODE_A,
			B = KEYCODE_B,
			C = KEYCODE_C,
			D = KEYCODE_D,
			E = KEYCODE_E,
			F = KEYCODE_F,
			G = KEYCODE_G,
			H = KEYCODE_H,
			I = KEYCODE_I,
			J = KEYCODE_J,
			K = KEYCODE_K,
			L = KEYCODE_L,
			M = KEYCODE_M,
			N = KEYCODE_N,
			O = KEYCODE_O,
			P = KEYCODE_P,
			Q = KEYCODE_Q,
			R = KEYCODE_R,
			S = KEYCODE_S,
			T = KEYCODE_T,
			U = KEYCODE_U,
			V = KEYCODE_V,
			W = KEYCODE_W,
			X = KEYCODE_X,
			Y = KEYCODE_Y,
			Z = KEYCODE_Z,
			LEFT_BRACKET = KEYCODE_LEFT_BRACKET,
			BACKSLASH = KEYCODE_BACKSLASH,
			RIGHT_BRACKET = KEYCODE_RIGHT_BRACKET,
			GRAVE_ACCENT = KEYCODE_GRAVE_ACCENT,
			WORLD_1 = KEYCODE_WORLD_1,
			WORLD_2 = KEYCODE_WORLD_2,
			ESCAPE = KEYCODE_ESCAPE,
			ENTER = KEYCODE_ENTER,
			TAB = KEYCODE_TAB,
			BACKSPACE = KEYCODE_BACKSPACE,
			INSERT = KEYCODE_INSERT,
			//DELETE = KEYCODE_DELETE,
			RIGHT = KEYCODE_RIGHT,   
			LEFT = KEYCODE_LEFT,
			DOWN = KEYCODE_DOWN,
			UP = KEYCODE_UP,
			PAGE_UP = KEYCODE_PAGE_UP,
			PAGE_DOWN = KEYCODE_PAGE_DOWN,
			HOME = KEYCODE_HOME,
			END = KEYCODE_END,
			CAPS_LOCK = KEYCODE_CAPS_LOCK,
			SCROLL_LOCK = KEYCODE_SCROLL_LOCK,
			NUM_LOCK = KEYCODE_NUM_LOCK,
			PRINT_SCREEN = KEYCODE_PRINT_SCREEN,
			PAUSE = KEYCODE_PAUSE,
			F1 = KEYCODE_F1,
			F2 = KEYCODE_F2,
			F3 = KEYCODE_F3,
			F4 = KEYCODE_F4,
			F5 = KEYCODE_F5,
			F6 = KEYCODE_F6,
			F7 = KEYCODE_F7,
			F8 = KEYCODE_F8,
			F9 = KEYCODE_F9,
			F10 = KEYCODE_F10,
			F11 = KEYCODE_F11,
			F12 = KEYCODE_F12,
			F13 = KEYCODE_F13,
			F14 = KEYCODE_F14,
			F15 = KEYCODE_F15,
			F16 = KEYCODE_F16,
			F17 = KEYCODE_F17,
			F18 = KEYCODE_F18,
			F19 = KEYCODE_F19,
			F20 = KEYCODE_F20,
			F21 = KEYCODE_F21,
			F22 = KEYCODE_F22,
			F23 = KEYCODE_F23,
			F24 = KEYCODE_F24,
			F25 = KEYCODE_F25,
			KP_0 = KEYCODE_KP_0,
			KP_1 = KEYCODE_KP_1,
			KP_2 = KEYCODE_KP_2,
			KP_3 = KEYCODE_KP_3,
			KP_4 = KEYCODE_KP_4,
			KP_5 = KEYCODE_KP_5,
			KP_6 = KEYCODE_KP_6,
			KP_7 = KEYCODE_KP_7,
			KP_8 = KEYCODE_KP_8,
			KP_9 = KEYCODE_KP_9,
			KP_DECIMAL = KEYCODE_KP_DECIMAL,
			KP_DIVIDE = KEYCODE_KP_DIVIDE,
			KP_MULTIPLY = KEYCODE_KP_MULTIPLY,
			KP_SUBTRACT = KEYCODE_KP_SUBTRACT,
			KP_ADD = KEYCODE_KP_ADD,
			KP_ENTER = KEYCODE_KP_ENTER,
			KP_EQUAL = KEYCODE_KP_EQUAL,
			LEFT_SHIFT = KEYCODE_LEFT_SHIFT,
			LEFT_CONTROL = KEYCODE_LEFT_CONTROL,
			LEFT_ALT = KEYCODE_LEFT_ALT,
			LEFT_SUPER = KEYCODE_LEFT_SUPER,
			RIGHT_SHIFT = KEYCODE_RIGHT_SHIFT,
			RIGHT_CONTROL = KEYCODE_RIGHT_CONTROL,
			RIGHT_ALT = KEYCODE_RIGHT_ALT,
			RIGHT_SUPER = KEYCODE_RIGHT_SUPER,
			MENU = KEYCODE_MENU,
			LAST = KEYCODE_LAST
		};

	protected:
		virtual int GetKeyCodeImpl(char key) = 0;

		

	private:
		static KeyCode* s_Instance;
	};

	class ENGINE_API Input
	{
	public:
		inline static bool IsKeyPressed(int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }

		inline static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }
		inline static std::pair<float, float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); }
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }

	protected:
		virtual bool IsKeyPressedImpl(int Keycode) = 0;
		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;

	private:
		static Input* s_Instance;

	};
}