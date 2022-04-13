#include "pch.h"
#include "Entity.h"
#include "Components.h"

Engine::Entity Engine::Entity::null = Engine::Entity();

namespace Engine
{

	Entity::Entity(entt::entity handle, Scene* scene) :
		m_EntityID(handle), m_Scene(scene)
	{
	}

	UUID Entity::GetUUID() { return GetComponent<EntityDataComponent>().GetID(); }

	std::string Entity::GetName()
	{
		return GetComponent<EntityDataComponent>().Name;
	}

	TransformComponent& Entity::GetTransform()
	{
		return GetComponent<TransformComponent>();
	}

	Entity Entity::GetParent()
	{
		return GetTransform().GetParent();
	}

	const std::vector<Entity>& Entity::GetChildren()
	{
		return GetTransform().GetChildren();
	}

}
