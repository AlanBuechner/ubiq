#pragma once

#include "core.h"

#include "InputControlerLayer.h"
#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

#include "InputControlerManeger.h"

#define BIND_ACTION(x) std::bind(x, this)
#define BIND_AXIS(x, p) std::bind(x, this, p)

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

		InputControler(InputControlerManeger* maneger);

		void RaiseEvent(int key, EventType state);

		int* BindEvent(int key, EventType state, std::function<void()> func);

	private:
		std::vector<EventData*> events;

	};
}