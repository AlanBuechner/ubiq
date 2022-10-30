#include "pch.h"
#include "SceneRegistry.h"

namespace Engine
{


	EntityData::~EntityData()
	{
		for (auto& component : m_Components)
			delete component.m_ComponentLoc;
	}


	EntityType SceneRegistry::CreateEntity()
	{
		return CreateEntity(UUID());
	}

	EntityType SceneRegistry::CreateEntity(UUID id)
	{
		if (m_FreeEntitys.empty())
		{
			// no locations in the entity list is free
			m_Entitys.push_back({ id, "" });
			m_UsedEntitys.push_back(m_Entitys.size()-1);
			return m_Entitys.size() - 1;
		}

		EntityType entity = m_FreeEntitys.back(); // get the most recently deleted entity
		m_FreeEntitys.pop_back();
		m_Entitys[entity] = { id, "" };
		m_UsedEntitys.push_back(entity);
		return entity;
	}

	void SceneRegistry::DestroyEntity(EntityType entity)
	{
		for (uint32 i = 0; i < m_UsedEntitys.size(); i++)
		{
			if (m_UsedEntitys[i] == entity)
			{
				// swap and pop
				m_UsedEntitys[i] = m_UsedEntitys.back();
				m_UsedEntitys.pop_back();
			}
		}
		m_FreeEntitys.push_back(entity);
	}

	void SceneRegistry::Each(Func func)
	{
		for (EntityType entity : m_UsedEntitys)
			func(entity);
	}

	void* SceneRegistry::AllocateComponent(ComponentType type, uint64 componentSize, EntityType entity)
	{
		for (uint32 i = 0; i < m_Pools.size(); i++)
		{
			if (m_Pools[i].GetTypeID() == type)
			{
				// allocate memory for component
				return m_Pools[i].Allocate(entity);
			}
		}

		// could not find the component pool
		m_Pools.push_back(ComponentPool(type, componentSize)); // create new pool

		// allocate memory for pool
		return m_Pools.back().Allocate(entity);
	}

}

