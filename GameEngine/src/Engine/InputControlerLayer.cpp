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

	}
}