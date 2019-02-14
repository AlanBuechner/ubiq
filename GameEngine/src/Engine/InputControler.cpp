#include "pch.h"
#include "InputControler.h"

namespace Engine
{
	InputControler::InputControler()
	{
		InputControlerLayer::AddControler(this);
	}

	void InputControler::ActionEvent(int key, EventType state)
	{
		for (auto i = actionEvents.begin(); i != actionEvents.end(); i++)
		{
			if ((*i)->Key == key && (*i)->type == state)
			{
				(*i)->function();
			}
		}
	}

	void InputControler::AxisEvent(int key, EventType state)
	{
		for (auto i = axisEvents.begin(); i != axisEvents.end(); i++)
		{
			if ((*i)->Key == key && (*i)->type == state)
			{
				(*i)->function((*i)->param);
			}
		}
	}

	int* InputControler::BindActionEvent(int key, EventType state, std::function<void()> func)
	{
		ActionEventData* action = new ActionEventData;
		action->type = state;
		action->function = func;
		action->Key = key;

		actionEvents.push_back(action);
		return &action->Key;
	}

	int * InputControler::BindAxisEvent(int key, EventType state, std::function<void(float)> func, float param)
	{
		AxisEventData* action = new AxisEventData;
		action->type = state;
		action->function = func;
		action->Key = key;
		action->param = param;

		axisEvents.push_back(action);
		return &action->Key;
	}

}