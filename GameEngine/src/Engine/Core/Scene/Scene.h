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
		void OnViewportResize(uint32_t width, uint32_t height);

		Entity CreateEntity(const std::string& name = "");
		void DestroyEntity(Entity entity);

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

	private:
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;

		entt::registry m_Registry;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}