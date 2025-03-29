#include "pch.h"
#include "SceneRegistry.h"
#include "Components.h"
#include "Scene.h"

namespace Engine
{


	void EntityData::RemoveComponentReferance(ComponentPool* pool)
	{
		Utils::Vector<ComponentRef>::ConstIterator i = std::find_if(m_Components.begin(), m_Components.end(), [pool](ComponentRef comp) {return comp.m_Pool == pool; });
		m_Components.Remove(i);
	}

	SceneRegistry::~SceneRegistry()
	{
		for (auto& pool : m_Pools)
			delete pool.second;
	}

	EntityType SceneRegistry::CreateEntity()
	{
		return CreateEntity(UUID(), "");
	}

	EntityType SceneRegistry::CreateEntity(UUID id, const std::string& name)
	{
		if (m_FreeEntitys.Empty())
		{
			// no locations in the entity list is free
			m_Entitys.Push({ id, name });
			m_UsedEntitys.Push(m_Entitys.Count()-1);
			return m_Entitys.Count() - 1;
		}

		EntityType entity = m_FreeEntitys.Back(); // get the most recently deleted entity
		m_FreeEntitys.Pop();
		m_Entitys[entity] = { id, name };
		m_UsedEntitys.Push(entity);
		return entity;
	}

	void SceneRegistry::DestroyEntity(EntityType entity)
	{
		// remove components from entity
		for (auto& compRef : m_Entitys[entity].m_Components)
			compRef.m_Pool->Free(entity);
		m_Entitys[entity].m_Components.Clear();

		// remove entity from used entity list
		for (uint32 i = 0; i < m_UsedEntitys.Count(); i++)
		{
			if (m_UsedEntitys[i] == entity)
			{
				// swap and pop
				m_UsedEntitys[i] = m_UsedEntitys.Back();
				m_UsedEntitys.Pop();
			}
		}

		// add entity to free entity list
		m_FreeEntitys.Push(entity);
	}

	void* SceneRegistry::AddComponent(EntityType entity, Scene* scene, const Reflect::Class& componentClass)
	{
		// check if entity already has component
		if (HasComponent(entity, componentClass))
			return nullptr;

		// find the component pool
		ComponentPool* pool = GetOrCreateCompnentPool(componentClass);

		// allocate memory for component
		uint32 componentIndex = pool->Allocate(entity);
		if (componentIndex == UINT32_MAX)
			return nullptr;

		void* componentLocation = pool->GetComponentRaw(componentIndex);
		CORE_ASSERT(componentLocation != nullptr, "faild to allocate memory for component"); // validate component was successfully created 

		Component* comp = (Component*)componentClass.CreateInstance(componentLocation); // create component in pre allocated memory
		m_Entitys[entity].m_Components.Push({ pool, componentIndex }); // add component to entity's list of components
		comp->Owner = { entity, scene };
		comp->OnComponentAdded();
		return comp; // return component
	}

	void SceneRegistry::RemoveComponent(EntityType entity, const Reflect::Class& componentClass)
	{
		ComponentType componentID = componentClass.GetTypeID();

		ComponentPool* pool = m_Pools[componentID];
		if (pool == nullptr) return;

		pool->Free(entity);

		m_Entitys[entity].RemoveComponentReferance(pool);
	}

	void* SceneRegistry::GetSceneStatic(const Reflect::Class& componentClass)
	{
		ComponentType componentID = componentClass.GetTypeID();

		ComponentPool* pool = m_Pools[componentID];
		if (pool == nullptr) return nullptr;

		if (pool->GetNumComponents() == 0)
			return nullptr;

		return pool->GetComponentRaw(0);
	}

	void* SceneRegistry::GetComponent(EntityType entity, const Reflect::Class& componentClass)
	{
		if (HasComponent(entity, componentClass))
			return GetOrCreateCompnentPool(componentClass)->GetComponentForEntityRaw(entity);
		return nullptr;
	}

	bool SceneRegistry::HasComponent(EntityType entity, const Reflect::Class& componentClass)
	{
		if (m_Entitys.Count() <= entity)
			return false;

		ComponentType componentID = componentClass.GetTypeID();
		EntityData& data = m_Entitys[entity];
		for (auto& compRef : data.m_Components)
		{
			if (compRef.m_Pool->GetTypeID() == componentID)
				return true;
		}
		return false;
	}

	ComponentPool* SceneRegistry::GetOrCreateCompnentPool(const Reflect::Class& componentClass)
	{
		ComponentType componentID = componentClass.GetTypeID();
		ComponentPool* pool = m_Pools[componentID];
		if (pool == nullptr)
			pool = m_Pools[componentID] = new ComponentPool(componentClass);
		return pool;
	}

	void SceneRegistry::EachEntity(EachEntityFunc func)
	{
		for (EntityType entity : m_UsedEntitys)
			func(entity);
	}

	void SceneRegistry::EachPool(EachComponentFunc func)
	{
		for (std::pair<ComponentType, ComponentPool*> pool : m_Pools)
			func(pool.second);
	}

}

