#include "pch.h"
#include "Entity.h"
#include "SceneSerializer.h"

#include "Components.h"
#include "TransformComponent.h"

#include <fstream>

#include "Engine/Util/Performance.h"

// temp
#include "Engine/Renderer/Abstractions/GPUProfiler.h"

namespace Engine
{

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene) :
		m_Scene(scene)
	{
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene";
		out << YAML::Value << "Name";
		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;

		// Serialize each entity
		m_Scene->m_Registry.EachEntity([&](auto entityID)
			{
				Entity entity = { entityID, m_Scene.get() };
				if (!entity)
					return;

				out << YAML::BeginMap;
				out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();
				out << YAML::Key << "Name" << YAML::Value << entity.GetName();

				// serialize each component
				out << YAML::Key << "Components" << YAML::Value << YAML::BeginSeq;
				for (Component* comp : entity.GetComponents())
				{
					const Reflect::Class& componentClass = comp->GetClass();

					out << YAML::BeginMap;
					out << YAML::Key << "Component" << YAML::Value << componentClass.GetSname();

					auto func = s_ComponentSerializers.find(componentClass.GetTypeID());
					if (func != s_ComponentSerializers.end())
						func->second->Serialize(entity, out);

					out << YAML::EndMap;
				}

				out << YAML::EndSeq;
				out << YAML::EndMap;
			});

		out << YAML::EndSeq;
		out << YAML::EndMap;
		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		// not implemented
		CORE_ASSERT(false, "not implemented");
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		CREATE_PROFILE_FUNCTIONI();
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		auto entities = data["Entities"];
		if (entities)
		{
			// load each entity and its components
			for (auto entity : entities)
			{
				uint64 uuid = entity["Entity"].as<uint64>();

				std::string name;
				name = entity["Name"].as<std::string>();

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

				auto components = entity["Components"];
				for (auto component : components)
				{
					std::string componentName = component["Component"].as<std::string>();
					const Reflect::Class* componentClass = Reflect::Registry::GetRegistry()->GetClass(componentName);
					if (componentClass == nullptr)
					{
						CORE_ERROR("Could not find reflection data for class {0}", componentName);
						break;
					}

					if (componentClass)
						deserializedEntity.GetScene()->GetRegistry().AddComponent(deserializedEntity, m_Scene.get(), *componentClass);
				}
			}

			// load entity components
			for (auto entity : entities)
			{
				Entity deserializedEntity = m_Scene->GetEntityWithUUID(entity["Entity"].as<uint64>());
				auto components = entity["Components"];
				for (auto component : components)
				{
					std::string componentName = component["Component"].as<std::string>();
					const Reflect::Class* componentClass = Reflect::Registry::GetRegistry()->GetClass(componentName);
					if (componentClass == nullptr)
					{
						CORE_ERROR("Could not find reflection data for class {0}", componentName);
						break;
					}

					auto func = s_ComponentSerializers.find(componentClass->GetTypeID());
					if (func != s_ComponentSerializers.end())
						func->second->Deserialize(deserializedEntity, component);
				}
			}

			// patch components
			for (auto entity : entities)
			{
				Entity deserializedEntity = m_Scene->GetEntityWithUUID(entity["Entity"].as<uint64>());
				auto components = entity["Components"];
				for (auto component : components)
				{
					std::string componentName = component["Component"].as<std::string>();
					const Reflect::Class* componentClass = Reflect::Registry::GetRegistry()->GetClass(componentName);
					if (componentClass == nullptr)
					{
						CORE_ERROR("Could not find reflection data for class {0}", componentName);
						break;
					}

					auto func = s_ComponentSerializers.find(componentClass->GetTypeID());
					if (func != s_ComponentSerializers.end())
						func->second->Patch(deserializedEntity);
				}
			}
		}
		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		// not implemented
		CORE_ASSERT(false, "not implemented");
		return false;
	}

	std::unordered_map<uint64, ComponentSerializer*> SceneSerializer::s_ComponentSerializers;
}

class TransformSerializer : public Engine::ComponentSerializer
{
	virtual void Serialize(Engine::Entity entity, YAML::Emitter& out) override
	{
		auto& transform = entity.GetTransform();

		out << YAML::Key << "Position" << YAML::Value << transform.GetPosition();
		out << YAML::Key << "Rotation" << YAML::Value << transform.GetRotation();
		out << YAML::Key << "Scale" << YAML::Value << transform.GetScale();

		if (transform.GetParent())
			out << YAML::Key << "Parent" << YAML::Value << (uint64)transform.GetParent().GetUUID();

		if (transform.GetChildren().size() > 0)
		{
			YAML::Node children;
			for (auto c : transform.GetChildren())
				children.push_back((uint64)c.GetUUID());
			out << YAML::Key << "Children" << YAML::Value << YAML::DoubleQuoted << children;
		}
	}

	virtual void Deserialize(Engine::Entity entity, YAML::Node data) override
	{
		auto& tc = *entity.GetComponent<Engine::TransformComponent>();

		tc.Owner = entity;
		tc.Parent = Engine::Entity::null;

		tc.SetPosition(data["Position"].as<Math::Vector3>());
		tc.SetRotation(data["Rotation"].as<Math::Vector3>());
		tc.SetScale(data["Scale"].as<Math::Vector3>());

		if (data["Children"])
		{
			YAML::Node chldren = data["Children"];
			for (auto c : chldren)
				tc.AddChild(tc.GetOwner().GetScene()->GetEntityWithUUID(c.as<uint64>()));
		}
	}
};
ADD_COMPONENT_SERIALIZER(Engine::TransformComponent, TransformSerializer);
