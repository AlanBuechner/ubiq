#pragma once
#include "Scene.h"
#include "entt.hpp"

#include "Engine/Core/UUID.h"

namespace Engine
{
	struct TransformComponent;
}

namespace Engine
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.has<T>(m_EntityID);
		}

		template<typename T>
		T& GetComponent()
		{
			CORE_ASSERT(HasComponent<T>(), "Entity does not have component");
			return m_Scene->m_Registry.get<T>(m_EntityID);
		}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			CORE_ASSERT(!HasComponent<T>(), "Entity alredy has component");
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityID, std::forward<Args>(args)...);
			component.Owner = *this;
			m_Scene->OnComponentAdded<T>(*this, component);
			component.OnComponentAdded();
			return component;
		}

		template<typename T>
		void RemoveComponent()
		{
			CORE_ASSERT(HasComponent<T>(), "Entity does not have component");
			GetComponent<T>().OnComponentRemoved();
			m_Scene->m_Registry.remove<T>(m_EntityID);
		}

		operator bool() const { return m_EntityID != entt::null && m_Scene != nullptr; }
		operator entt::entity() const { return m_EntityID; }
		operator uint32() const { return (uint32)m_EntityID; }

		UUID GetUUID();
		std::string GetName();
		TransformComponent& GetTransform();

		Entity GetParent();
		void AddChild(Entity child);
		void RemoveChild(Entity child);
		const std::vector<Entity>& GetChildren();
		void SetParentToRoot();

		Scene* GetScene() { return m_Scene; }

		bool operator == (const Entity& other) const { return m_EntityID == other.m_EntityID && m_Scene == other.m_Scene; }
		bool operator != (const Entity& other) const { return !(*this == other); }

		static Entity null;

	private:
		entt::entity m_EntityID = { entt::null };
		Scene* m_Scene = nullptr;
	};
}
