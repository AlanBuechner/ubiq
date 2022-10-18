#pragma once
#include "Engine/Core/Core.h"
#include "Material.h"
#include "Mesh.h"
#include "SceneRenderer.h"
#include "Engine/Core/Scene/Components.h"

namespace Engine
{
	class MeshRendererComponent : public Component
	{
	public:

		MeshRendererComponent() = default;
		MeshRendererComponent(MeshRendererComponent&& other);
		MeshRendererComponent& MeshRendererComponent::operator=(MeshRendererComponent&& other);
		~MeshRendererComponent();

		virtual void OnComponentAdded() override;

		void SetMesh(Ref<Mesh> mesh) { m_Mesh = mesh; Invalidate(); }
		Ref<Mesh> GetMesh() { return m_Mesh; }

		void SetMaterial(Ref<Material> mat) { m_Mat = mat; Invalidate(); }
		Ref<Material> GetMaterial() { return m_Mat; }

		void Invalidate();

	private:
		void ObjectMovedCallback(const Math::Mat4& transform);

	private:
		Ref<Mesh> m_Mesh;
		Ref<Material> m_Mat;
		SceneRenderer::ObjectControlBlockRef m_Object = nullptr;
		Ref<SceneRenderer> m_SceneRenderer;
	};
}
