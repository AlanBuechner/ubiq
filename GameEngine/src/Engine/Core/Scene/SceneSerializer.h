#pragma once

#include "Scene.h"
#include "Entity.h"
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
#define ADD_COMPONENT_SERIALIZER(type, serializer) static Engine::SceneSerializer::AddComponentSerializer CAT(serialize,__LINE__)(typeid(type).hash_code(), new serializer());
	};
}

#pragma region YAML

namespace YAML
{

	template<>
	struct convert<Math::Vector2>
	{
		static Node encode(const Math::Vector2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);

			return node;
		}

		static bool decode(const Node& node, Math::Vector2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Math::Vector3>
	{
		static Node encode(const Math::Vector3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);

			return node;
		}

		static bool decode(const Node& node, Math::Vector3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};


	template<>
	struct convert<Math::Vector4>
	{
		static Node encode(const Math::Vector4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.a);

			return node;
		}

		static bool decode(const Node& node, Math::Vector4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.a = node[3].as<float>();

			return true;
		}
	};
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const Math::Vector2& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
	return out;
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const Math::Vector3& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const Math::Vector4& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
	return out;
}

#pragma endregion
