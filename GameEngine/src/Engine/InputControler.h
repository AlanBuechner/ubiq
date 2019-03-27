#pragma once

#include "core.h"

#define BIND_ACTION(x) std::bind(x, this)
#define BIND_AXIS(x, p) std::bind(x, this, p)

namespace Engine
{
	class InputControlerManeger;
}

namespace Engine
{

	struct ENGINE_API EventData
	{
		int type;
		std::function<void()> function;
		int Key;
	};

	class ENGINE_API InputControler
	{
	public:

		InputControler(InputControlerManeger* maneger);

		void RaiseEvent(int key, int state);

		int* BindEvent(int key, int state, std::function<void()> func);

	private:
		std::vector<EventData*> events;

	};
}

typedef Engine::InputControler InputControler;