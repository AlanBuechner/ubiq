#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Model.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Core/Scene/Components.h"

namespace Engine
{
	CLASS(GROUP = Component) StaticModelRendererComponent : public Component
	{
	public:
		REFLECTED_BODY(Engine::StaticModelRendererComponent);

	public:

		struct MeshEntry
		{
			MeshEntry(std::string name, Ref<Mesh> mesh) :
				m_Name(name), m_Mesh(mesh)
			{}

			std::string m_Name;
			Ref<Mesh> m_Mesh = nullptr;
			Ref<Material> m_Material = nullptr;
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
		PROPERTY() Ref<Model> m_Model;
		std::vector<MeshEntry> m_CollapsedModelTree;
		Ref<SceneRenderer> m_SceneRenderer;
	};
}
