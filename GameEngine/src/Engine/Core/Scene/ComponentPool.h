#pragma once
#include "Engine/Core/Core.h"
#include "Utils/Vector.h"
#include <memory>
#include <typeinfo>
#include <bitset>

namespace Engine
{
	using ComponentType = uint64;


	constexpr uint32 PoolSize = 32;
	class ComponentPoolPage
	{
	public:
		ComponentPoolPage(uint32 componentSize);

		bool HasEntry(uint32 index) { return m_UsedSlots.test(index); }
		void SetEntry(uint32 index, bool val) { m_UsedSlots.set(index, val); }
		void* GetComponentMemory(uint32 index) { return (void*)(m_Data + m_ComponentSize * index); }

	private:
		uint32 m_ComponentSize;
		byte* m_Data;
		std::bitset<PoolSize> m_UsedSlots;

		friend class ComponentPool;
	};


	class ComponentPool
	{
	public:
		using EachEntityFunc = std::function<void (void*)>;

		ComponentPool(const Reflect::Class& reflectedClass) :
			m_ReflectionClass(reflectedClass), 
			m_TypeID(reflectedClass.GetTypeID()),
			m_ComponentSize(reflectedClass.GetSize()),
			m_SceneStatic(reflectedClass.HasFlag("SceneStatic"))
		{}
		~ComponentPool();

		ComponentType GetTypeID() { return m_TypeID; }
		uint64 GetComponentSize() { return m_ComponentSize; }
		Utils::Vector<uint32>& GetUsedSlots() { return m_UsedSlots; }

		void EachEntity(EachEntityFunc func);

		// returns the location in the pool
		uint32 Allocate(uint64 entity);
		void Free(uint64 entity);
		uint32 GetIndexInPage(uint32 i) { return i % PoolSize; }
		ComponentPoolPage& GetPageForIndex(uint32 i) { return m_Pages[i/PoolSize]; }
		void* GetComponentRaw(uint32 i) { return GetPageForIndex(i).GetComponentMemory(GetIndexInPage(i)); }
		template<typename T> T* GetComponent(uint32 i) { return (T*)GetComponentRaw(i); }
		void* GetComponentForEntityRaw(uint32 entityID) { return GetComponentRaw(m_EntityComponentMapping[entityID]); }
		template<typename T> T* GetComponentForEntity(uint32 entityID) { return GetComponent<T>(m_EntityComponentMapping[entityID]); }

	protected:
		const Reflect::Class& m_ReflectionClass;
		const ComponentType m_TypeID;
		const uint64 m_ComponentSize;
		const bool m_SceneStatic;

		Utils::Vector<uint32> m_EntityComponentMapping; // index is the entity value is the component location
		Utils::Vector<uint32> m_FreeSlots;
		Utils::Vector<uint32> m_UsedSlots;

		uint32 m_NumComponents = 0;
		std::vector<ComponentPoolPage> m_Pages;
	};
}
