#include "pch.h"
#include "Layer.h"
#include "Engine/Events/Event.h"
#include "Engine/Core/Input/InputControlerManeger.h"

namespace Engine {

	Layer::Layer(const std::string& debugName)
		: m_DebugName(debugName)
	{
		m_InputManeger = CreateRef<InputControlerManeger>();
	}

	Layer::~Layer()
	{
		m_InputManeger.~shared_ptr();
	}

	inline void Layer::OnEvent(Event* event) { m_InputManeger->Update(event); }
}
