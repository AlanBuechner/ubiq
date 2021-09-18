#include <Engine.h>
#include "ContentBrowserPanel.h"
#include "../EditorLayer.h"
#include <imgui/imgui.h>

#include <filesystem>

namespace Engine
{
	// get from project file
	static const std::filesystem::path s_AssetsDirectory = "Assets";

	ContentBrowserPanel::ContentBrowserPanel() :
		m_CurrentDirectory(s_AssetsDirectory)
	{
		m_DefaultFileIcon = Texture2D::Create("Resources/DefaultFileIcon.png");
		m_ShaderFileIcon = Texture2D::Create("Resources/ShaderFileIcon.png");
		m_SceneFileIcon = Texture2D::Create("Resources/SceneFileIcon.png");
		m_ImageFileIcon = Texture2D::Create("Resources/ImageFileIcon.png");
		m_FolderIcon = Texture2D::Create("Resources/FolderIcon.png");
		m_BackIcon = Texture2D::Create("Resources/BackIcon.png");
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		ImGui::Text("%s", m_CurrentDirectory.string().c_str());
		
		const int windowWidth = ImGui::GetContentRegionAvail().x;
		int padding = 10;
		int columnCount = windowWidth / (m_ImageSize + padding);

		if (columnCount < 1)
			columnCount = 1;
		
		ImGui::Columns(columnCount, 0, false);

		ImVec2 imageSize = { (float)m_ImageSize, (float)m_ImageSize };

		if (m_CurrentDirectory != s_AssetsDirectory)
		{
			ImGui::ImageButton((ImTextureID)m_BackIcon->GetRendererID(), imageSize, { 0,1 }, { 1,0 });
			if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			ImGui::Text("%s", "Back");
			ImGui::NextColumn();
		}

		for (auto& p : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			auto& path = p.path();
			std::string fileName = path.filename().string();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
			ImGui::ImageButton((ImTextureID)GetFileIcon(p)->GetRendererID(), imageSize, { 0,1 }, { 1,0 });
			ImGui::PopStyleColor();

			if (ImGui::BeginDragDropSource())
			{
				const wchar_t* itempath = path.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itempath, (wcslen(itempath) + 1) * sizeof(wchar_t));
				ImGui::EndDragDropSource();
			}

			if (ImGui::IsItemHovered())
			{
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					if (p.is_directory())
						m_CurrentDirectory = path;
					else if (path.extension().string() == ".ubiq")
						EditorLayer::Get()->LoadScene(path.string());

				}
			}

			ImGui::Text(fileName.c_str());

			ImGui::NextColumn();
		}

		ImGui::Columns(1);

		//ImGui::SliderInt("Image Size", &m_ImageSize, 16, 512);

		ImGui::End();
	}

	Ref<Texture2D> ContentBrowserPanel::GetFileIcon(std::filesystem::directory_entry file)
	{
		if (file.is_directory())
			return m_FolderIcon;

		std::string ext = file.path().extension().string();

		if (ext == ".glsl")
			return m_ShaderFileIcon;

		if (ext == ".ubiq")
			return m_SceneFileIcon;

		if (ext == ".png" || ext == ".jpg" || ext == "jpeg")
			return m_ImageFileIcon;

		return m_DefaultFileIcon;
	}


}