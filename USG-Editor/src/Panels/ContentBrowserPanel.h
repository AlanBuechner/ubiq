#pragma once
#include <filesystem>
#include <Engine.h>

namespace fs = std::filesystem;

namespace Engine
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();

		void SetDirectory(fs::path& path) { m_CurrentDirectory = path; }

	private:
		void DrawDirectory(fs::path path);

		Ref<Texture2D> GetFileIcon(fs::directory_entry file);

		void FileDropTarget(fs::path path);

	private:
		fs::path m_CurrentDirectory;
		fs::path m_RootDirectory;
		int m_ImageSize = 64;

		std::string m_OldFileName;
		std::string m_NewFileName;
	};
}
