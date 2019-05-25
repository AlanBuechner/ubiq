#include "pch.h"
#include "InputControlerManeger.h"
#include "InputControler.h"

#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

namespace Engine
{
	InputControlerManeger::InputControlerManeger()
	{
	}

	InputControlerManeger::~InputControlerManeger()
	{
	}

	void InputControlerManeger::Update(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(&InputControlerManeger::OnKeyPressedEvent));
		dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN(&InputControlerManeger::OnKeyRelesedEvent));
		dispatcher.Dispatch<KeyDownEvent>(BIND_EVENT_FN(&InputControlerManeger::OnKeyDownEvent));
		dispatcher.Dispatch<KeyTypedEvent>(BIND_EVENT_FN(&InputControlerManeger::OnKeyTypedEvent));
		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(&InputControlerManeger::OnMousePressedEvent));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN(&InputControlerManeger::OnMouseReleasedEvent));
	}

	void InputControlerManeger::AddControler(InputControler* controler)
	{
		controlers.push_back(controler);
	}

	bool InputControlerManeger::OnKeyPressedEvent(KeyPressedEvent& e)
	{
		EventType type = e.GetEventType();
		for (auto i = begin(); i != end(); i++)
		{
			(*i)->RaiseEvent(e.GetKeyCode(), (int)type);
		}
		return false;
	}

	bool InputControlerManeger::OnKeyRelesedEvent(KeyReleasedEvent& e)
	{
		EventType type = e.GetEventType();
		for (auto i = begin(); i != end(); i++)
		{
			(*i)->RaiseEvent(e.GetKeyCode(), (int)type);
		}
		return false;
	}

	bool InputControlerManeger::OnKeyDownEvent(KeyDownEvent& e)
	{
		EventType type = e.GetEventType();
		for (auto i = begin(); i != end(); i++)
		{
			(*i)->RaiseEvent(e.GetKeyCode(), (int)type);
		}
		return false;
	}

	bool InputControlerManeger::OnKeyTypedEvent(KeyTypedEvent& e)
	{
		EventType type = e.GetEventType();
		for (auto i = begin(); i != end(); i++)
		{
			(*i)->RaiseEvent(e.GetKeyCode(), (int)type);
		}
		return false;
	}

	bool InputControlerManeger::OnMousePressedEvent(MouseButtonPressedEvent& e)
	{
		EventType type = e.GetEventType();
		for (auto i = begin(); i != end(); i++)
		{
			(*i)->RaiseEvent(e.GetMouseButton(), (int)type);
		}
		return false;
	}

	bool InputControlerManeger::OnMouseReleasedEvent(MouseButtonReleasedEvent & e)
	{
		EventType type = e.GetEventType();
		for (auto i = begin(); i != end(); i++)
		{
			(*i)->RaiseEvent(e.GetMouseButton(), (int)type);
		}
		return false;
	}

}