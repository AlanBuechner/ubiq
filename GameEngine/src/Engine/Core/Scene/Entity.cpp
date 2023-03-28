#include "pch.h"
#include "Entity.h"
#include "Components.h"

Engine::Entity Engine::Entity::null = Engine::Entity();

namespace Engine
{

	Entity::Entity(EntityType handle, Scene* scene) :
		m_EntityID(handle), m_Scene(scene)
	{
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

	const std::vector<Entity>& Entity::GetChildren()
	{
		return GetTransform().GetChildren();
	}

	void Entity::SetParentToRoot()
	{
		GetTransform().SetParentToRoot();
	}

	std::vector<Component*> Entity::GetComponents()
	{
		EntityData& data = m_Scene->m_Registry.GetEntityData(m_EntityID);
		std::vector<Component*> components(data.m_Components.size());
		for (uint32 i = 0; i < components.size(); i++)
			components[i] = (Component*)(data.m_Components[i].m_Pool->GetComponentMemory(data.m_Components[i].m_ComponentLoc));

		return components;
	}

}
