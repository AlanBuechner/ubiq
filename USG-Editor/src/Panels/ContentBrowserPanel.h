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

	private:
		std::filesystem::path m_CurrentDirectory;
		int m_ImageSize = 64;

		Ref<Texture2D> m_DefaultFileIcon;
		Ref<Texture2D> m_ShaderFileIcon;
		Ref<Texture2D> m_SceneFileIcon;
		Ref<Texture2D> m_ImageFileIcon;
		Ref<Texture2D> m_FolderIcon;
		Ref<Texture2D> m_BackIcon;
	};
}