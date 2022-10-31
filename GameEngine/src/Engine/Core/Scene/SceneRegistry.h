#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/UUID.h"
#include "ComponentPool.h"
#include <vector>
#include <unordered_map>

namespace Engine
{

	struct EntityData
	{
		UUID ID;
		std::string name;

		struct ComponentRef {
			ComponentPool* m_Pool;
			uint32 m_ComponentLoc;
		};
		std::vector<ComponentRef> m_Components;

		~EntityData();

		void RemoveComponentReferance(ComponentPool* pool);

	};
	using EntityType = uint64;

	class SceneRegistry
	{
	public:
		using Func = std::function<void (EntityType)>;

		~SceneRegistry();

		EntityType CreateEntity();
		EntityType CreateEntity(UUID id);
		void DestroyEntity(EntityType entity);

		// EntityData Does not have referential integrity
		EntityData& GetEntityData(EntityType entity) { return m_Entitys[entity]; }

		template<class T>
		T& AddComponent(EntityType entity)
		{
			uint64 componentID = typeid(T).hash_code(); // get component type id

			// find the component pool
			ComponentPool* pool = m_Pools[componentID];
			if (pool == nullptr) pool = m_Pools[componentID] = new SizeComponentPool<sizeof(T)>(componentID);

			// allocate memory for component
			uint32 componentIndex = pool->Allocate(entity);
			void* componentLocation = pool->GetComponentMemory(componentIndex);
			CORE_ASSERT(componentLocation != nullptr, "faild to allocate memory for component"); // validate component was successfully created 

			*new(componentLocation) T(); // create component in pre allocated memory
			m_Entitys[entity].m_Components.push_back({ pool, componentIndex }); // add component to entity's list of components
			return *(T*)componentLocation; // return component
		}

		template<class T>
		void RemoveComponent(EntityType entity)
		{
			uint64 componentID = typeid(T).hash_code(); // get component type id

			ComponentPool* pool = m_Pools[componentID];
			if (pool == nullptr) return;

			pool->Free(entity);

			m_Entitys[entity].RemoveComponentReferance(pool);
		}

		template<class T>
		ComponentPool* GetComponentPool()
		{
			ComponentType typeID = typeid(T).hash_code();
			return m_Pools[typeID];
		}

		void Each(Func func);

	private:

		template<uint64 TSize>
		void* AllocateComponent(ComponentType type, EntityType entity, uint32& index)
		{
			ComponentPool* pool = m_Pools[type];

			if (pool == nullptr)
				pool = m_Pools[type] = new SizeComponentPool<TSize>(type);

			// allocate memory for pool
			index = pool->Allocate(entity);
			return pool->GetComponentMemory(index);
		}

	private:
		std::vector<EntityData> m_Entitys;
		std::vector<EntityType> m_FreeEntitys;
		std::vector<EntityType> m_UsedEntitys;

		std::unordered_map<ComponentType, ComponentPool*> m_Pools;
	};
}
