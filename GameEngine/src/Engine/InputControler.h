#pragma once

#include "core.h"

#include "InputControlerLayer.h"
#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

namespace Engine
{

	struct ENGINE_API ActionEventData
	{
		EventType type;
		std::function<void()> function;
		int Key;
	};

	struct ENGINE_API AxisEventData
	{
		EventType type;
		std::function<void(float)> function;
		int Key;
		float param;
	};

	class ENGINE_API InputControler
	{
	public:

		InputControler();

		void ActionEvent(int key, EventType state);
		void AxisEvent(int key, EventType state);

		int* BindActionEvent(int key, EventType state, std::function<void()> func);
		int* BindAxisEvent(int key, EventType state, std::function<void(float)> func, float param);

	private:
		std::vector<ActionEventData*> actionEvents;
		std::vector<AxisEventData*> axisEvents;

	};
}