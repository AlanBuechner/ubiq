#include "pch.h"
#include "Layer.h"
#include "Engine/Events/Event.h"

namespace Engine {

	Layer::Layer(const std::string& debugName)
		: m_DebugName(debugName)
	{}

	Layer::~Layer()
	{}

}
