#include "Engine/Core/Core.h"
#include "InputSystem.h"
#include "Engine/Core/Input/Input.h"

namespace Game
{

	InputLayer::InputLayer(const std::string& name) :
		m_Name(name)
	{}





	void InputSystem::OnAttatch()
	{
		std::vector<std::string> layerNames
		{
			"UI",
			"HUD",
			"Game"
		};

		for (uint32 i = 0; i < layerNames.size(); i++)
		{
			m_Layers.push_back(InputLayer(layerNames[i]));
			m_LayerMap[layerNames[i]] = i;
		}
	}

	void InputSystem::OnEvent(Engine::Event* e)
	{
		CORE_INFO("HELLO WORLD!!!!");
	}
}
