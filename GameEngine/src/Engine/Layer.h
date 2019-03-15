#pragma once

#include "core.h"
#include "Events/Event.h"
#include "InputControlerManeger.h"

namespace Engine {

	class ENGINE_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) { InputManeger->Update(event); }

		inline const std::string& GetName() const { return m_DebugName; }

	protected:

		InputControlerManeger* InputManeger = new InputControlerManeger();

		std::string m_DebugName;
	};
}