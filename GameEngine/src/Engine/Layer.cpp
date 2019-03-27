#include "pch.h"
#include "Layer.h"
#include "Events/Event.h"
#include "InputControlerManeger.h"

namespace Engine {

	Layer::Layer(const std::string& debugName)
		: m_DebugName(debugName)
	{
		InputManeger = new InputControlerManeger();
	}

	Layer::~Layer()
	{
		delete InputManeger;
	}
	inline void Layer::OnEvent(Event & event) { InputManeger->Update(event); }
}