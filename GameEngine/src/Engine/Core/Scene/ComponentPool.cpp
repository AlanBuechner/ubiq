#include "ComponentPool.h"
#include "Engine/Core/Logging/Log.h"

namespace Engine
{

	ComponentPool::~ComponentPool()
	{
		for (auto& comp : m_UsedSlots)
		{
			m_ReflectionClass.DestroyInstance(GetComponentRaw(comp), false);
		}
	}

	void ComponentPool::EachEntity(EachEntityFunc func)
	{
		for (auto& comp : m_UsedSlots)
			func(GetComponentRaw(comp));
	}

	uint32 ComponentPool::Allocate(uint64 entity)
	{
		CORE_INFO("Allocate Component Data {0}", m_ReflectionClass.GetSname());
		// allocate more room for the entity's list
		if (m_SceneStatic && m_EntityComponentMapping.size() >= 1)
			return UINT32_MAX;

		if (m_EntityComponentMapping.size() <= entity)
			m_EntityComponentMapping.resize(entity + 1);

		if (m_FreeSlots.empty())
		{
			CORE_INFO("Resize Component Data Array {0}", m_ReflectionClass.GetSname());
			uint32 oldCount = m_NumComponents;
			byte* oldData = m_Data;

			uint32 newCount = m_NumComponents * 2;
			byte* newData = (byte*)malloc(m_ComponentSize * newCount);

			memcpy(newData, oldData, oldCount * m_ComponentSize);
			m_NumComponents = newCount;
			m_Data = newData;

			for (uint32 i = newCount - 1; i >= oldCount; i--)
			{
				CORE_INFO("Adding New Index {0}", i);
				m_FreeSlots.push_back(i);
			}
		}

		uint32 componentIndex = m_FreeSlots.back();
		m_FreeSlots.pop_back();
		m_UsedSlots.push_back(componentIndex);

		m_EntityComponentMapping[entity] = componentIndex;
		return componentIndex;
	}

	void ComponentPool::Free(uint64 entity)
	{
		uint32 componentIndex = m_EntityComponentMapping[entity];
		m_FreeSlots.push_back(componentIndex);

		m_ReflectionClass.DestroyInstance(GetComponentRaw(componentIndex), false);

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
