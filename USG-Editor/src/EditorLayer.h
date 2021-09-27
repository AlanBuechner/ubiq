#pragma once

#include <Engine.h>
#include <glm/glm.hpp>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"

#include "Engine/Renderer/EditorCamera.h"

namespace Engine
{

	class EditorLayer : public Layer
	{
		static EditorLayer* s_Instance;

		enum class SceneState
		{
			Edit = 0,
			Play = 1,
		};

	public:
		EditorLayer();
		~EditorLayer() = default;

		static EditorLayer* Get() { return s_Instance; }

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate() override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;

		void LoadScene(const std::string& file);

	private:
		void NewScene();
		void OpenScene();
		void SaveSceneAs();

		bool OnKeyPressed(KeyPressedEvent& e);

		void DrawCustomGizmo();

		void UI_Toolbar();
		void UI_Viewport();

		void OnScenePlay();
		void OnSceneStop();


	private:
		const glm::vec4 m_GridColor = { 0.5f,0.5f,0.5f,1 };
		const float m_GridExtent = 40.0f;
		const int m_GridLines = 80;
		const float m_GridLineOffset = m_GridExtent * 2 / m_GridLines;
		LineMesh m_GridMesh;

		EditorCamera m_EditorCamera;

		Ref<Scene> m_ActiveScene;

		Entity m_CameraEntity;

		glm::vec2 m_ViewPortSize;
		Ref<FrameBuffer> m_FrameBuffer;

		glm::vec2 m_ViewportBounds[2];

		int m_GizmoType = -1;

		SceneHierarchyPanel m_HierarchyPanel;
		ContentBrowserPanel m_ContentPanel;

		Ref<Texture2D> m_PlayButton;
		Ref<Texture2D> m_StopButton;

		SceneState m_SceneState = SceneState::Edit;

		bool m_TransformingEntity = false;

		bool m_OpenScene = false;
		bool m_SaveScene = false;
	};
}
