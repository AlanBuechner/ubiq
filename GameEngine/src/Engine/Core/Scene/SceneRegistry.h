#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/UUID.h"
//#include "Components.h"
#include "ComponentPool.h"
#include "ComponentView.h"
#include "Engine/Util/Performance.h"
#include <vector>
#include <unordered_map>
#include <type_traits>

#include "Engine/Physics/AABB.h"
#include "Engine/Physics/Plain.h"

namespace Engine
{

	inline constexpr auto NullEntity = UINT64_MAX;

	struct EntityData
	{
		UUID ID;
		std::string name;
		AABB aabb;
		PlainVolume volume;

		bool dirtyAABB = true;
		bool dirtyVolume = true;

		struct ComponentRef {
			ComponentPool* m_Pool;
			uint32 m_ComponentLoc;
		};
		Utils::Vector<ComponentRef> m_Components;

		void RemoveComponentReferance(ComponentPool* pool);
	};
	using EntityType = uint64;

	class Scene;

	class SceneRegistry
	{
	public:
		using EachEntityFunc = std::function<void (EntityType)>;
		using EachComponentFunc = std::function<void (ComponentPool*)>;

		~SceneRegistry();

		using Iterator = Utils::Vector<EntityType>::Iterator;

		Iterator begin() { return m_UsedEntitys.begin(); }
		Iterator end() { return m_UsedEntitys.end(); }

		EntityType CreateEntity();
		EntityType CreateEntity(UUID id, const std::string& name);
		void DestroyEntity(EntityType entity);

		// EntityData Does not have referential integrity
		EntityData& GetEntityData(EntityType entity) { return m_Entitys[entity]; }

		template<class T>
		T* AddComponent(EntityType entity, Scene* scene) { return (T*)AddComponent(entity, scene, T::GetStaticClass()); }
		void* AddComponent(EntityType entity, Scene* scene, const Reflect::Class& componentClass);

		template<class T>
		void RemoveComponent(EntityType entity) { RemoveComponent(entity, T::GetStaticClass()); }
		void RemoveComponent(EntityType entity, const Reflect::Class& componentClass);

		template<class T>
		T* GetSceneStatic() { return (T*)GetSceneStatic(T::GetStaticClass()); }
		void* GetSceneStatic(const Reflect::Class& componentClass);

		template<class T>
		T* GetComponent(EntityType entity) { return (T*)GetComponent(entity, T::GetStaticClass()); }
		void* GetComponent(EntityType entity, const Reflect::Class& componentClass);

		template<class T>
		bool HasComponent(EntityType entity) { return HasComponent(entity, T::GetStaticClass()); }
		bool HasComponent(EntityType entity, const Reflect::Class& componentClass);

		template<class T>
		ComponentView<T> View() { return ComponentView<T>(GetOrCreateCompnentPool<T>()); }

		template<class T>
		ComponentPool* GetComponentPool() { return GetOrCreateCompnentPool<T>(); }
		template<class T>
		ComponentPool* GetOrCreateCompnentPool() { return GetOrCreateCompnentPool(T::GetStaticClass()); }
		ComponentPool* GetOrCreateCompnentPool(const Reflect::Class& reflectedClass);

		void EachEntity(EachEntityFunc func);
		void EachPool(EachComponentFunc func);

	private:
		Utils::Vector<EntityData> m_Entitys;
		Utils::Vector<EntityType> m_FreeEntitys;
		Utils::Vector<EntityType> m_UsedEntitys;

		std::unordered_map<ComponentType, ComponentPool*> m_Pools;
	};
}
