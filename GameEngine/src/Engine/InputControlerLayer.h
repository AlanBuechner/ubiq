#pragma once

#include "core.h"

#include "Layer.h"
#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

namespace Engine
{
	class InputControler;
}

namespace Engine
{
	class ENGINE_API InputControlerLayer : public Layer
	{
	public:
		InputControlerLayer();
		~InputControlerLayer();

		void OnAttach();
		void OnDetach();
		void OnUpdate();
		void OnEvent(Event& event);

		static void AddControler(InputControler* controler);

	private:

		bool OnKeyPressedEvent(KeyPressedEvent& e);
		bool OnKeyRelesedEvent(KeyReleasedEvent& e);
		bool OnKeyTypedEvent(KeyTypedEvent& e);

		std::vector<InputControler*> controlers;

		std::vector<InputControler*>::iterator begin() { return controlers.begin(); }
		std::vector<InputControler*>::iterator end() { return controlers.end(); }

		static InputControlerLayer* s_Instance;
	};
}