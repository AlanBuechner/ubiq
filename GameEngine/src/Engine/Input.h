#pragma once

#include "core.h"
#include "KeyCodes.h"
#include <map>
#include <string>
#include "Events/Event.h"
#include "Application.h"

#define KEY_PRESSED		(int)Engine::Input::KeyPressed
#define KEY_RELEASED	(int)Engine::Input::KeyReleased
#define KEY_DOWN		(int)Engine::Input::KeyDown
#define KEY_UP			(int)Engine::Input::KeyUp

#define MOUSE_PRESSED	(int)Engine::Input::MousePressed
#define MOUSE_RELEASED	(int)Engine::Input::MouseReleased

//#define IE_PRESSED		(int)Engine::Input::KeyPressed | (int)Engine::Input::MousePressed
//#define IE_RELEASED		(int)Engine::Input::KeyReleased | (int)Engine::Input::MouseReleased

namespace Engine
{
	class ENGINE_API KeyCode
	{
	public:
		KeyCode();

		static inline int GetKeyCode(std::string key) { return s_Instance->GetKeyCodeImpl(key); }
		static inline int OglToUbiq(int key) { return s_Instance->OglToUbiqImpl(key); }
		static inline int UbiqToOgl(int key) { return s_Instance->UbiqToOglImpl(key); }
		static inline int OglToUbiqMouse(int key) { return s_Instance->OglToUbiqMouseImpl(key); }

		enum
		{
			UNKNOWN = KEYCODE_UNKNOWN,
			NUM_0 = KEYCODE_0,
			NUM_1 = KEYCODE_1,
			NUM_2 = KEYCODE_2,
			NUM_3 = KEYCODE_3,
			NUM_4 = KEYCODE_4,
			NUM_5 = KEYCODE_5,
			NUM_6 = KEYCODE_6,
			NUM_7 = KEYCODE_7,
			NUM_8 = KEYCODE_8,
			NUM_9 = KEYCODE_9,
			
			NUMPAD_LOCK = KEYCODE_NUM_LOCK,
			NUMPAD_0 = KEYCODE_NUM_0,
			NUMPAD_1 = KEYCODE_NUM_1, 
			NUMPAD_2 = KEYCODE_NUM_2, 
			NUMPAD_3 = KEYCODE_NUM_3, 
			NUMPAD_4 = KEYCODE_NUM_4, 
			NUMPAD_5 = KEYCODE_NUM_5, 
			NUMPAD_6 = KEYCODE_NUM_6, 
			NUMPAD_7 = KEYCODE_NUM_7, 
			NUMPAD_8 = KEYCODE_NUM_8, 
			NUMPAD_9 = KEYCODE_NUM_9, 
			NUMPAD_DECIMAL = KEYCODE_NUM_DECIMAL,
			NUMPAD_DIVIDE = KEYCODE_NUM_DIVIDE,
			NUMPAD_MULTIPLY = KEYCODE_NUM_MULTIPLY,
			NUMPAD_SUBTRACT = KEYCODE_NUM_SUBTRACT,
			NUMPAD_ADD = KEYCODE_NUM_ADD,
			NUMPAD_ENTER = KEYCODE_NUM_ENTER,
			
			A =  KEYCODE_A,
			B =  KEYCODE_B,
			C =  KEYCODE_C,
			D =  KEYCODE_D,
			E =  KEYCODE_E,
			F =  KEYCODE_F,
			G =  KEYCODE_G,
			H =  KEYCODE_H,
			I =  KEYCODE_I,
			J =  KEYCODE_J,
			K =  KEYCODE_K,
			L =  KEYCODE_L,
			M =  KEYCODE_M,
			N =  KEYCODE_N,
			O =  KEYCODE_O,
			P =  KEYCODE_P,
			Q =  KEYCODE_Q,
			R =  KEYCODE_R,
			S =  KEYCODE_S,
			T =  KEYCODE_T,
			U =  KEYCODE_U,
			V =  KEYCODE_V,
			W =  KEYCODE_W,
			X =  KEYCODE_X,
			Y =  KEYCODE_Y,
			Z =  KEYCODE_Z,
			
			MINUS = KEYCODE_MINUS,
			PERIOD = KEYCODE_PERIOD,
			LEFT_BRACKET = KEYCODE_LEFT_BRACKET,
			RIGHT_BRACKET = KEYCODE_RIGHT_BRACKET,
			SEMICOLON = KEYCODE_SEMICOLON,
			BACKSLASH = KEYCODE_BACKSLASH,
			FORWORDSLASH = KEYCODE_FORWORDSLASH,
			SLASH = KEYCODE_SLASH,
			QUOTE = KEYCODE_QUOTE,
			APOSTROPHE = KEYCODE_APOSTROPHE,
			GRAVE_ACCENT = KEYCODE_GRAVE_ACCENT,
			TILDE = KEYCODE_TILDE,
			ESCAPE = KEYCODE_ESCAPE,
			TAB = KEYCODE_TAB,
			BACKSPACE = KEYCODE_BACKSPACE,
			SPACE = KEYCODE_SPACE,
			ENTER = KEYCODE_ENTER,
			EQUAL = KEYCODE_EQUAL,
			COMMA = KEYCODE_COMMA,
			
			LEFT = KEYCODE_LEFT,
			UP = KEYCODE_UP,
			RIGHT = KEYCODE_RIGHT,
			DOWN = KEYCODE_DOWN,

			INSERT = KEYCODE_INSERT,
			DELETE_KEY = KEYCODE_DELETE,
			HOME = KEYCODE_HOME,
			END = KEYCODE_END,
			PAGE_UP = KEYCODE_PAGE_UP,
			PAGE_DOWN = KEYCODE_PAGE_DOWN,
			
			CAPS_LOCK = KEYCODE_CAPS_LOCK,
			SELECT = KEYCODE_SELECT,
			PRINT = KEYCODE_PRINT,
			EXECUTE = KEYCODE_EXECUTE,
			HELP = KEYCODE_HELP,
			PRINT_SCREEN = KEYCODE_HELP,
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
			
			
			SHIFT = KEYCODE_SHIFT,
			LEFT_SHIFT = KEYCODE_LEFT_SHIFT,
			RIGHT_SHIFT = KEYCODE_RIGHT_SHIFT,
			
			CONTROL = KEYCODE_CONTROL,
			LEFT_CONTROL = KEYCODE_LEFT_CONTROL,
			RIGHT_CONTROL = KEYCODE_RIGHT_CONTROL,
			
			ALT = KEYCODE_ALT,
			LEFT_ALT = KEYCODE_LEFT_ALT,
			RIGHT_ALT = KEYCODE_RIGHT_ALT,
			
			LEFT_SUPER = KEYCODE_LEFT_SUPER,
			RIGHT_SUPER = KEYCODE_RIGHT_SUPER,
			
			MENU = KEYCODE_MENU,
			LAST = KEYCODE_LAST
		};

	protected:
		inline int GetKeyCodeImpl(std::string key) { return asciiToUbiq[key]; }
		inline int OglToUbiqImpl(int key) { return oglToUbiq[key]; }
		inline int UbiqToOglImpl(int key) { return ubiqToOgl[key]; }
		inline int OglToUbiqMouseImpl(int key) { return oglToUbiqMouse[key]; }

		std::map<std::string, int> asciiToUbiq;
		std::map<int, int> oglToUbiq;
		std::map<int, int> ubiqToOgl;
		std::map<int, int> oglToUbiqMouse;

	private:
		static KeyCode* s_Instance;
	};

	class ENGINE_API Input
	{
		friend Application;
	public:
		enum KeyState
		{
			KeyUp,
			KeyDown,
			KeyPressed = EventType::KeyPressed,
			KeyReleased = EventType::KeyReleased,
			MousePressed = EventType::MouseButtonPressed,
			MouseReleased = EventType::MouseButtonReleased
		};

		inline static KeyState GetKeyState(int keycode) { return s_Instance->GetKeyStateImpl(keycode); }
		inline static bool GetKeyDown(int keycode);
		inline static bool GetKeyUp(int keycode);
		inline static bool GetKeyPressed(int keycode);
		inline static bool GetKeyReleased(int keycode);

		inline static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }
		inline static std::pair<float, float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); }
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }

	protected:
		inline KeyState GetKeyStateImpl(int keycode);
		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;

		void SetKeyState(int key, KeyState state) { KeyStates[key] = state; }
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnKeyReleased(KeyReleasedEvent& e);
		static void UpdateKeyState() { s_Instance->UpdateKeyStateImpl(); }
		void UpdateKeyStateImpl();

	private:
		static Input* s_Instance;

		std::map<int, KeyState> KeyStates;

		std::vector<int> ToUpdate;

	};
}

typedef Engine::Input Input;