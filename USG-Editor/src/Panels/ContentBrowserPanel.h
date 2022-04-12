#pragma once
#include <filesystem>
#include <Engine.h>

#include <map>

namespace fs = std::filesystem;

namespace Engine
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();

		void SelectAsset(const fs::path& path) { SetDirectory(path.parent_path()); m_SelectedAsset = path; };
		void SetDirectory(const fs::path& path) { m_CurrentDirectory = path; }

	private:
		void DrawDirectory(const fs::path& path);

		Ref<Texture2D> GetFileIcon(const fs::directory_entry& file);

		void FileDragSorce(const fs::path& path);
		void FileDropTarget(const fs::path& path);

		void OpenAsset(const fs::path& path);

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
	};
}
