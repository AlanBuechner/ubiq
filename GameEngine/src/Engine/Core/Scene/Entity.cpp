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

	UUID Entity::GetUUID() { return GetComponent<IDComponent>().ID; }

	std::string Entity::GetTag()
	{
		return GetComponent<TagComponent>().Tag;
	}

	TransformComponent& Entity::GetTransform()
	{
		return GetComponent<TransformComponent>();
	}

}
