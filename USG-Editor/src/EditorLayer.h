#pragma once

#include <Engine.h>
#include <Engine/Math/Math.h>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"

#include "Engine/Renderer/EditorCamera.h"
#include "Engine/Renderer/SceneRenderer.h"

#include "ProjectManager/Project.h"
#include "Engine/Core/GameBase.h"

namespace Engine
{

	class EditorLayer : public Layer
	{
		static EditorLayer* s_Instance;

	public:
		EditorLayer();
		~EditorLayer() = default;

		static EditorLayer* Get() { return s_Instance; }

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate() override;
		virtual void OnRender() override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event* event) override;

		void NewScene();
		void LoadScene(const std::string& file);

		ContentBrowserPanel& GetContantBrowser() { return m_ContentPanel; }

	private:
		void OpenScene();
		void SaveScene();
		void SaveSceneAs();

		bool OnKeyPressed(KeyPressedEvent* e);

		void DrawCustomGizmo();

		void UI_Viewport();

		Entity GetEntityAtMousePosition(bool& inWindow);

	private:
		const Math::Vector4 m_GridColor = { 0.5f,0.5f,0.5f,1 };
		const float m_GridExtent = 40.0f;
		const uint32 m_GridLines = 80;
		const float m_GridLineOffset = m_GridExtent * 2 / m_GridLines;
		LineMesh m_GridMesh;

		Ref<EditorCamera> m_EditorCamera;

		fs::path m_LoadedScene;

		Entity m_CameraEntity;

		Math::Vector2 m_ViewPortSize;

		Math::Vector2 m_ViewportBounds[2];

		int m_GizmoType = -1;

		SceneHierarchyPanel m_HierarchyPanel;
		ContentBrowserPanel m_ContentPanel;

		GameBase* m_Game;

		fs::path m_DropPath;
		fs::path m_EditorDirectory;
	};
}
