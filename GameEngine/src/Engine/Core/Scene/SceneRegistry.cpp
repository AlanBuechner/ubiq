#include "pch.h"
#include "SceneRegistry.h"

namespace Engine
{


	void EntityData::RemoveComponentReferance(ComponentPool* pool)
	{
		std::vector<ComponentRef>::const_iterator i = std::find_if(m_Components.begin(), m_Components.end(), [pool](ComponentRef comp) {return comp.m_Pool == pool; });
		m_Components.erase(i);
	}

	SceneRegistry::~SceneRegistry()
	{
		for (auto& pool : m_Pools)
			delete pool.second;
	}

	EntityType SceneRegistry::CreateEntity()
	{
		return CreateEntity(UUID(), "");
	}

	EntityType SceneRegistry::CreateEntity(UUID id, const std::string& name)
	{
		if (m_FreeEntitys.empty())
		{
			// no locations in the entity list is free
			m_Entitys.push_back({ id, name });
			m_UsedEntitys.push_back(m_Entitys.size()-1);
			return m_Entitys.size() - 1;
		}

		EntityType entity = m_FreeEntitys.back(); // get the most recently deleted entity
		m_FreeEntitys.pop_back();
		m_Entitys[entity] = { id, name };
		m_UsedEntitys.push_back(entity);
		return entity;
	}

	void SceneRegistry::DestroyEntity(EntityType entity)
	{
		// remove components from entity
		for (auto& compRef : m_Entitys[entity].m_Components)
			compRef.m_Pool->Free(entity);
		m_Entitys[entity].m_Components.clear();

		// remove entity from used entity list
		for (uint32 i = 0; i < m_UsedEntitys.size(); i++)
		{
			if (m_UsedEntitys[i] == entity)
			{
				// swap and pop
				m_UsedEntitys[i] = m_UsedEntitys.back();
				m_UsedEntitys.pop_back();
			}
		}

		// add entity to free entity list
		m_FreeEntitys.push_back(entity);
	}

	void SceneRegistry::Each(Func func)
	{
		for (EntityType entity : m_UsedEntitys)
			func(entity);
	}

}

