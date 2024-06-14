#pragma once

#include "Engine.h"

namespace Editor
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Engine::Ref<Engine::Scene>& context);

		void SetContext(const Engine::Ref<Engine::Scene>& context);

		void OnImGuiRender();

		Engine::Entity GetSelectedEntity() const { return m_Selected; };
		void SelectEntity(Engine::Entity e);

	private:
		void CreateNewEntity(Engine::Entity parent = Engine::Entity());

		void DrawEntityNode(Engine::Entity entity);
		void DrawComponents(Engine::Entity entity);
	private:
		Engine::Ref<Engine::Scene> m_Context;
		Engine::Entity m_Selected;
	};
}
