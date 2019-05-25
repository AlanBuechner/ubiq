#pragma once

#include "core.h"

namespace Engine
{
	class Event;
	class InputControlerManeger;
}

namespace Engine 
{

	class ENGINE_API Layer
	{
#define SUPER Engine::Layer

	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event);

		inline const std::string& GetName() const { return m_DebugName; }

	protected:

		InputControlerManeger* InputManeger;

		std::string m_DebugName;
	};
}