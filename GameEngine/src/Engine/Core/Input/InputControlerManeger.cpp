#include "pch.h"
#include "InputControlerManeger.h"
#include "InputControler.h"

#include "Engine/Events/Event.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"

namespace Engine
{
	InputControlerManeger::InputControlerManeger()
	{
	}

	InputControlerManeger::~InputControlerManeger()
	{
	}

	void InputControlerManeger::Update(Event* e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(&InputControlerManeger::OnKeyEvent<KeyPressedEvent>));
		dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN(&InputControlerManeger::OnKeyEvent<KeyReleasedEvent>));
		dispatcher.Dispatch<KeyDownEvent>(BIND_EVENT_FN(&InputControlerManeger::OnKeyEvent<KeyDownEvent>));
		dispatcher.Dispatch<KeyTypedEvent>(BIND_EVENT_FN(&InputControlerManeger::OnKeyEvent<KeyTypedEvent>));
		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(&InputControlerManeger::OnMouseEvent<MouseButtonPressedEvent>));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN(&InputControlerManeger::OnMouseEvent<MouseButtonReleasedEvent>));
		dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(&InputControlerManeger::OnMouseMoveEvent));
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(&InputControlerManeger::OnMouseScrollEvent));
	}

	void InputControlerManeger::AddControler(InputControler* controler)
	{
		m_Controlers.Push(controler);
	}

	void InputControlerManeger::RemoveControler(InputControler* controler)
	{
		int index = 0;
		for (auto i : m_Controlers)
		{
			if (i == controler)
			{
				m_Controlers.Remove(index);
			}
			++index;
		}
	}

	bool InputControlerManeger::OnMouseMoveEvent(MouseMovedEvent* e)
	{
		if (m_Controlers.Empty())
			return false;
		for (auto i : m_Controlers)
		{
			i->RaiseMouseMoveEvent(e->GetMouseBindMode(), Math::Vector2(e->GetX(), e->GetY()));
		}
		return false;
	}

	bool InputControlerManeger::OnMouseScrollEvent(MouseScrolledEvent* e)
	{
		if (m_Controlers.Empty())
			return false;
		for (auto i : m_Controlers)
		{
			i->RaiseMouseMoveEvent(e->GetMouseBindMode(), Math::Vector2({e->GetXOffset(), e->GetYOffset()}));
		}
		return false;
	}

	template<class T>
	bool InputControlerManeger::OnKeyEvent(T* e) 
	{
		if (m_Controlers.Empty())
			return false;
		EventType type = e->GetEventType();
		for (auto i : m_Controlers)
		{
			i->RaiseEvent(e->GetKeyCode(), (int)type);
		}
		return false;
	}

	template<class T>
	bool InputControlerManeger::OnMouseEvent(T* e)
	{
		if (m_Controlers.Empty())
			return false;
		EventType type = e->GetEventType();
		for (auto i : m_Controlers)
		{
			i->RaiseEvent(e->GetMouseButton(), (int)type);
		}
		return false;
	}
}
