#pragma once

#include "core.h"

#include "InputControlerLayer.h"
#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

namespace Engine
{

	struct ENGINE_API EventData
	{
		EventType type;
		std::function<void()> function;
		int Key;
	};

	class ENGINE_API InputControler
	{
	public:

		InputControler();

		void RaiseEvent(int key, EventType state);

		int* BindEvent(int key, EventType state, std::function<void()> func);

	private:
		std::vector<EventData*> events;

	};
}