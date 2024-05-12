#pragma once

#include "Scene.h"
#include <yaml-cpp/yaml.h>

namespace Engine
{
	class ComponentSerializer
	{
	public:
		virtual void Serialize(Entity entity, YAML::Emitter& out) = 0;
		virtual void Deserialize(Entity entity, YAML::Node data) = 0;
		virtual void Patch(Entity entity) {};
	};

	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const std::string& filepath);
		void SerializeRuntime(const std::string& filepath);

		bool Deserialize(const std::string& filepath);
		bool DeserializeRuntime(const std::string& filepath);

	private:
		Ref<Scene> m_Scene;

	public:
		static std::unordered_map<uint64, ComponentSerializer*> s_ComponentSerializers;
		class AddComponentSerializer
		{
		public:
			AddComponentSerializer(uint64 typeID, ComponentSerializer* serializer) {
				s_ComponentSerializers.emplace(typeID, serializer);
			}
		};
#define ADD_COMPONENT_SERIALIZER(type, serializer) Engine::SceneSerializer::AddComponentSerializer CAT(Serialize,__LINE__)(typeid(type).hash_code(), new serializer());
	};
}
