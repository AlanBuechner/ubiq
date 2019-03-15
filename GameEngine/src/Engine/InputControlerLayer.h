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

	private:
		static InputControlerLayer* s_Instance;
	};
}