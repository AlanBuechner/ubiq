#pragma once
#include <filesystem>
#include <Engine.h>

namespace Engine
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();

	private:
		Ref<Texture2D> GetFileIcon(std::filesystem::directory_entry file);

		std::string CreateNewFile(const std::string& name, const std::string& ext);
		std::string ChangeFileLocation(const std::filesystem::path& src, const std::filesystem::path& dest);

	private:
		std::filesystem::path m_CurrentDirectory;
		int m_ImageSize = 64;

		std::string m_OldFileName;
		std::string m_NewFileName;

		Ref<Texture2D> m_DefaultFileIcon;
		Ref<Texture2D> m_ShaderFileIcon;
		Ref<Texture2D> m_SceneFileIcon;
		Ref<Texture2D> m_ImageFileIcon;
		Ref<Texture2D> m_FolderIcon;
		Ref<Texture2D> m_BackIcon;
	};
}