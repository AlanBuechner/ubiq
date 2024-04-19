#include "pch.h"
#include "SceneSerializer.h"
#include "Entity.h"

#include "Components.h"
#include "TransformComponent.h"
#include "Engine/Renderer/Components/SceneRendererComponents.h"
#include "Engine/Renderer/Components/StaticModelRendererComponent.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

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

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();
		out << YAML::Key << "Name" << YAML::Value << entity.GetName();

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "Engine::TransformComponent";
			out << YAML::BeginMap;

			auto& transform = entity.GetTransform();
			
			out << YAML::Key << "Position" << YAML::Value << transform.GetPosition();
			out << YAML::Key << "Rotation" << YAML::Value << transform.GetRotation();
			out << YAML::Key << "Scale" << YAML::Value << transform.GetScale();

			if(transform.GetParent())
				out << YAML::Key << "Parent" << YAML::Value << (uint64)transform.GetParent().GetUUID();

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
			out << YAML::Key << "Engine::CameraComponent";
			out << YAML::BeginMap;

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

			out << YAML::EndMap; // end camera component
		}

		if (entity.HasComponent<DirectionalLightComponent>())
		{
			out << YAML::Key << "Engine::DirectionalLightComponent";
			out << YAML::BeginMap;

			auto& dirLightComponent = *entity.GetComponent<DirectionalLightComponent>();
			out << YAML::Key << "Direction" << YAML::Value << dirLightComponent.GetDirectinalLight()->GetDirection();
			out << YAML::Key << "Temperature" << YAML::Value << dirLightComponent.GetDirectinalLight()->GetCCT();
			out << YAML::Key << "Color" << YAML::Value << dirLightComponent.GetDirectinalLight()->GetTint();
			out << YAML::Key << "Intensity" << YAML::Value << dirLightComponent.GetDirectinalLight()->GetIntensity();
			out << YAML::Key << "Size" << YAML::Value << dirLightComponent.GetDirectinalLight()->GetSize();

			out << YAML::EndMap;
		}

		if (entity.HasComponent<StaticModelRendererComponent>())
		{
			out << YAML::Key << "Engine::StaticModelRendererComponent";
			out << YAML::BeginMap;

			auto& meshRenderer = *entity.GetComponent<StaticModelRendererComponent>();
			if(meshRenderer.GetModel())
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

			out << YAML::EndMap;
		}

		if (entity.HasComponent<SkyboxComponent>())
		{
			out << YAML::Key << "Engine::SkyboxComponent";
			out << YAML::BeginMap;

			auto& skyboxComponent = *entity.GetComponent<SkyboxComponent>();
			if (skyboxComponent.GetSkyboxTexture())
				out << YAML::Key << "Texture" << YAML::Value << skyboxComponent.GetSkyboxTexture()->GetAssetID();

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
		m_Scene->m_Registry.EachEntity([&](auto entityID)
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
		CREATE_PROFILE_FUNCTIONI();
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		CORE_INFO("Deserializeing Scene {0}", sceneName);
		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64 uuid = entity["Entity"].as<uint64>();

				std::string name;
				name = entity["Name"].as<std::string>();

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

				auto transformComponent = entity["Engine::TransformComponent"];
				if (transformComponent)
				{
					auto& tc = *deserializedEntity.GetComponent<TransformComponent>();

					tc.Owner = deserializedEntity;
					tc.Parent = Entity::null;

					tc.SetPosition(transformComponent["Position"].as<Math::Vector3>());
					tc.SetRotation(transformComponent["Rotation"].as<Math::Vector3>());
					tc.SetScale(transformComponent["Scale"].as<Math::Vector3>());
				}

				auto cameraComponent = entity["Engine::CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = *deserializedEntity.AddComponent<CameraComponent>();

					auto camera = cameraComponent["Camera"];
					cc.Camera->SetProjectionType((SceneCamera::ProjectionType)camera["ProjectionType"].as<int>());
					cc.Camera->SetPerspectiveVerticalFOV(camera["PerspectiveFOV"].as<float>());
					cc.Camera->SetPerspectiveNearClip(camera["PerspectiveNear"].as<float>());
					cc.Camera->SetOrthographicFarClip(camera["PerspectiveFar"].as<float>());

					cc.Camera->SetOrthographicSize(camera["OrthographicSize"].as<float>());
					cc.Camera->SetOrthographicNearClip(camera["OrthographicNear"].as<float>());
					cc.Camera->SetOrthographicFarClip(camera["OrthographicFar"].as<float>());

					cc.Primary = cameraComponent["Primary"].as<bool>();
					cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				auto dirLightComponent = entity["Engine::DirectionalLightComponent"];
				if (dirLightComponent)
				{
					auto& dlc = *deserializedEntity.AddComponent<DirectionalLightComponent>();
					dlc.SetDirection(dirLightComponent["Direction"].as<Math::Vector3>());
					dlc.SetIntensity(dirLightComponent["Intensity"].as<float>());
					dlc.SetTemperature(dirLightComponent["Temperature"].as<float>());
					dlc.SetTint(dirLightComponent["Color"].as<Math::Vector3>());
					dlc.SetSize(dirLightComponent["Size"].as<float>());
				}

				auto staticModelRendererComponent = entity["Engine::StaticModelRendererComponent"];
				if (staticModelRendererComponent)
				{
					auto& mrc = *deserializedEntity.AddComponent<StaticModelRendererComponent>();
					if(staticModelRendererComponent["Model"])
						mrc.SetModel(Application::Get().GetAssetManager().GetAsset<Model>(staticModelRendererComponent["Model"].as<uint64>()));

					if (staticModelRendererComponent["Materials"])
					{
						YAML::Node materials = staticModelRendererComponent["Materials"];

						for (auto& entry : mrc.GetMeshes())
						{
							UUID matID = 0;
							if(materials[entry.m_Name])
								matID = materials[entry.m_Name].as<uint64>();
							entry.m_Material = Application::Get().GetAssetManager().GetAsset<Material>(matID);
						}
					}
					mrc.Invalidate();
				}

				auto skyboxComponent = entity["Engine::SkyboxComponent"];
				if (skyboxComponent)
				{
					auto& sbc = *deserializedEntity.AddComponent<SkyboxComponent>();
					if (skyboxComponent["Texture"])
						sbc.SetSkyboxTexture(Application::Get().GetAssetManager().GetAsset<Texture2D>(skyboxComponent["Texture"].as<uint64>()));
				}
			}

			// update entity and component refs
			for (auto entity : entities)
			{
				Entity deserializedEntity = m_Scene->GetEntityWithUUID(entity["Entity"].as<uint64>());

				auto transformComponent = entity["Engine::TransformComponent"];
				if (transformComponent)
				{
					auto& tc = *deserializedEntity.GetComponent<TransformComponent>();

					if (transformComponent["Children"])
					{
						YAML::Node chldren = transformComponent["Children"];
						for (auto c : chldren)
							tc.AddChild(m_Scene->GetEntityWithUUID(c.as<uint64>()));
					}
				}
			}
		}
		CORE_INFO("Finished Deserializeing Scene {0}", sceneName);
		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		// not implemented
		CORE_ASSERT(false, "not implemented");
		return false;
	}

}
