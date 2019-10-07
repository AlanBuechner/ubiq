#pragma once
#include "Engine/Core/core.h"
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
	class MouseMovedEvent;
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
		void RemoveControler(InputControler* controler);

	private:
		template<class T>
		bool OnKeyEvent(T& e);

		template<class T>
		bool OnMouseEvent(T& e);

		bool OnMouseMoveEvent(MouseMovedEvent& e);

		std::vector<InputControler*> m_Controlers;
	};
}