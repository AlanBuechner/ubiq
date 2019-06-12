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
		for (auto i : events)
		{
			if (i->Key == key && i->Type == state)
			{
				i->Function();
			}
		}
	}

	EventData* InputControler::BindEvent(int key, int state, std::function<void()> func)
	{
		EventData* action = new EventData;
		action->Type = state;
		action->Function = func;
		action->Key = key;

		events.push_back(action);
		Input::BindKey(&action->Key);
		return action;
	}

	void InputControler::UnbindKey(EventData * event)
	{
		int index = 0;
		for (EventData* e : events)
		{
			if (e == event)
			{
				Input::UbindKey(e->Key);
				events.erase(events.begin() + index);
				return;
			}
			index++;
		}
	}
}