#pragma once
#include "Scene.h"
#include "SceneRegistry.h"

#include "Engine/Core/UUID.h"

namespace Engine
{
	struct Component;
	struct TransformComponent;
}

namespace Engine
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(EntityType handle, Scene* scene);
		Entity(const Entity& other) = default;

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.HasComponent<T>(m_EntityID);
		}

		template<typename T>
		T& GetComponent()
		{
			CORE_ASSERT(HasComponent<T>(), "Entity does not have component");
			return m_Scene->m_Registry.GetComponent<T>(m_EntityID);
		}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			CORE_ASSERT(!HasComponent<T>(), "Entity alredy has component");
			T& component = m_Scene->m_Registry.AddComponent<T>(m_EntityID, std::forward<Args>(args)...);
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
			m_Scene->m_Registry.RemoveComponent<T>(m_EntityID);
		}

		operator bool() const { return m_EntityID != NullEntity && m_Scene != nullptr; }
		operator EntityType() const { return m_EntityID; }
		operator uint32() const { return (uint32)m_EntityID; }

		UUID GetUUID();
		std::string& GetName();
		TransformComponent& GetTransform();

		Entity GetParent();
		void AddChild(Entity child);
		void RemoveChild(Entity child);
		const std::vector<Entity>& GetChildren();
		void SetParentToRoot();
		std::vector<Component*> GetComponents();

		Scene* GetScene() { return m_Scene; }

		bool operator == (const Entity& other) const { return m_EntityID == other.m_EntityID && m_Scene == other.m_Scene; }
		bool operator != (const Entity& other) const { return !(*this == other); }

		static Entity null;

	private:
		EntityType m_EntityID = { NullEntity };
		Scene* m_Scene = nullptr;
	};
}
