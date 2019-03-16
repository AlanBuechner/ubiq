#include "pch.h"

#include "InputControlerLayer.h"
#include "InputControler.h"
#include "Input.h"

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
		EventType type = e.GetEventType();
		if (type == EventType::KeyPressed || type == EventType::KeyReleased)
		{
			auto event = (KeyPressedEvent&)e;
			Input::SetKeyState(event.GetKeyCode(), (Input::KeyState)type);
			ToUpdate.push_back(event.GetKeyCode());
		}
	}
	void InputControlerLayer::UpdateKeyStateImpl()
	{
		for (auto i = ToUpdate.begin(); i != ToUpdate.end(); i++)
		{
			int state = Input::GetKeyState(*i);
			if (state == Input::KeyPressed)
			{
				Input::SetKeyState(*i, Input::KeyDown);
			}
			else if (state == Input::KeyReleased)
			{
				Input::SetKeyState(*i, Input::KeyUp);
			}
		}
		ToUpdate.clear();
	}
}