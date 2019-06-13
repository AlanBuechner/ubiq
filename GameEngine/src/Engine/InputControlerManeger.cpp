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
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(&InputControlerManeger::OnKeyEvent<KeyPressedEvent>));
		dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN(&InputControlerManeger::OnKeyEvent<KeyReleasedEvent>));
		dispatcher.Dispatch<KeyDownEvent>(BIND_EVENT_FN(&InputControlerManeger::OnKeyEvent<KeyDownEvent>));
		dispatcher.Dispatch<KeyTypedEvent>(BIND_EVENT_FN(&InputControlerManeger::OnKeyEvent<KeyTypedEvent>));
		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(&InputControlerManeger::OnMouseEvent<MouseButtonPressedEvent>));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN(&InputControlerManeger::OnMouseEvent<MouseButtonReleasedEvent>));
	}

	void InputControlerManeger::AddControler(InputControler* controler)
	{
		controlers.push_back(controler);
	}

	template<class T>
	bool InputControlerManeger::OnKeyEvent(T& e) 
	{
		EventType type = e.GetEventType();
		for (auto i = begin(); i != end(); i++)
		{
			(*i)->RaiseEvent(e.GetKeyCode(), (int)type);
		}
		return false;
	}

	template<class T>
	bool InputControlerManeger::OnMouseEvent(T & e)
	{
		EventType type = e.GetEventType();
		for (auto i = begin(); i != end(); i++)
		{
			(*i)->RaiseEvent(e.GetMouseButton(), (int)type);
		}
		return false;
	}
}