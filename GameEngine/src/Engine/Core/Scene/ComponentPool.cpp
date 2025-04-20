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
		if (m_SceneStatic && m_NumComponents >= 1)
			return UINT32_MAX;

		if (m_EntityComponentMapping.Count() <= entity)
			m_EntityComponentMapping.Resize(entity + 1);

		if (m_FreeSlots.Empty())
		{
			// create new page
			m_Pages.Push(ComponentPoolPage(m_ComponentSize));

			// get old and updated count for number of component slots
			uint32 oldCount = m_NumComponents;
			uint32 newCount = m_NumComponents + PoolSize;

			// add all new component slots to free slots list and set them to unused
			for (int64 i = newCount - 1; i >= oldCount; i--)
			{
				m_FreeSlots.Push(i);
				GetPageForIndex(i).SetEntry(GetIndexInPage(i), false);
			}
		}

		uint32 componentIndex = m_FreeSlots.Pop();
		m_UsedSlots.Push(componentIndex);
		GetPageForIndex(componentIndex).SetEntry(GetIndexInPage(componentIndex), true);

		m_EntityComponentMapping[entity] = componentIndex;

		m_NumComponents++;
		return componentIndex;
	}

	void ComponentPool::Free(uint64 entity)
	{
		uint32 componentIndex = m_EntityComponentMapping[entity];
		m_FreeSlots.Push(componentIndex);
		m_NumComponents--;

		m_ReflectionClass.DestroyInstance(GetComponentRaw(componentIndex), false);
		GetPageForIndex(componentIndex).SetEntry(GetIndexInPage(componentIndex), false);

		// remove entity from used list
		for (uint32 i = 0; i < m_UsedSlots.Count(); i++)
		{
			if (m_UsedSlots[i] == componentIndex)
			{
				m_UsedSlots[i] = m_UsedSlots.Back();
				m_UsedSlots.Pop();
			}
		}
	}
}
