#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Model.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Core/Scene/Components.h"

namespace Engine
{
	class StaticModelRendererComponent : public Component
	{
	public:

		struct MeshEntry
		{
			Ref<Mesh> m_Mesh;
			Ref<Material> m_Material;
			SceneRenderer::ObjectControlBlockRef m_Object = nullptr;
		};

	public:
		virtual void OnComponentRemoved() override;

		virtual void OnTransformChange(const Math::Mat4& transform) override;

		void SetModel(Ref<Model> model);
		Ref<Model> GetModel() { return m_Model; }

		std::vector<MeshEntry>& GetMeshes() { return m_CollapsedModelTree; }

		void Invalidate();

	private:
		Ref<Model> m_Model;
		std::vector<MeshEntry> m_CollapsedModelTree;
		Ref<SceneRenderer> m_SceneRenderer;
	};
}
