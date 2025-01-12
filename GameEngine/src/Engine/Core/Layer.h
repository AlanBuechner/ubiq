#pragma once

#include "Core.h"

namespace Engine
{
	class Event;
}

namespace Engine 
{

	class  Layer
	{
	protected:
		typedef Engine::Layer Super;

	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnRender() {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event* event) {}

		inline const std::string& GetName() const { return m_DebugName; }

		inline bool IsLayerStackOwner() { return m_IsLayerStackOwner; }

	protected:

		std::string m_DebugName;
		bool m_IsLayerStackOwner = true;
	};
}
