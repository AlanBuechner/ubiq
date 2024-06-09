#include "pch.h"
#include "StaticModelRendererComponent.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Core/Scene/TransformComponent.h"


namespace Game
{

	// mesh component
	void StaticModelRendererComponent::OnComponentRemoved()
	{
		for (uint32 i = 0; i < m_CollapsedModelTree.Count(); i++)
		{
			MeshEntry& entry = m_CollapsedModelTree[i];
			Owner.GetScene()->GetSceneRendererAs<SceneRenderer>()->RemoveObject(entry.m_Object);
		}
	}

	void StaticModelRendererComponent::OnTransformChange(const Math::Mat4& transform)
	{
		for (uint32 i = 0; i < m_CollapsedModelTree.Count(); i++)
		{
			MeshEntry& entry = m_CollapsedModelTree[i];
			if(entry.m_Object)
				entry.m_Object->UpdateTransform(transform);
		}
	}

	void StaticModelRendererComponent::SetModel(Engine::Ref<Model> model)
	{
		m_Model = model;

		// remove old model
		for (uint32 i = 0; i < m_CollapsedModelTree.Count(); i++)
			Owner.GetScene()->GetSceneRendererAs<SceneRenderer>()->RemoveObject(m_CollapsedModelTree[i].m_Object);
		m_CollapsedModelTree.Clear();

		if (model)
		{
			for (uint32 i = 0; i < model->GetBakedMeshes().Count(); i++)
			{
				m_CollapsedModelTree.Push({
					model->GetNames()[i],
					model->GetBakedMeshes()[i],
				});
			}
		}
	}

	void StaticModelRendererComponent::Invalidate()
	{
		for (uint32 i = 0; i < m_CollapsedModelTree.Count(); i++)
		{
			MeshEntry& entry = m_CollapsedModelTree[i];
			Engine::Ref<Engine::Material> mat = Engine::Renderer::GetDefultMaterial();
			if (entry.m_Material) mat = entry.m_Material;

			if (mat)
			{
				Owner.GetScene()->GetSceneRendererAs<SceneRenderer>()->RemoveObject(entry.m_Object);
				entry.m_Object = Owner.GetScene()->GetSceneRendererAs<SceneRenderer>()->Submit(entry.m_Mesh, mat, Owner.GetTransform().GetTransform());
			}
		}

		DirtyVolume();
	}

	Utils::Vector<Engine::AABB> StaticModelRendererComponent::GenVolumes()
	{
		return m_Model->GetVolumes();
	}

}




#pragma region Editor
#include "Panels/PropertiesPanel.h"
#include "Editor/Properties.h"

namespace Game
{
	ADD_EXPOSE_PROP_FUNC(StaticModelRendererComponent) {
		bool changed = false;
		StaticModelRendererComponent& component = *(StaticModelRendererComponent*)voidData;
		Engine::Ref<Model> model = component.GetModel();
		if (DrawModelControl("Model", model))
		{
			component.SetModel(model);
			component.Invalidate();
		}

		for (uint32_t i = 0; i < component.GetMeshes().Count(); i++)
		{
			auto& entry = component.GetMeshes()[i];
			if (Engine::PropertysPanel::DrawMaterialControl(entry.m_Name, entry.m_Material))
				component.Invalidate();
		}
		return changed;
	});
}
#pragma endregion


#pragma region Serialization
#include "Engine/Core/Scene/SceneSerializer.h"
namespace Game
{
	class StaticModelRendererSerializer : public Engine::ComponentSerializer
	{
		virtual void Serialize(Engine::Entity entity, YAML::Emitter& out) override
		{
			auto& meshRenderer = *entity.GetComponent<Game::StaticModelRendererComponent>();
			if (meshRenderer.GetModel())
				out << YAML::Key << "Model" << YAML::Value << meshRenderer.GetModel()->GetAssetID();

			if (!meshRenderer.GetMeshes().Empty())
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

		virtual void Deserialize(Engine::Entity entity, YAML::Node data) override
		{
			auto& mrc = *entity.GetComponent<Game::StaticModelRendererComponent>();
			if (data["Model"])
				mrc.SetModel(Engine::Application::Get().GetAssetManager().GetAsset<Model>(data["Model"].as<uint64>()));

			if (data["Materials"])
			{
				YAML::Node materials = data["Materials"];

				for (auto& entry : mrc.GetMeshes())
				{
					Engine::UUID matID = 0;
					if (materials[entry.m_Name])
						matID = materials[entry.m_Name].as<uint64>();
					entry.m_Material = Engine::Application::Get().GetAssetManager().GetAsset<Engine::Material>(matID);
				}
			}
			mrc.Invalidate();
		}
	};
	ADD_COMPONENT_SERIALIZER(Game::StaticModelRendererComponent, StaticModelRendererSerializer);


}
#pragma endregion
