#include "pch.h"
#include "StaticModelRendererComponent.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Core/Scene/TransformComponent.h"


namespace Game
{

	// mesh component
	void StaticModelRendererComponent::OnComponentRemoved()
	{
		for (uint32 i = 0; i < m_CollapsedModelTree.size(); i++)
		{
			MeshEntry& entry = m_CollapsedModelTree[i];
			if (entry.m_Object) entry.m_Object->DestroySelf();
		}
	}

	void StaticModelRendererComponent::OnTransformChange(const Math::Mat4& transform)
	{
		for (uint32 i = 0; i < m_CollapsedModelTree.size(); i++)
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
		for (uint32 i = 0; i < m_CollapsedModelTree.size(); i++)
		{
			MeshEntry& entry = m_CollapsedModelTree[i];
			if (entry.m_Object) entry.m_Object->DestroySelf();
		}
		m_CollapsedModelTree.clear();

		if (model)
		{
			for (uint32 i = 0; i < model->GetBakedMeshes().size(); i++)
			{
				m_CollapsedModelTree.push_back({
					model->GetNames()[i],
					model->GetBakedMeshes()[i],
				});
			}
		}
	}

	void StaticModelRendererComponent::Invalidate()
	{
		for (uint32 i = 0; i < m_CollapsedModelTree.size(); i++)
		{
			MeshEntry& entry = m_CollapsedModelTree[i];
			Engine::Ref<Engine::Material> mat = Engine::Renderer::GetDefultMaterial();
			if (entry.m_Material) mat = entry.m_Material;

			if (mat)
			{
				if (entry.m_Object) entry.m_Object->DestroySelf();
				entry.m_Object = Owner.GetScene()->GetSceneRendererAs<SceneRenderer>()->GetMainPass().Submit(entry.m_Mesh, mat, Owner.GetTransform().GetTransform());
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
#if defined(EDITOR)

#include "Editor/Panels/PropertiesPanel.h"
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

		for (uint32_t i = 0; i < component.GetMeshes().size(); i++)
		{
			auto& entry = component.GetMeshes()[i];
			if (Editor::PropertysPanel::DrawMaterialControl(entry.m_Name, entry.m_Material))
				component.Invalidate();
		}
		return changed;
	});
}
#endif
#pragma endregion


#pragma region Serialization
#include "Engine/Core/ObjectDescription/ObjectDescription.h"


namespace Engine
{
	template<>
	struct Convert<Game::StaticModelRendererComponent>
	{
		CONVERTER_BASE(Game::StaticModelRendererComponent);

		static ObjectDescription Encode(const Game::StaticModelRendererComponent& val)
		{
			ObjectDescription desc(ObjectDescription::Type::Object);

			if (val.GetModel())
				desc["Model"] = ObjectDescription::CreateFrom<uint64>(val.GetModel()->GetAssetID());

			if (!val.GetMeshes().empty())
			{
				ObjectDescription materials(ObjectDescription::Type::Object);

				for (auto& entry : val.GetMeshes())
				{
					uint64 matID = 0;
					if (entry.m_Material)
						matID = (uint64)entry.m_Material->GetAssetID();

					materials[entry.m_Name] = ObjectDescription::CreateFrom(matID);
				}

				desc["Materials"] = materials;
			}

			return desc;
		}

		static bool Decode(Game::StaticModelRendererComponent& comp, const ObjectDescription& data)
		{
			if (data.HasEntery("Model"))
				comp.SetModel(Engine::Application::Get().GetAssetManager().GetAsset<Game::Model>(data["Model"].Get<uint64>()));

			if (data.HasEntery("Materials"))
			{
				auto& materials = data["Materials"];
				for (auto& entry : comp.GetMeshes())
				{
					uint64 matID = 0;
					if (materials.HasEntery(entry.m_Name))
						matID = materials[entry.m_Name].Get<uint64>();

					entry.m_Material = Engine::Application::Get().GetAssetManager().GetAsset<Engine::Material>(matID);
				}
			}
			return true;
		}
	};
	ADD_OBJECT_CONVERTER(Game::StaticModelRendererComponent);

}
#pragma endregion
