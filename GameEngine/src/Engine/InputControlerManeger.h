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
		template<class T>
		bool OnKeyEvent(T& e);

		template<class T>
		bool OnMouseEvent(T& e);

		std::vector<InputControler*> controlers;

		std::vector<InputControler*>::iterator begin() { return controlers.begin(); }
		std::vector<InputControler*>::iterator end() { return controlers.end(); }
	};
}