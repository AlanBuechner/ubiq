#pragma once

#include <Engine.h>
#include <Engine/Math/Math.h>

#include "Editor/EditorCamera.h"
#include "Engine/Renderer/SceneRenderer.h"

#include "ProjectManager/Project.h"
#include "Engine/Core/GameBase.h"

#include "GUI/GUI.h"

namespace Editor
{

	class EditorLayer : public Engine::Layer
	{
		static EditorLayer* s_Instance;

	public:
		EditorLayer();
		~EditorLayer();

		static EditorLayer* Get() { return s_Instance; }

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate() override;
		virtual void OnRender() override;

		void NewScene();
		void DefaultScene();
		void LoadScene(const fs::path& file);

		Engine::Ref<Editor::EditorCamera> GetEditorCamera() { return m_EditorCamera; }

		bool IsPlaying() { return m_Playing; }

	private:
		bool GetMousePositionInViewport(Math::Vector2& pos);

		// dialogs
		void OpenSceneDialog();
		void SaveSceneDialog();
		void SaveSceneAsDialog();

		// events
		virtual void OnEvent(Engine::Event* event) override;
		bool OnKeyPressed(Engine::KeyPressedEvent* e);

	private:
		// editor
		Engine::Ref<Editor::EditorCamera> m_EditorCamera;
		fs::path m_LoadedScene;
		bool m_Playing = false;

		// viewport
		Math::Vector2 m_ViewPortSize;
		Math::Vector2 m_ViewportBounds[2];

		// game
		Engine::GameBase* m_Game;

		// extra
		fs::path m_DropPath;
		fs::path m_EditorDirectory;

		GUI::Context m_Context;
	};

}
