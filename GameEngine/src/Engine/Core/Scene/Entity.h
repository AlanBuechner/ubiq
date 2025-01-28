#pragma once
#include "Scene.h"
#include "SceneRegistry.h"

#include "Engine/Core/UUID.h"
#include "Engine/Core/ObjectDescription/ObjectDescription.h"


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
		bool HasComponent() const
		{
			return m_Scene->m_Registry.HasComponent<T>(m_EntityID);
		}

		template<typename T>
		T* GetComponent() const
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
		bool operator == (const Entity& other) const { return m_EntityID == other.m_EntityID && m_Scene == other.m_Scene; }
		bool operator != (const Entity& other) const { return !(*this == other); }

		UUID GetUUID() const;
		std::string& GetName() const;
		TransformComponent& GetTransform() const;

		Entity GetParent();
		void AddChild(Entity child);
		void RemoveChild(Entity child);
		const Utils::Vector<Entity>& GetChildren() const;
		void SetParentToRoot();
		Utils::Vector<Component*> GetComponents() const;

		Entity FindChiledWithName(const std::string& name);

		void DirtyAABB();
		void DirtyVolume();

		AABB GetLocalAABB();
		PlainVolume GetPlainVolume();

		Scene* GetScene() { return m_Scene; }

		ObjectDescription CreateObjectDescription() const;
		void LoadComponentFromDescription(const ObjectDescription& desc);

		static Entity null;

	private:
		EntityType m_EntityID = { NullEntity };
		Scene* m_Scene = nullptr;
	};

}
