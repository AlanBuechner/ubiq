#pragma once
#include "entt.hpp"
#include "Engine/Core/Time.h"

namespace Engine
{
	class Entity;
}

namespace Engine
{
	class Scene
	{
	public:
		Scene();
		~Scene();

		void OnUpdate();

		Entity CreateEntity(const std::string& name = "");

	private:
		entt::registry m_Registry;

		friend class Entity;
	};
}