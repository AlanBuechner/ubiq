#include "pch.h"
#include "InputControler.h"
#include "InputControlerManeger.h"

namespace Engine
{
	InputControler::InputControler(InputControlerManeger* maneger)
	{
		maneger->AddControler(this);
	}

	void InputControler::RaiseEvent(int key, int state)
	{
		for (auto i = events.begin(); i != events.end(); i++)
		{
			if ((*i)->Key == key && (*i)->type == state)
			{
				(*i)->function();
			}
		}
	}

	int* InputControler::BindEvent(int key, int state, std::function<void()> func)
	{
		EventData* action = new EventData;
		action->type = state;
		action->function = func;
		action->Key = key;

		events.push_back(action);
		return &action->Key;
	}
}