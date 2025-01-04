#pragma once

#include <Engine.h>
#include <Engine/Math/Math.h>

#include "Editor/Panels/EditorPanel.h"

#include "Engine/Renderer/EditorCamera.h"
#include "Engine/Renderer/SceneRenderer.h"

#include "ProjectManager/Project.h"
#include "Engine/Core/GameBase.h"

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

		virtual void OnImGuiRender() override;
		virtual void OnUpdate() override;
		virtual void OnRender() override;

		void NewScene();
		void DefaultScene();
		void LoadScene(const fs::path& file);

		Engine::Ref<Engine::EditorCamera> GetEditorCamera() { return m_EditorCamera; }

		template<class T>
		Engine::Ref<T> GetPanel();

		bool IsPlaying() { return m_Playing; }

	private:
		// draw ui
		void DrawViewport();

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
		Engine::Ref<Engine::EditorCamera> m_EditorCamera;
		fs::path m_LoadedScene;
		bool m_Playing = false;
		Utils::Vector<Engine::Ref<EditorPanel>> m_Panels;

		// viewport
		Math::Vector2 m_ViewPortSize;
		Math::Vector2 m_ViewportBounds[2];

		// game
		Engine::GameBase* m_Game;

		// extra
		fs::path m_DropPath;
		fs::path m_EditorDirectory;
	};



	template<class T>
	Engine::Ref<T> Editor::EditorLayer::GetPanel()
	{
		for (uint32 i = 0; i < m_Panels.Count(); i++)
		{
			if (m_Panels[i]->GetClass().GetTypeID() == T::GetStaticClass().GetTypeID())
				return std::dynamic_pointer_cast<T>(m_Panels[i]);
		}
		return nullptr;
	}

}
