#include "pch.h"
#include "Input.h"
#include "InputControler.h"
#include "InputControlerManeger.h"

namespace Engine
{
	InputControler::InputControler(InputControlerManeger* maneger)
	{
		m_Maneger = maneger;
		m_Maneger->AddControler(this);
	}

	InputControler::~InputControler()
	{
		// remove controler from maneger
		m_Maneger->RemoveControler(this);
		
		// unbind events
		for (auto i : events)
		{
			Input::UnbindKey(i->Key, i->Type);
			delete i;
		}
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
		Input::BindKey(&action->Key, &action->Type);
		return action;
	}

	void InputControler::UnbindKey(EventData * event)
	{
		int index = 0;
		for (EventData* e : events)
		{
			if (e == event)
			{
				Input::UnbindKey(e->Key, e->Type);
				events.erase(events.begin() + index);
				return;
			}
			index++;
		}
	}
}