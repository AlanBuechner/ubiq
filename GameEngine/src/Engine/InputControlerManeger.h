#pragma once
#include "core.h"
#include <vector>

namespace Engine
{
	class InputControler;
	class Event;
	class KeyPressedEvent;
	class KeyReleasedEvent;
	class KeyDownEvent;
	class KeyTypedEvent;
	class MouseButtonPressedEvent;
	class MouseButtonReleasedEvent;
}

namespace Engine
{
	class ENGINE_API InputControlerManeger
	{
	public:
		InputControlerManeger();
		~InputControlerManeger();

		void Update(Event& event);
		void AddControler(InputControler* controler);
	private:
		bool OnKeyPressedEvent(KeyPressedEvent& e);
		bool OnKeyRelesedEvent(KeyReleasedEvent& e);
		bool OnKeyDownEvent(KeyDownEvent& e);
		bool OnKeyTypedEvent(KeyTypedEvent& e);
		bool OnMousePressedEvent(MouseButtonPressedEvent& e);
		bool OnMouseReleasedEvent(MouseButtonReleasedEvent& e);

		std::vector<InputControler*> controlers;

		std::vector<InputControler*>::iterator begin() { return controlers.begin(); }
		std::vector<InputControler*>::iterator end() { return controlers.end(); }
	};
}