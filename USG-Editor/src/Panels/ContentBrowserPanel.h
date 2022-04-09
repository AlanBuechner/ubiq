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

	private:
		Ref<Texture2D> GetFileIcon(fs::directory_entry file);

	private:
		fs::path m_CurrentDirectory;
		int m_ImageSize = 64;

		std::string m_OldFileName;
		std::string m_NewFileName;
	};
}
