#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Core/Scene/Components.h"
#include "Renderer/SceneRenderer/SceneRenderer.h"

#include "Renderer/Model.h"

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
			SceneRenderer::ObjectControlBlockRef m_Object = nullptr;
		};

	public:
		virtual void OnComponentRemoved() override;

		virtual void OnTransformChange(const Math::Mat4& transform) override;

		void SetModel(Engine::Ref<Model> model);
		Engine::Ref<Model> GetModel() { return m_Model; }

		Utils::Vector<MeshEntry>& GetMeshes() { return m_CollapsedModelTree; }

		void Invalidate();

	protected:
		virtual Utils::Vector<Engine::AABB> GenVolumes() override;

	private:
		PROPERTY(HideInInspector) Engine::Ref<Model> m_Model;
		Utils::Vector<MeshEntry> m_CollapsedModelTree;
		Engine::Ref<Engine::SceneRenderer> m_SceneRenderer;
	};
}
