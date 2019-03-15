#include "pch.h"
#include "InputControlerManeger.h"
#include "InputControler.h"


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
		dispatcher.Dispatch<KeyTypedEvent>(BIND_EVENT_FN(&InputControlerManeger::OnKeyTypedEvent));
		dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN(&InputControlerManeger::OnKeyRelesedEvent));
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
			(*i)->RaiseEvent(e.GetKeyCode(), type);
		}
		return false;
	}

	bool InputControlerManeger::OnKeyRelesedEvent(KeyReleasedEvent& e)
	{
		EventType type = e.GetEventType();
		for (auto i = begin(); i != end(); i++)
		{
			(*i)->RaiseEvent(e.GetKeyCode(), type);
		}
		return false;
	}

	bool InputControlerManeger::OnKeyTypedEvent(KeyTypedEvent& e)
	{
		EventType type = e.GetEventType();
		for (auto i = begin(); i != end(); i++)
		{
			(*i)->RaiseEvent(e.GetKeyCode(), type);
		}
		return false;
	}

}