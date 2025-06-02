#include "pch.h"
#include "Utils/Performance.h"
#include "Entity.h"
#include "SceneSerializer.h"

#include "Components.h"

#include "Engine/Core/ObjectDescription/ObjectDescription.h"
#include "Engine/Core/ObjectDescription/YAMLObjectDescription.h"

#include <yaml-cpp/yaml.h>
#include <fstream>


namespace Engine
{

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene) :
		m_Scene(scene)
	{
	}

	void SceneSerializer::Serialize(const fs::path& filepath)
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

				auto funcs = ConverterBase::GetObjectConverterFunctions().find(typeid(Entity).hash_code());
				if (funcs != ConverterBase::GetObjectConverterFunctions().end())
					WriteObjectDescriptionToYAML(funcs->second->EncodeObj(&entity), out);
				else
				{
					// TODO : serialize component with reflection
				}
			});

		out << YAML::EndSeq;
		out << YAML::EndMap;

		fs::create_directories(filepath.parent_path());
		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const fs::path& filepath)
	{
		// not implemented
		CORE_ASSERT(false, "not implemented");
	}

	bool SceneSerializer::Deserialize(const fs::path& filepath)
	{
		CREATE_PROFILE_FUNCTIONI();
		ObjectDescription data = LoadObjectDescriptionFromYAMLFile(filepath);

		if (!data.HasEntery("Scene"))
			return false;

		std::string sceneName = data["Scene"].GetAsString();
		if (data.HasEntery("Entities"))
		{
			auto entities = data["Entities"];
			CREATE_PROFILE_SCOPEI("Load Components");

			// load Entities
			for (auto entity : entities.GetAsObjectArray())
			{
				// create entity
				const std::string& name = entity["Name"].GetAsString();
				CREATE_PROFILE_SCOPEI_DYNAMIC("Creating Entity " + name);
				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(entity["Entity"].Get<uint64>(), name);

				for (auto component : entity["Components"].GetAsObjectArray())
				{
					const std::string& componentName = component["Component"].GetAsString();
					const Reflect::Class* componentClass = Reflect::Registry::GetRegistry()->GetClass(componentName);
					CORE_ASSERT_CONTINUE(componentClass != nullptr, "Could not find reflection data for class { 0 }", componentName);
					deserializedEntity.GetScene()->GetRegistry().AddComponent(deserializedEntity, m_Scene.get(), *componentClass);
				}
			}


			// load entity components
			for (auto entity : entities.GetAsObjectArray())
			{
				uint64 uuid = entity["Entity"].Get<uint64>();
				Entity deserializedEntity = m_Scene->GetEntityWithUUID(uuid);

				// deserialize components
				for (auto component : entity["Components"].GetAsObjectArray())
					deserializedEntity.LoadComponentFromDescription(component);
			}
		}
		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const fs::path& filepath)
	{
		// not implemented
		CORE_ASSERT(false, "not implemented");
		return false;
	}

	ObjectDescription SceneSerializer::GetComponentObjectDescription(Component* comp)
	{
		ObjectDescription desc;
		const Reflect::Class& componentClass = comp->GetClass();

		auto funcs = ConverterBase::GetObjectConverterFunctions().find(componentClass.GetTypeID());
		if (funcs != ConverterBase::GetObjectConverterFunctions().end())
		{
			desc = funcs->second->EncodeObj(comp);
			desc.GetAsDescriptionMap()["Component"] = ObjectDescription::CreateFrom(componentClass.GetSname());
		}

		return desc;
	}

}
