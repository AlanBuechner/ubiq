#include "ComponentPool.h"
#include "Engine/Core/Logging/Log.h"

namespace Engine
{


	ComponentPoolPage::ComponentPoolPage(uint32 componentSize) :
		m_ComponentSize(componentSize)
	{
		m_Data = (byte*)malloc(m_ComponentSize*PoolSize);
	}



	ComponentPool::~ComponentPool()
	{

		for (ComponentPoolPage& page : m_Pages)
		{
			for (uint32 i = 0; i < PoolSize; i++)
			{
				if (page.HasEntry(i))
					m_ReflectionClass.DestroyInstance(page.GetComponentMemory(i), false);
			}
			free(page.m_Data);
		}
	}

	void ComponentPool::EachEntity(EachEntityFunc func)
	{

		for (ComponentPoolPage& page : m_Pages)
		{
			for (uint32 i = 0; i < PoolSize; i++)
			{
				if (page.HasEntry(i))
					func(page.GetComponentMemory(i));
			}
		}
	}

	uint32 ComponentPool::Allocate(uint64 entity)
	{
		// allocate more room for the entity's list
		if (m_SceneStatic && m_EntityComponentMapping.size() >= 1)
			return UINT32_MAX;

		if (m_EntityComponentMapping.size() <= entity)
			m_EntityComponentMapping.resize(entity + 1);

		if (m_FreeSlots.empty())
		{
			// resize
			m_Pages.push_back(ComponentPoolPage(m_ComponentSize));

			uint32 oldCount = m_NumComponents;
			uint32 newCount = m_NumComponents + PoolSize;

			for (int64 i = newCount - 1; i >= oldCount; i--)
			{
				m_FreeSlots.push_back(i);
				GetPageForIndex(i).SetEntry(GetIndexInPage(i), false);
			}
		}

		uint32 componentIndex = m_FreeSlots.back();
		m_FreeSlots.pop_back();
		m_UsedSlots.push_back(componentIndex);
		GetPageForIndex(componentIndex).SetEntry(GetIndexInPage(componentIndex), true);

		m_EntityComponentMapping[entity] = componentIndex;
		return componentIndex;
	}

	void ComponentPool::Free(uint64 entity)
	{
		uint32 componentIndex = m_EntityComponentMapping[entity];
		m_FreeSlots.push_back(componentIndex);

		m_ReflectionClass.DestroyInstance(GetComponentRaw(componentIndex), false);
		GetPageForIndex(componentIndex).SetEntry(GetIndexInPage(componentIndex), false);

		// remove entity from used list
		for (uint32 i = 0; i < m_UsedSlots.size(); i++)
		{
			if (m_UsedSlots[i] == componentIndex)
			{
				m_UsedSlots[i] = m_UsedSlots.back();
				m_UsedSlots.pop_back();
			}
		}
	}
}
