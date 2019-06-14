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
		int Type;
		std::function<void()> Function;
		int Key;
	};

	class ENGINE_API InputControler
	{
	public:

		InputControler(InputControlerManeger* maneger);
		~InputControler();

		void RaiseEvent(int key, int state);

		EventData* BindEvent(int key, int state, std::function<void()> func);
		void UnbindKey(EventData* event);

	private:
		std::vector<EventData*> events;

		InputControlerManeger* m_Maneger;

	};
}

typedef Engine::InputControler InputControler;