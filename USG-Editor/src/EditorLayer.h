#pragma once

#include <Engine.h>
#include <glm/glm.hpp>

#include "Panels/SceneHierarchyPanel.h"
#include "Engine/Renderer/EditorCamera.h"

namespace Engine
{

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate() override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;

	private:

		bool OnKeyPressed(KeyPressedEvent& e);

		void NewScene();
		void OpenScene();
		void SaveSceneAs();

	private:

		EditorCamera m_EditorCamera;

		Ref<Scene> m_ActiveScene;

		Ref<Texture2D> m_LogoTexture;
		Ref<SubTexture2D> m_Texture;

		Entity m_CameraEntity;

		glm::vec2 m_ViewPortSize;
		Ref<FrameBuffer> m_FrameBuffer;

		int m_GizmoType = -1;

		SceneHierarchyPanel m_HierarchyPanel;

	};
}