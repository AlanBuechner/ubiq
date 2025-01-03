#pragma once
#include "Scene.h"
#include "SceneRegistry.h"

#include "Engine/Core/UUID.h"


namespace Engine
{
	class Component;
	class TransformComponent;
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
		T* GetComponent()
		{
			return m_Scene->m_Registry.GetComponent<T>(m_EntityID);
		}

		template<typename T>
		T* AddComponent()
		{
			T* component = m_Scene->m_Registry.AddComponent<T>(m_EntityID, m_Scene);
			CORE_ASSERT(component, "Entity alredy has component");
			return component;
		}

		template<typename T>
		T* GetOrAddComponent()
		{
			T* comp = GetComponent<T>();
			if(comp == nullptr)
				comp = m_Scene->m_Registry.AddComponent<T>(m_EntityID, m_Scene);
			return comp;
		}

		template<typename T>
		void RemoveComponent()
		{
			T* comp = GetComponent<T>();
			RemoveComponent(comp);
		}

		void RemoveComponent(Component* comp);

		operator bool() const { return m_EntityID != NullEntity && m_Scene != nullptr; }
		operator EntityType() const { return m_EntityID; }
		operator uint32() const { return (uint32)m_EntityID; }

		UUID GetUUID();
		std::string& GetName() const;
		TransformComponent& GetTransform();

		Entity GetParent();
		void AddChild(Entity child);
		void RemoveChild(Entity child);
		const Utils::Vector<Entity>& GetChildren();
		void SetParentToRoot();
		Utils::Vector<Component*> GetComponents();

		Entity FindChiledWithName(const std::string& name);

		void DirtyAABB();
		void DirtyVolume();

		AABB GetLocalAABB();
		PlainVolume GetPlainVolume();

		Scene* GetScene() { return m_Scene; }

		bool operator == (const Entity& other) const { return m_EntityID == other.m_EntityID && m_Scene == other.m_Scene; }
		bool operator != (const Entity& other) const { return !(*this == other); }

		static Entity null;

	private:
		EntityType m_EntityID = { NullEntity };
		Scene* m_Scene = nullptr;
	};
}
