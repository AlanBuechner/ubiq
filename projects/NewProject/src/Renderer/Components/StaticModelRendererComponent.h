#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Model.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Core/Scene/Components.h"

namespace Game
{
	CLASS(GROUP = Component) StaticModelRendererComponent : public Engine::Component
	{
	public:
		REFLECTED_BODY(Game::StaticModelRendererComponent);

	public:

		struct MeshEntry
		{
			MeshEntry(std::string name, Engine::Ref<Engine::Mesh> mesh) :
				m_Name(name), m_Mesh(mesh)
			{}

			std::string m_Name;
			Engine::Ref<Engine::Mesh> m_Mesh = nullptr;
			Engine::Ref<Engine::Material> m_Material = nullptr;
			Engine::SceneRenderer::ObjectControlBlockRef m_Object = nullptr;
		};

	public:
		virtual void OnComponentRemoved() override;

		virtual void OnTransformChange(const Math::Mat4& transform) override;

		void SetModel(Engine::Ref<Engine::Model> model);
		Engine::Ref<Engine::Model> GetModel() { return m_Model; }

		std::vector<MeshEntry>& GetMeshes() { return m_CollapsedModelTree; }

		void Invalidate();

	private:
		PROPERTY(HideInInspector) Engine::Ref<Engine::Model> m_Model;
		std::vector<MeshEntry> m_CollapsedModelTree;
		Engine::Ref<Engine::SceneRenderer> m_SceneRenderer;
	};
}
