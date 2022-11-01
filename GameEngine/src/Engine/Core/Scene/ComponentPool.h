#pragma once
#include "Engine/Core/Core.h"
#include <memory>
#include <typeinfo>

namespace Engine
{
	using ComponentType = uint64;

	template<uint64 TSize>
	class SizeComponentPool;


	class ComponentPool
	{
	public:
		using Func = std::function<void (void*)>;

		ComponentPool(ComponentType typeID, uint64 componentSize) :
			m_TypeID(typeID), m_ComponentSize(componentSize)
		{}

		ComponentType GetTypeID() { return m_TypeID; }
		uint64 GetComponentSize() { return m_ComponentSize; }
		virtual std::vector<uint32>& GetUsedSlots() = 0;

		virtual void Each(Func func) = 0;

		// returns the location in the pool
		virtual uint32 Allocate(uint64 entity) = 0;
		virtual void Free(uint64 entity) = 0;
		virtual void* GetComponentMemory(uint32 index) = 0;
		virtual void* GetEntityComponentMemory(uint64 entity) = 0;

	protected:
		const ComponentType m_TypeID;
		const uint64 m_ComponentSize;
	};

	template<uint64 TSize>
	class SizeComponentPool : public ComponentPool
	{
	public:
		struct ComponentData
		{byte data[TSize];};
	public:
		SizeComponentPool(ComponentType typeID) :
			ComponentPool(typeID, TSize)
		{}

		virtual std::vector<uint32>& GetUsedSlots() override
		{
			return m_UsedSlots;
		}

		virtual void Each(Func func) override
		{
			for (auto& comp : m_UsedSlots)
			{
				func(&m_Components[comp]);
			}
		}

		virtual uint32 Allocate(uint64 entity) override
		{
			// allocate more room for the entity's list
			if (m_Entitys.size() <= entity)
				m_Entitys.resize(entity+1);

			if (m_FreeSlots.empty())
			{
				m_Components.push_back({});
				m_UsedSlots.push_back((uint32)m_Components.size() - 1);
				m_Entitys[entity] = (uint32)m_Components.size() - 1;
				return (uint32)m_Components.size() - 1;
			}

			uint32 componentIndex = m_FreeSlots.back();
			m_FreeSlots.pop_back();
			m_UsedSlots.push_back(componentIndex);

			m_Entitys[componentIndex] = componentIndex;
			return componentIndex;
		}

		virtual void Free(uint64 entity) override
		{
			uint32 componentIndex = m_Entitys[entity];
			m_FreeSlots.push_back(componentIndex);

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



		virtual void* GetComponentMemory(uint32 index) override
		{
			return &m_Components[index];
		}

		virtual void* GetEntityComponentMemory(uint64 entity) override
		{
			return GetComponentMemory(m_Entitys[entity]);
		}

	private:
		std::vector<uint32> m_Entitys; // index is the entity value is the component location
		std::vector<uint32> m_FreeSlots;
		std::vector<uint32> m_UsedSlots;
		std::vector<ComponentData> m_Components;
	};
}
