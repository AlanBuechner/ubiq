#include "pch.h"
#include "Input.h"
#include "InputControler.h"
#include "InputControlerManeger.h"

namespace Engine
{
	InputControler::InputControler(Ref<InputControlerManeger> maneger)
	{
		m_Maneger = maneger;
		m_Maneger->AddControler(this);
	}

	InputControler::~InputControler()
	{
		// remove controler from maneger
		m_Maneger->RemoveControler(this);
		
		// unbind events
		for (auto i : m_Events)
		{
			Input::UnbindKey(i->Key, i->Type);
			delete i;
		}
	}

	void InputControler::RaiseEvent(int key, int state)
	{
		for (auto i : m_Events)
		{
			if (i->Type == state)
			{
				switch (i->Key)
				{
				case KEYCODE_SHIFT:
					if (key == KEYCODE_LEFT_SHIFT || key == KEYCODE_RIGHT_SHIFT)
					{
						i->Function();
					}
					break;
				case KEYCODE_CONTROL:
					if (key == KEYCODE_LEFT_CONTROL || key == KEYCODE_RIGHT_CONTROL)
					{
						i->Function();
					}
					break;
				case KEYCODE_ALT:
					if (key == KEYCODE_LEFT_ALT || key == KEYCODE_RIGHT_ALT)
					{
						i->Function();
					}
					break;
				default:
					if(i->Key == key)
						i->Function();
					break;
				}
			}
		}
	}

	void InputControler::RaiseMouseMoveEvent(MouseMoveBindMode bindMode, glm::vec2& pos)
	{
		for (auto i : m_MouseMoveEvents)
		{
			if (i->BindMode == (int)bindMode)
			{
				i->Function(pos);
			}
		}
	}

	EventData* InputControler::BindEvent(int key, int state, std::function<void()> func)
	{
		EventData* action = new EventData;
		action->Type = state;
		action->Function = func;
		action->Key = key;

		m_Events.push_back(action);
		Input::BindKey(&action->Key, &action->Type);
		return action;
	}

	void InputControler::UnbindKey(EventData* event)
	{
		int index = 0;
		for (EventData* e : m_Events)
		{
			if (e == event)
			{
				Input::UnbindKey(e->Key, e->Type);
				m_Events.erase(m_Events.begin() + index);
				return;
			}
			index++;
		}
	}

	MouseMoveEventData* InputControler::BindMouseMoveEvent(MouseMoveBindMode bindMode, std::function<void(glm::vec2&)> func)
	{
		MouseMoveEventData* action = new MouseMoveEventData;
		action->Function = func;
		action->BindMode = (int)bindMode;
		m_MouseMoveEvents.push_back(action);
		Input::SendMouseMovedEvents(true);
		return action;
	}

	void InputControler::UnbindMouseEvent(MouseMoveEventData* event)
	{
		int index = 0;
		for (auto i : m_MouseMoveEvents)
		{
			if (i == event)
			{
				m_MouseMoveEvents.erase(m_MouseMoveEvents.begin() + index);
				Input::SendMouseMovedEvents(false);
			}
			index++;
		}
	}

}