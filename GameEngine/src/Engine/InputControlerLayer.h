#pragma once

#include "core.h"

#include "Layer.h"
#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Input.h"

namespace Engine
{
	class InputControler;
}

namespace Engine
{
	class InputControlerLayer : public Layer
	{
	public:
		InputControlerLayer();
		~InputControlerLayer();

		void OnAttach();
		void OnDetach();
		void OnUpdate();
		void OnEvent(Event& event);

		static void UpdateKeyState() { s_Instance->UpdateKeyStateImpl(); }

		inline static Input::KeyState GetKeyState(int keycode) { return s_Instance->GetKeyStateImpl(keycode); }
		inline static bool GetKeyDown(int keycode) { return s_Instance->GetKeyDownImpl(keycode); }
		inline static bool GetKeyUp(int keycode) { return s_Instance->GetKeyUpImpl(keycode); }
		inline static bool GetKeyPressed(int keycode) { return s_Instance->GetKeyPressedImpl(keycode); }
		inline static bool GetKeyReleased(int keycode) { return s_Instance->GetKeyReleasedImpl(keycode); }

	private:

		virtual Input::KeyState GetKeyStateImpl(int keycode) { return KeyStates[keycode]; };
		virtual bool GetKeyDownImpl(int keycode) { return (GetKeyStateImpl(keycode) == Input::KeyDown); }
		virtual bool GetKeyUpImpl(int keycode) { return (GetKeyStateImpl(keycode) == Input::KeyUp); }
		virtual bool GetKeyPressedImpl(int keycode) { return (GetKeyStateImpl(keycode) == Input::KeyPressed); }
		virtual bool GetKeyReleasedImpl(int keycode) { return (GetKeyStateImpl(keycode) == Input::KeyReleased); }

		void UpdateKeyStateImpl();

		void SetKeyState(int key, Input::KeyState state) { KeyStates[key] = state; }

		std::map<int, Input::KeyState> KeyStates;

		std::vector<int> ToUpdate;
		static InputControlerLayer* s_Instance;
	};
}