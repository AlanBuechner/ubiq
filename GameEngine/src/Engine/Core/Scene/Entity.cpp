#include "pch.h"
#include "Entity.h"

namespace Engine
{

	Entity::Entity(entt::entity handle, Scene* scene) :
		m_EntityID(handle), m_Scene(scene)
	{
	}
}