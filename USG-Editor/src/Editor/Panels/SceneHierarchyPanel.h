#pragma once

#include "Engine.h"
#include "EditorPanel.h"

namespace Editor
{
	CLASS(GROUP = EditorPanel) SceneHierarchyPanel : public EditorPanel
	{
	public:
		REFLECTED_BODY(Editor::SceneHierarchyPanel);

	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(Engine::Ref<Engine::Scene> context);

		virtual void OnSceneChange(Engine::Ref<Engine::Scene> context) override;
		virtual void OnScreenClick(Math::Vector2 pos) override;

		virtual void OnUpdate() override {};
		virtual void OnImGuiRender() override;
		virtual void OnDrawGizmos() override;

		virtual void OnEvent(Engine::Event* e) override;

		Engine::Entity GetSelectedEntity() const { return m_Selected; };
		void SelectEntity(Engine::Entity e);

	private:

		bool OnKeyPressed(Engine::KeyPressedEvent* e);

		void CreateNewEntity(Engine::Entity parent = Engine::Entity());

		void DrawEntityNode(Engine::Entity entity);
		void DrawComponents(Engine::Entity entity);

	private:
		Engine::Ref<Engine::Scene> m_Context;
		Engine::Entity m_Selected;

		int m_GizmoType = -1;
	};
}
