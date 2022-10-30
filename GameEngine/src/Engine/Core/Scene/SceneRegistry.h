#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/UUID.h"
#include "ComponentPool.h"
#include <vector>

namespace Engine
{

	struct EntityData
	{
		UUID ID;
		std::string name;

		struct ComponentRef {
			ComponentType m_Type;
			void* m_ComponentLoc;
		};
		std::vector<ComponentRef> m_Components;

		~EntityData();

	};
	using EntityType = uint64;

	class SceneRegistry
	{
	public:
		using Func = std::function<void (EntityType)>;

		EntityType CreateEntity();
		EntityType CreateEntity(UUID id);
		void DestroyEntity(EntityType entity);

		// EntityData Does not have referential integrity
		EntityData& GetEntityData(EntityType entity) { return m_Entitys[entity]; }

		template<class T>
		T& AddComponent(EntityType entity)
		{
			uint64 componentID = typeid(T).hash_code(); // get component type id
			void* componentLocation = AllocateComponent(componentID, sizeof(T), entity); // allocate memory for the component
			CORE_ASSERT(componentLocation != nullptr, "faild to allocate memory for component"); // validate component was successfully created 
			*new(componentLocation) T(); // create component in pre allocated memory
			m_Entitys[entity].m_Components.push_back({ componentID, componentLocation }); // add component to entity's list of components
			return *(T*)componentLocation; // return component
		}

		template<class T>
		ComponentPool* GetComponentPool()
		{
			ComponentType typeID = typeid(T).hash_code();
			for (auto& pool : m_Pools)
			{
				if (pool.GetTypeID() == typeID)
					return &pool;
			}
			return nullptr;
		}

		void Each(Func func);

	private:
		void* AllocateComponent(ComponentType type, uint64 componentSize, EntityType entity);

	private:
		std::vector<EntityData> m_Entitys;
		std::vector<EntityType> m_FreeEntitys;
		std::vector<EntityType> m_UsedEntitys;

		std::vector<ComponentPool> m_Pools;
	};
}
