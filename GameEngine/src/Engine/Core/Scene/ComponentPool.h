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
		using EachEntityFunc = std::function<void (void*)>;

		ComponentPool(const Reflect::Class& reflectedClass) :
			m_ReflectionClass(reflectedClass), 
			m_TypeID(reflectedClass.GetTypeID()),
			m_ComponentSize(reflectedClass.GetSize()),
			m_SceneStatic(reflectedClass.HasFlag("SceneStatic")),
			m_NumComponents(1),
			m_Data((byte*)malloc(reflectedClass.GetSize())),
			m_FreeSlots({ 0 })
		{}
		~ComponentPool();

		ComponentType GetTypeID() { return m_TypeID; }
		uint64 GetComponentSize() { return m_ComponentSize; }
		std::vector<uint32>& GetUsedSlots() { return m_UsedSlots; }

		void EachEntity(EachEntityFunc func);

		// returns the location in the pool
		uint32 Allocate(uint64 entity);
		void Free(uint64 entity);
		uint32 GetComponentOffset(uint32 i) { return  m_ComponentSize * i; }
		void* GetComponentRaw(uint32 i) { return (void*)(m_Data + GetComponentOffset(i)); }
		template<typename T> T* GetComponent(uint32 i) { return (T*)GetComponentRaw(i); }
		void* GetComponentForEntityRaw(uint32 entityID) { return GetComponentRaw(m_EntityComponentMapping[entityID]); }
		template<typename T> T* GetComponentForEntity(uint32 entityID) { return GetComponent<T>(m_EntityComponentMapping[entityID]); }

	protected:
		const Reflect::Class& m_ReflectionClass;
		const ComponentType m_TypeID;
		const uint64 m_ComponentSize;
		const bool m_SceneStatic;

		std::vector<uint32> m_EntityComponentMapping; // index is the entity value is the component location
		std::vector<uint32> m_FreeSlots;
		std::vector<uint32> m_UsedSlots;

		uint32 m_NumComponents;
		byte* m_Data;
	};
}
