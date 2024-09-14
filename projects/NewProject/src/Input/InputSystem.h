#pragma once
#include "Engine/Core/Scene/SceneSystem.h"
#include "Engine/Core/Input/KeyCodes.h"



namespace Engine
{
	class Event;
}

namespace Game
{

	// input state
	class InputState
	{
	public:
		InputState()
		{
			std::fill_n(m_DownStates, s_NumKeyStates, false);
			std::fill_n(m_FirstEventStates, s_NumKeyStates, false);
			std::fill_n(m_HandledKeys, s_NumKeyStates, false);
		}

	public:
		bool GetKeyDown(Engine::KeyCode key) { return m_DownStates[key]; }
		bool GetKeyUp(Engine::KeyCode key) { return !m_DownStates[key]; }
		bool GetKeyPressed(Engine::KeyCode key) { return m_DownStates[key] && m_FirstEventStates[key]; }
		bool GetKeyReleased(Engine::KeyCode key) { return !m_DownStates[key] && m_FirstEventStates[key]; }
		bool GetKeyHeld(Engine::KeyCode key) { return m_DownStates[key] && !m_FirstEventStates[key]; }

		void HandleKey(Engine::KeyCode key) { m_HandledKeys[key] = true; }
		void HandleLayer() { m_HandleAllKeys = true; }

	private:
		static constexpr int s_NumKeyStates = Engine::KeyCode::LAST;
		bool m_DownStates[s_NumKeyStates];
		bool m_FirstEventStates[s_NumKeyStates];
		bool m_HandledKeys[s_NumKeyStates];
		bool m_HandleAllKeys = false;

		friend class InputLayer;
		friend class InputSystem;
	};


	// input layer
	class InputLayer
	{
	public:
		typedef std::function<void(InputState&)> InputBindFunc;

	public:
		InputLayer(const std::string& name);

		const std::string& GetLayerName() { return m_Name; }
		InputState& GetInputState() { return m_States; }

		void HandleEvents(InputState& newStates);

		void BindFunc(InputBindFunc func) { m_BindedFuncs.push_back(func); }
		void UnbindFunc(InputBindFunc func);

	private:
		std::string m_Name;
		InputState m_States;
		std::vector<InputBindFunc> m_BindedFuncs;
		
	};



	// input system
	CLASS(GROUP = SceneSystem) InputSystem : public Engine::SceneSystem
	{
	public:
		REFLECTED_BODY(Game::InputSystem);

		virtual void OnAttatch() override;
		void OnEvent(Engine::Event* e);

		FUNCTION()
		void HandleEvents();

		InputLayer* GetLayer(uint32 i) { return &m_Layers[i]; }
		InputLayer* GetLayer(const std::string& name);

	private:

		std::vector<InputLayer> m_Layers;
		std::unordered_map<std::string, uint32> m_LayerMap;
	};
}
