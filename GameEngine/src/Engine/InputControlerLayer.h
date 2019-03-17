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

	private:

		void UpdateKeyStateImpl();

		std::vector<int> ToUpdate;
		static InputControlerLayer* s_Instance;
	};
}