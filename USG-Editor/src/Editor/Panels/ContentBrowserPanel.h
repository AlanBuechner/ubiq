#pragma once
#include <Engine.h>
#include "EditorPanel.h"


#include <map>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace Editor
{
	CLASS(GROUP == EditorPanel) ContentBrowserPanel : public EditorPanel
	{
	public:
		REFLECTED_BODY(Editor::ContentBrowserPanel)

	public:
		ContentBrowserPanel();

		virtual void OnUpdate() override;
		virtual void OnImGuiRender() override;

		void SelectAsset(fs::path path);
		void SetDirectory(const fs::path& path);
		bool IsActive() { return m_Active; }
		void SetActive(bool active) { m_Active = active; }

	private:
		void DrawDirectory(const fs::path& path);

		Engine::Ref<Engine::Texture2D> GetFileIcon(const fs::directory_entry& file);

		void FileDragSorce(const fs::path& path);
		void FileDropTarget(const fs::path& path);

	private:
		fs::path m_CurrentDirectory;
		fs::path m_RootDirectory;
		fs::path m_SelectedAsset;
		int m_ImageSize = 64;

		std::string m_OldFileName;
		std::string m_NewFileName;

		std::map<fs::path, bool> m_OpenFolders;

		float m_DivideLoc = 300.0f;

		bool m_DrawTreeView = true;
		bool m_Active = false;
	};
}
