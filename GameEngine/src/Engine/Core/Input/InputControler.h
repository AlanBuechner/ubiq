#pragma once

#include "Engine/Core/core.h"
#include "glm/vec2.hpp"

#define BIND_ACTION(x) std::bind(x, this)
#define BIND_AXIS(x, p) std::bind(x, this, p)
#define BIND_MOUSEMOVE(x) std::bind(x, this, std::placeholders::_1)

#define MOUSE_POSITON Engine::MouseMoveBindMode::CurserPosition
#define MOUSE_DELTA Engine::MouseMoveBindMode::DeltaPosition
#define MOUSE_SCROLL_WHEEL Engine::MouseMoveBindMode::ScrollWheel

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

	struct ENGINE_API MouseMoveEventData
	{
		std::function<void(glm::vec2& pos)> Function;
		int BindMode;
	};

	enum class MouseMoveBindMode
	{
		CurserPosition,
		DeltaPosition,
		ScrollWheel
	};

	class ENGINE_API InputControler
	{
	public:

		InputControler(Ref<InputControlerManeger> maneger);
		~InputControler();

		void RaiseEvent(int key, int state);
		void RaiseMouseMoveEvent(MouseMoveBindMode bindMode, glm::vec2& pos);

		EventData* BindEvent(int key, int state, std::function<void()> func);
		void UnbindKey(EventData* event);

		MouseMoveEventData* BindMouseMoveEvent(MouseMoveBindMode bindMode, std::function<void(glm::vec2&)> func);
		void UnbindMouseEvent(MouseMoveEventData* event);

	private:
		std::vector<EventData*> m_Events;
		std::vector<MouseMoveEventData*> m_MouseMoveEvents;

		Ref<InputControlerManeger> m_Maneger;

	};
}

typedef Engine::InputControler InputControler;