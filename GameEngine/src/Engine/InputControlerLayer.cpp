#include "pch.h"

#include "InputControlerLayer.h"
#include "InputControler.h"

namespace Engine
{
	InputControlerLayer* InputControlerLayer::s_Instance = nullptr;

	InputControlerLayer::InputControlerLayer()
		: Layer("InputControlerLayer")
	{
		if (s_Instance == nullptr)
		{
			s_Instance = this;
		}
	}

	InputControlerLayer::~InputControlerLayer()
	{}

	void InputControlerLayer::OnAttach()
	{

	}

	void InputControlerLayer::OnDetach()
	{

	}

	void InputControlerLayer::OnUpdate()
	{

	}

	void InputControlerLayer::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(&InputControlerLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<KeyTypedEvent>(BIND_EVENT_FN(&InputControlerLayer::OnKeyTypedEvent));
		dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN(&InputControlerLayer::OnKeyRelesedEvent));

		
	}
	void InputControlerLayer::AddControler(InputControler* controler)
	{
		s_Instance->controlers.push_back(controler);
	}
	bool InputControlerLayer::OnKeyPressedEvent(KeyPressedEvent & e)
	{
		EventType type = e.GetEventType();
		for (auto i = begin(); i != end(); i++)
		{
			(*i)->RaiseEvent(e.GetKeyCode(), type);
		}
		return false;
	}
	bool InputControlerLayer::OnKeyRelesedEvent(KeyReleasedEvent & e)
	{
		EventType type = e.GetEventType();
		for (auto i = begin(); i != end(); i++)
		{
			(*i)->RaiseEvent(e.GetKeyCode(), type);
		}
		return false;
	}
	bool InputControlerLayer::OnKeyTypedEvent(KeyTypedEvent & e)
	{
		EventType type = e.GetEventType();
		for (auto i = begin(); i != end(); i++)
		{
			(*i)->RaiseEvent(e.GetKeyCode(), type);
		}
		return false;
	}
}