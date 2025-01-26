#include "pch.h"
#include "Entity.h"
#include "Components.h"
#include "TransformComponent.h"

Engine::Entity Engine::Entity::null = Engine::Entity();

namespace Engine
{

	Entity::Entity(EntityType handle, Scene* scene) :
		m_EntityID(handle), m_Scene(scene)
	{
	}

	void Entity::RemoveComponent(Component* comp)
	{
		if (comp != nullptr)
		{
			comp->OnComponentRemoved();
			m_Scene->m_Registry.RemoveComponent(m_EntityID, comp->GetClass());
		}
	}

	UUID Entity::GetUUID() const
	{
		if (*this)
			return m_Scene->m_Registry.GetEntityData(m_EntityID).ID;
		else
			return 0;
	}

	std::string& Entity::GetName() const
	{
		return m_Scene->m_Registry.GetEntityData(m_EntityID).name;
	}

	TransformComponent& Entity::GetTransform() const
	{
		TransformComponent* comp = GetComponent<TransformComponent>();
		CORE_ASSERT(comp != nullptr, "entety does not have transform");
		return *comp;
	}

	Entity Entity::GetParent()
	{
		return GetTransform().GetParent();
	}

	void Entity::AddChild(Entity child)
	{
		GetTransform().AddChild(child);
	}

	void Entity::RemoveChild(Entity child)
	{
		GetTransform().RemoveChild(child);
	}

	const Utils::Vector<Entity>& Entity::GetChildren() const
	{
		return GetTransform().GetChildren();
	}

	void Entity::SetParentToRoot()
	{
		GetTransform().SetParentToRoot();
	}

	Utils::Vector<Component*> Entity::GetComponents() const
	{
		EntityData& data = m_Scene->m_Registry.GetEntityData(m_EntityID);
		Utils::Vector<Component*> components(data.m_Components.Count(), {});
		for (uint32 i = 0; i < components.Count(); i++)
			components[i] = (Component*)(data.m_Components[i].m_Pool->GetComponentRaw(data.m_Components[i].m_ComponentLoc));

		return components;
	}

	Entity Entity::FindChiledWithName(const std::string& name)
	{
		TransformComponent& transform = GetTransform();
		for (uint32 i = 0; i < transform.GetChildren().Count(); i++)
		{
			if (transform.GetChildren()[i].GetName() == name)
				return transform.GetChildren()[i];
		}
		return Entity::null;
	}

	void Entity::DirtyAABB()
	{
		EntityData& data = m_Scene->m_Registry.GetEntityData(m_EntityID);
		data.dirtyAABB = true;
		data.dirtyVolume = true;
	}

	void Entity::DirtyVolume()
	{
		EntityData& data = m_Scene->m_Registry.GetEntityData(m_EntityID);
		data.dirtyVolume = true;
	}

	AABB Entity::GetLocalAABB()
	{
		EntityData& data = m_Scene->m_Registry.GetEntityData(m_EntityID);
		if (data.dirtyAABB)
		{
			Utils::Vector<AABB> aabbs;
			for (Component* component : GetComponents())
			{
				Utils::Vector<AABB> componentVolumes = component->GetVolumes();
				for (AABB aabb : componentVolumes)
					aabbs.Push(aabb);
			}

			data.aabb = AABB::GetExtents(aabbs);
			if(data.aabb.Valid())
				data.aabb.MinThickness(0.001f);
			data.dirtyAABB = false;
		}

		return data.aabb;
	}

	PlainVolume Entity::GetPlainVolume()
	{
		EntityData& data = m_Scene->m_Registry.GetEntityData(m_EntityID);
		if (data.dirtyVolume)
		{
			AABB aabb = GetLocalAABB();
			data.volume = aabb.GetPlainVolume();
			data.volume.Transform(GetTransform().GetGlobalTransform());
			data.dirtyVolume = false;
		}

		return data.volume;
	}




	// ------------------- Converter ------------------- //
	template<>
	struct Convert<Entity>
	{
		CONVERTER_BASE(Entity);
		static ObjectDescription Encode(const Entity& entity)
		{
			ObjectDescription entityDesc = ObjectDescription(ObjectDescription::Type::Object);

			entityDesc["Entity"] = ObjectDescription::CreateFrom((uint64)entity.GetUUID());
			entityDesc["Name"] = ObjectDescription::CreateFrom(entity.GetName());

			// serialize each component
			ObjectDescription& comps = entityDesc["Components"].SetType(ObjectDescription::Type::Array);
			comps.GetAsObjectArray().Reserve(entity.GetComponents().Count());
			for (Component* comp : entity.GetComponents())
			{
				const Reflect::Class& componentClass = comp->GetClass();

				auto funcs = ConverterBase::GetObjectConverterFunctions().find(componentClass.GetTypeID());
				if (funcs != ConverterBase::GetObjectConverterFunctions().end())
				{
					comps.GetAsObjectArray().Push(funcs->second->EncodeObj(comp));
					comps.GetAsObjectArray().Back().GetAsDescriptionMap()["Component"] = ObjectDescription::CreateFrom(componentClass.GetSname());
				}
			}
			return entityDesc;
		}

		static bool Decode(Entity& entity, const ObjectDescription& data)
		{
			CORE_ERROR("Can not deserealize entity wihout scene please use scene serializer");
			return false;
		}
	};
	ADD_OBJECT_CONVERTER(Entity);
	ObjectDescription Entity::CreateObjectDescription() const
	{
		return Convert<Entity>::Encode(*this);
	}

}
