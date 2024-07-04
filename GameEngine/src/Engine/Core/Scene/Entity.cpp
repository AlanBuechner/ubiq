#include "pch.h"
#include "Entity.h"
#include "Components.h"
#include "TransformComponent.h"

Engine::Entity Engine::Entity::null = Engine::Entity();

namespace Engine
{

	Entity::Entity(EntityType handle, Scene* scene) :
		m_EntityID(handle), m_Scene(scene)
	{
	}

	void Entity::RemoveComponent(Component* comp)
	{
		if (comp != nullptr)
		{
			comp->OnComponentRemoved();
			m_Scene->m_Registry.RemoveComponent(m_EntityID, comp->GetClass());
		}
	}

	UUID Entity::GetUUID() { return m_Scene->m_Registry.GetEntityData(m_EntityID).ID; }

	std::string& Entity::GetName()
	{
		return m_Scene->m_Registry.GetEntityData(m_EntityID).name;
	}

	TransformComponent& Entity::GetTransform()
	{
		TransformComponent* comp = GetComponent<TransformComponent>();
		CORE_ASSERT(comp != nullptr, "entety does not have transform");
		return *comp;
	}

	Entity Entity::GetParent()
	{
		return GetTransform().GetParent();
	}

	void Entity::AddChild(Entity child)
	{
		GetTransform().AddChild(child);
	}

	void Entity::RemoveChild(Entity child)
	{
		GetTransform().RemoveChild(child);
	}

	const Utils::Vector<Entity>& Entity::GetChildren()
	{
		return GetTransform().GetChildren();
	}

	void Entity::SetParentToRoot()
	{
		GetTransform().SetParentToRoot();
	}

	Utils::Vector<Component*> Entity::GetComponents()
	{
		EntityData& data = m_Scene->m_Registry.GetEntityData(m_EntityID);
		Utils::Vector<Component*> components(data.m_Components.size(), {});
		for (uint32 i = 0; i < components.Count(); i++)
			components[i] = (Component*)(data.m_Components[i].m_Pool->GetComponentRaw(data.m_Components[i].m_ComponentLoc));

		return components;
	}

	void Entity::DirtyAABB()
	{
		EntityData& data = m_Scene->m_Registry.GetEntityData(m_EntityID);
		data.dirtyAABB = true;
		data.dirtyVolume = true;
	}

	void Entity::DirtyVolume()
	{
		EntityData& data = m_Scene->m_Registry.GetEntityData(m_EntityID);
		data.dirtyVolume = true;
	}

	AABB Entity::GetLocalAABB()
	{
		EntityData& data = m_Scene->m_Registry.GetEntityData(m_EntityID);
		if (data.dirtyAABB)
		{
			Utils::Vector<AABB> aabbs;
			for (Component* component : GetComponents())
			{
				Utils::Vector<AABB> componentVolumes = component->GetVolumes();
				for (AABB aabb : componentVolumes)
					aabbs.Push(aabb);
			}

			data.aabb = AABB::GetExtents(aabbs);
			if(data.aabb.Valid())
				data.aabb.MinThickness(0.001f);
			data.dirtyAABB = false;
		}

		return data.aabb;
	}

	PlainVolume Entity::GetPlainVolume()
	{
		EntityData& data = m_Scene->m_Registry.GetEntityData(m_EntityID);
		if (data.dirtyVolume)
		{
			AABB aabb = GetLocalAABB();
			data.volume = aabb.GetPlainVolume();
			data.volume.Transform(GetTransform().GetGlobalTransform());
			data.dirtyVolume = false;
		}

		return data.volume;
	}

}
