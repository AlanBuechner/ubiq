#include "pch.h"
#include "InputControler.h"

namespace Engine
{
	InputControler::InputControler()
	{
		InputControlerLayer::AddControler(this);
	}

	void InputControler::RaiseEvent(int key, EventType state)
	{
		for (auto i = events.begin(); i != events.end(); i++)
		{
			if ((*i)->Key == key && (*i)->type == state)
			{
				(*i)->function();
			}
		}
	}

	int* InputControler::BindEvent(int key, EventType state, std::function<void()> func)
	{
		EventData* action = new EventData;
		action->type = state;
		action->function = func;
		action->Key = key;

		events.push_back(action);
		return &action->Key;
	}
}