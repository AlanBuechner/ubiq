#pragma once
#include "Engine/Core/Scene/SceneSystem.h"

namespace Engine
{
	class Event;
}

namespace Game
{
	class InputLayer
	{
	public:
		InputLayer(const std::string& name);

		const std::string& GetLayerName() { return m_Name; }

	private:
		std::string m_Name;
	};


	CLASS(GROUP = SceneSystem) InputSystem : public Engine::SceneSystem
	{
	public:
		REFLECTED_BODY(Game::InputSystem);

		virtual void OnAttatch() override;

		void OnEvent(Engine::Event* e);

	private:

		std::vector<InputLayer> m_Layers;
		std::unordered_map<std::string, uint32> m_LayerMap;
	};
}
