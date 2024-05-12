#include "pch.h"
#include "Entity.h"
#include "SceneSerializer.h"

#include "Components.h"
#include "TransformComponent.h"
#include "Engine/Renderer/Components/SceneRendererComponents.h"
#include "Engine/Renderer/Components/StaticModelRendererComponent.h"

#include <fstream>

#include "Engine/Util/Performance.h"

// temp
#include "Engine/Renderer/Abstractions/GPUProfiler.h"

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
			Entity entity = {entityID, m_Scene.get()};
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

	class TransformSerializer : public ComponentSerializer
	{
		virtual void Serialize(Entity entity, YAML::Emitter& out) override
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

		virtual void Deserialize(Entity entity, YAML::Node data) override
		{
			auto& tc = *entity.GetComponent<TransformComponent>();

			tc.Owner = entity;
			tc.Parent = Entity::null;

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
	ADD_COMPONENT_SERIALIZER(TransformComponent, TransformSerializer);


	class CameraSerializer : public ComponentSerializer
	{
		virtual void Serialize(Entity entity, YAML::Emitter& out) override
		{
			auto& cameraComponent = *entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;
			out << YAML::Key << "Camera";
			out << YAML::BeginMap;
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera->GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera->GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera->GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera->GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera->GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera->GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera->GetOrthographicFarClip();
			out << YAML::EndMap; // end camera
		}

		virtual void Deserialize(Entity entity, YAML::Node data) override
		{
			auto& cc = *entity.GetComponent<CameraComponent>();

			auto camera = data["Camera"];
			cc.Camera->SetProjectionType((SceneCamera::ProjectionType)camera["ProjectionType"].as<int>());
			cc.Camera->SetPerspectiveVerticalFOV(camera["PerspectiveFOV"].as<float>());
			cc.Camera->SetPerspectiveNearClip(camera["PerspectiveNear"].as<float>());
			cc.Camera->SetOrthographicFarClip(camera["PerspectiveFar"].as<float>());

			cc.Camera->SetOrthographicSize(camera["OrthographicSize"].as<float>());
			cc.Camera->SetOrthographicNearClip(camera["OrthographicNear"].as<float>());
			cc.Camera->SetOrthographicFarClip(camera["OrthographicFar"].as<float>());

			cc.Primary = data["Primary"].as<bool>();
			cc.FixedAspectRatio = data["FixedAspectRatio"].as<bool>();
		}
	};
	ADD_COMPONENT_SERIALIZER(CameraComponent, CameraSerializer);


	class DirectionalLightSerializer : public ComponentSerializer
	{
		virtual void Serialize(Entity entity, YAML::Emitter& out) override
		{
			auto& dirLightComponent = *entity.GetComponent<DirectionalLightComponent>();
			out << YAML::Key << "Direction" << YAML::Value << dirLightComponent.GetDirectinalLight()->GetDirection();
			out << YAML::Key << "Temperature" << YAML::Value << dirLightComponent.GetDirectinalLight()->GetCCT();
			out << YAML::Key << "Color" << YAML::Value << dirLightComponent.GetDirectinalLight()->GetTint();
			out << YAML::Key << "Intensity" << YAML::Value << dirLightComponent.GetDirectinalLight()->GetIntensity();
			out << YAML::Key << "Size" << YAML::Value << dirLightComponent.GetDirectinalLight()->GetSize();
		}

		virtual void Deserialize(Entity entity, YAML::Node data) override
		{
			auto& dlc = *entity.GetComponent<DirectionalLightComponent>();
			dlc.SetDirection(data["Direction"].as<Math::Vector3>());
			dlc.SetIntensity(data["Intensity"].as<float>());
			dlc.SetTemperature(data["Temperature"].as<float>());
			dlc.SetTint(data["Color"].as<Math::Vector3>());
			dlc.SetSize(data["Size"].as<float>());
		}
	};
	ADD_COMPONENT_SERIALIZER(DirectionalLightComponent, DirectionalLightSerializer);


	class StaticModelRendererSerializer : public ComponentSerializer
	{
		virtual void Serialize(Entity entity, YAML::Emitter& out) override
		{
			auto& meshRenderer = *entity.GetComponent<StaticModelRendererComponent>();
			if (meshRenderer.GetModel())
				out << YAML::Key << "Model" << YAML::Value << meshRenderer.GetModel()->GetAssetID();

			if (!meshRenderer.GetMeshes().empty())
			{
				out << YAML::Key << "Materials";
				out << YAML::BeginMap;

				for (auto& entry : meshRenderer.GetMeshes())
				{
					uint64 matID = 0;
					if (entry.m_Material)
						matID = (uint64)entry.m_Material->GetAssetID();

					out << YAML::Key << entry.m_Name << YAML::Value << matID;
				}

				out << YAML::EndMap;
			}
		}

		virtual void Deserialize(Entity entity, YAML::Node data) override
		{
			auto& mrc = *entity.GetComponent<StaticModelRendererComponent>();
			if (data["Model"])
				mrc.SetModel(Application::Get().GetAssetManager().GetAsset<Model>(data["Model"].as<uint64>()));

			if (data["Materials"])
			{
				YAML::Node materials = data["Materials"];

				for (auto& entry : mrc.GetMeshes())
				{
					UUID matID = 0;
					if (materials[entry.m_Name])
						matID = materials[entry.m_Name].as<uint64>();
					entry.m_Material = Application::Get().GetAssetManager().GetAsset<Material>(matID);
				}
			}
			mrc.Invalidate();
		}
	};
	ADD_COMPONENT_SERIALIZER(StaticModelRendererComponent, StaticModelRendererSerializer);


	class SkyboxSerializer : public ComponentSerializer
	{
		virtual void Serialize(Entity entity, YAML::Emitter& out) override
		{
			auto& skyboxComponent = *entity.GetComponent<SkyboxComponent>();
			if (skyboxComponent.GetSkyboxTexture())
				out << YAML::Key << "Texture" << YAML::Value << skyboxComponent.GetSkyboxTexture()->GetAssetID();
		}

		virtual void Deserialize(Entity entity, YAML::Node data) override
		{
			auto& sbc = *entity.GetComponent<SkyboxComponent>();
			if (data["Texture"])
				sbc.SetSkyboxTexture(Application::Get().GetAssetManager().GetAsset<Texture2D>(data["Texture"].as<uint64>()));
		}
	};
	ADD_COMPONENT_SERIALIZER(SkyboxComponent, SkyboxSerializer);

}
