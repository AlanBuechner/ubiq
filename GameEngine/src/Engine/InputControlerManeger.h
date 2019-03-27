#pragma once
#include "core.h"
#include <vector>

#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

namespace Engine
{
	class InputControler;
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
		bool OnKeyTypedEvent(KeyTypedEvent& e);
		bool OnMousePressedEvent(MouseButtonPressedEvent& e);
		bool OnMouseReleasedEvent(MouseButtonReleasedEvent& e);

		std::vector<InputControler*> controlers;

		std::vector<InputControler*>::iterator begin() { return controlers.begin(); }
		std::vector<InputControler*>::iterator end() { return controlers.end(); }
	};
}