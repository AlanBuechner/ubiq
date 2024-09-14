#include "Engine/Core/Core.h"
#include "InputSystem.h"
#include "Engine/Core/Input/Input.h"

namespace Game
{

	InputLayer::InputLayer(const std::string& name) :
		m_Name(name)
	{}





	void InputLayer::HandleEvents(InputState& newStates)
	{
		// update layer states
		for (uint32 i = 0; i < Engine::KeyCode::LAST; i++)
		{
			if(m_States.m_DownStates[i] == false && newStates.m_DownStates[i] == false && newStates.m_FirstEventStates[i] == true)
				continue;
			m_States.m_DownStates[i] = newStates.m_DownStates[i];
			m_States.m_FirstEventStates[i] = newStates.m_FirstEventStates[i];
		}

		// handle events
		for (const InputBindFunc& func : m_BindedFuncs)
			func(m_States);
	}

	void InputLayer::UnbindFunc(InputBindFunc func)
	{
		/*auto loc = std::find(m_BindedFuncs.begin(), m_BindedFuncs.end(), func);
		if (loc == m_BindedFuncs.end())
			return;

		m_BindedFuncs.erase(loc);*/
	}

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
		// handle typed events
	}

	void InputSystem::HandleEvents()
	{

		InputState state;
		
		for (uint32 i = 0; i < Engine::KeyCode::LAST; i++)
		{
			Engine::KeyState s = Engine::Input::GetKeyState(i);
			switch (s)
			{
			case Engine::Up:
				state.m_DownStates[i] = false;
				state.m_FirstEventStates[i] = false;
				break;
			case Engine::Down:
				state.m_DownStates[i] = true;
				state.m_FirstEventStates[i] = false;
				break;
			case Engine::Pressed:
				state.m_DownStates[i] = true;
				state.m_FirstEventStates[i] = true;
				break;
			case Engine::Released:
				state.m_DownStates[i] = false;
				state.m_FirstEventStates[i] = true;
				break;
			}
		}


		for (InputLayer& layer : m_Layers)
		{
			layer.HandleEvents(state);
			InputState& layerState = layer.GetInputState();

			// update handled keys
			for (uint32 i = 0; i < Engine::KeyCode::LAST; i++)
			{
				// if key was handled change event to release
				if (layerState.m_HandledKeys[i] || layerState.m_HandleAllKeys)
				{
					state.m_DownStates[i] = false;
					state.m_FirstEventStates[i] = true;
				}
			}
		}
		
	}

	InputLayer* InputSystem::GetLayer(const std::string& name)
	{
		auto loc = m_LayerMap.find(name);
		if (loc == m_LayerMap.end())
			return nullptr;
		return &m_Layers[loc->second];
	}

}
