#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/UUID.h"
#include "ComponentPool.h"
#include "ComponentView.h"
#include <vector>
#include <unordered_map>

namespace Engine
{

	inline constexpr auto NullEntity = UINT64_MAX;

	struct EntityData
	{
		UUID ID;
		std::string name;

		struct ComponentRef {
			ComponentPool* m_Pool;
			uint32 m_ComponentLoc;
		};
		std::vector<ComponentRef> m_Components;

		void RemoveComponentReferance(ComponentPool* pool);

	};
	using EntityType = uint64;

	class SceneRegistry
	{
	public:
		using Func = std::function<void (EntityType)>;

		~SceneRegistry();

		using iterator = std::vector<EntityType>::iterator;

		iterator begin() { return m_UsedEntitys.begin(); }
		iterator end() { return m_UsedEntitys.end(); }

		EntityType CreateEntity();
		EntityType CreateEntity(UUID id, const std::string& name);
		void DestroyEntity(EntityType entity);

		// EntityData Does not have referential integrity
		EntityData& GetEntityData(EntityType entity) { return m_Entitys[entity]; }

		template<class T, typename... Args>
		T& AddComponent(EntityType entity, Args&&... args)
		{
			//uint64 componentID = typeid(T).hash_code(); // get component type id

			// find the component pool
			ComponentPool* pool = GetOrCreateCompnentPool<T>();

			// allocate memory for component
			uint32 componentIndex = pool->Allocate(entity);
			void* componentLocation = pool->GetComponentMemory(componentIndex);
			CORE_ASSERT(componentLocation != nullptr, "faild to allocate memory for component"); // validate component was successfully created 

			T* comp = new(componentLocation) T(std::forward<Args>(args)...); // create component in pre allocated memory
			m_Entitys[entity].m_Components.push_back({ pool, componentIndex }); // add component to entity's list of components
			return *comp; // return component
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
			return GetOrCreateCompnentPool<T>();;
		}

		template<class T>
		T& GetComponent(EntityType entity)
		{
			return *(T*)GetComponentPool<T>()->GetEntityComponentMemory(entity);
		}

		template<class T>
		bool HasComponent(EntityType entity)
		{
			if (m_Entitys.size() <= entity)
				return false;

			ComponentType componentID = typeid(T).hash_code();
			EntityData& data = m_Entitys[entity];
			for (auto& compRef : data.m_Components)
			{
				if (compRef.m_Pool->GetTypeID() == componentID)
					return true;
			}
			return false;
		}

		void Each(Func func);

		template<class T>
		ComponentView<T> View()
		{
			//uint64 componentID = typeid(T).hash_code();
			ComponentPool* pool = GetOrCreateCompnentPool<T>();
			return ComponentView<T>((SizeComponentPool<sizeof(T)>*)pool);
		}

		template<class T>
		static void ComponentDestructorFunc(void* comp)
		{
			((T*)comp)->~T();
		}

		template<class T>
		ComponentPool* GetOrCreateCompnentPool()
		{
			uint64 componentID = typeid(T).hash_code();
			ComponentPool* pool = m_Pools[componentID];
			if (pool == nullptr) 
				pool = m_Pools[componentID] = new SizeComponentPool<sizeof(T)>(componentID, ComponentDestructorFunc<T>);
			return pool;
		}

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
