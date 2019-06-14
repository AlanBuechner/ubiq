#include "pch.h"
#include "Layer.h"
#include "Events/Event.h"
#include "InputControlerManeger.h"

namespace Engine {

	Layer::Layer(const std::string& debugName)
		: m_DebugName(debugName)
	{
		m_InputManeger = new InputControlerManeger();
	}

	Layer::~Layer()
	{
		delete m_InputManeger;
	}
	inline void Layer::OnEvent(Event & event) { m_InputManeger->Update(event); }
}