#include "pch.h"
#include "SceneSerializer.h"
#include "Components.h"
#include "Entity.h"

#include "Engine/Physics/PhysicsComponent.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

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

namespace Engine
{

	YAML::Emitter& operator<<(YAML::Emitter& out, const Math::Vector2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Math::Vector3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Math::Vector4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene) : 
		m_Scene(scene)
	{
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		if (entity.HasComponent<EntityDataComponent>())
		{
			out << YAML::Key << "EntityDataComponent";
			out << YAML::BeginMap;
			
			auto& Name = entity.GetComponent<EntityDataComponent>().Name;

			out << YAML::Key << "Name" << YAML::Value << Name;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;

			auto& transform = entity.GetComponent<TransformComponent>();
			
			out << YAML::Key << "Position" << YAML::Value << transform.Position;
			out << YAML::Key << "Rotation" << YAML::Value << transform.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << transform.Scale;

			if (transform.GetChildren().size() > 0)
			{
				YAML::Node children;
				for (auto c : transform.GetChildren())
					children.push_back((uint64)c.GetUUID());
				out << YAML::Key << "Children" << YAML::Value << children;
			}

			out << YAML::EndMap;
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;
			out << YAML::Key << "Camera";
			out << YAML::BeginMap;
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap; // end camera

			out << YAML::EndMap; // end camera component
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap;

			auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << spriteRenderer.Color;
			if (spriteRenderer.Texture)
				out << YAML::Key << "TextureID" << YAML::Value << spriteRenderer.Texture->GetAssetID();

			out << YAML::EndMap;
		}

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap;

			auto& component = entity.GetComponent<Rigidbody2DComponent>();
			out << YAML::Key << "Type";
			switch (component.Type)
			{
			case Engine::Rigidbody2DComponent::BodyType::Static:	out << YAML::Value << "Static"; break;
			case Engine::Rigidbody2DComponent::BodyType::Dynamic:	out << YAML::Value << "Dynamic"; break;
			case Engine::Rigidbody2DComponent::BodyType::Kinematic:	out << YAML::Value << "Kinematic"; break;
			}
			out << YAML::Key << "FixedRotation" << YAML::Value << component.FixedRotation;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap;

			auto& component = entity.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << component.Offset;
			out << YAML::Key << "Size" << YAML::Value << component.Size;
			out << YAML::Key << "Density" << YAML::Value << component.Density;
			out << YAML::Key << "Friction" << YAML::Value << component.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << component.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << component.RestitutionThreshold;

			out << YAML::EndMap;
		}
		
		if (entity.HasComponent<CircleColliderComponent>())
		{
			out << YAML::Key << "CircleColiderComponent";
			out << YAML::BeginMap;

			auto& component = entity.GetComponent<CircleColliderComponent>();
			out << YAML::Key << "Offset" << YAML::Value << component.Offset;
			out << YAML::Key << "Size" << YAML::Value << component.Size;
			out << YAML::Key << "Density" << YAML::Value << component.Density;
			out << YAML::Key << "Friction" << YAML::Value << component.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << component.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << component.RestitutionThreshold;

			out << YAML::EndMap;
		}

		out << YAML::EndMap;
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene";
		out << YAML::Value << "Name";
		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.each([&](auto entityID)
			{
				Entity entity = {entityID, m_Scene.get()};
				if (!entity)
					return;

				SerializeEntity(out, entity);
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
			for (auto entity : entities)
			{
				uint64 uuid = entity["Entity"].as<uint64>();

				std::string name;
				auto dataComponent = entity["EntityDataComponent"];
				if (dataComponent)
					name = dataComponent["Name"].as<std::string>();

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					tc.Position = transformComponent["Position"].as<Math::Vector3>();
					tc.Rotation = transformComponent["Rotation"].as<Math::Vector3>();
					tc.Scale = transformComponent["Scale"].as<Math::Vector3>();

					tc.Owner = deserializedEntity;
					tc.Parent = Entity::null;
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = deserializedEntity.AddComponent<CameraComponent>();

					auto camera = cameraComponent["Camera"];
					cc.Camera.SetProjectionType((SceneCamera::ProjectionType)camera["ProjectionType"].as<int>());
					cc.Camera.SetPerspectiveVerticalFOV(camera["PerspectiveFOV"].as<float>());
					cc.Camera.SetPerspectiveNearClip(camera["PerspectiveNear"].as<float>());
					cc.Camera.SetOrthographicFarClip(camera["PerspectiveFar"].as<float>());

					cc.Camera.SetOrthographicSize(camera["OrthographicSize"].as<float>());
					cc.Camera.SetOrthographicNearClip(camera["OrthographicNear"].as<float>());
					cc.Camera.SetOrthographicFarClip(camera["OrthographicFar"].as<float>());

					cc.Primary = cameraComponent["Primary"].as<bool>();
					cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				auto spriteRendererComponent = entity["SpriteRendererComponent"];
				if (spriteRendererComponent)
				{
					auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
					src.Color = spriteRendererComponent["Color"].as<Math::Vector4>();
					if (spriteRendererComponent["TextureID"])
						src.Texture = Application::Get().GetAssetManager().GetAsset<Texture2D>(spriteRendererComponent["TextureID"].as<uint64>());
				}

				auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
				if (rigidbody2DComponent)
				{
					auto& rbc = deserializedEntity.AddComponent<Rigidbody2DComponent>();

					std::string type = rigidbody2DComponent["Type"].as<std::string>();
					if (type == "Static") rbc.Type = Rigidbody2DComponent::BodyType::Static;
					else if (type == "Dynamic") rbc.Type = Rigidbody2DComponent::BodyType::Dynamic;
					else if (type == "Kinematic") rbc.Type = Rigidbody2DComponent::BodyType::Kinematic;

					rbc.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
				}

				auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
				if (boxCollider2DComponent)
				{
					auto& bcc = deserializedEntity.AddComponent<BoxCollider2DComponent>();
					bcc.Offset = boxCollider2DComponent["Offset"].as<Math::Vector2>();
					bcc.Size = boxCollider2DComponent["Size"].as<Math::Vector2>();
					bcc.Density = boxCollider2DComponent["Density"].as<float>();
					bcc.Friction = boxCollider2DComponent["Friction"].as<float>();
					bcc.Restitution = boxCollider2DComponent["Restitution"].as<float>();
					bcc.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
				}
				
				auto circleCollider2DComponent = entity["CircleColiderComponent"];
				if (circleCollider2DComponent)
				{
					auto& bcc = deserializedEntity.AddComponent<CircleColliderComponent>();
					bcc.Offset = circleCollider2DComponent["Offset"].as<Math::Vector2>();
					bcc.Size = circleCollider2DComponent["Size"].as<float>();
					bcc.Density = circleCollider2DComponent["Density"].as<float>();
					bcc.Friction = circleCollider2DComponent["Friction"].as<float>();
					bcc.Restitution = circleCollider2DComponent["Restitution"].as<float>();
					bcc.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
				}
			}

			// update entity and component refs
			for (auto entity : entities)
			{
				Entity deserializedEntity = m_Scene->GetEntityWithUUID(entity["Entity"].as<uint64>());

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();

					if (transformComponent["Children"])
					{
						YAML::Node chldren = transformComponent["Children"];
						for (auto c : chldren)
							tc.AddChild(m_Scene->GetEntityWithUUID(c.as<uint64>()));
					}
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

}
