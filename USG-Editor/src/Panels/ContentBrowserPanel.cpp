#include <Engine.h>
#include "ContentBrowserPanel.h"
#include "../EditorLayer.h"
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <fstream>
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

		int itemID = 0;
		for (auto& p : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			auto& path = p.path();
			std::string filename = path.filename().string();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
			ImGui::ImageButton((ImTextureID)GetFileIcon(p)->GetRendererID(), imageSize, { 0,1 }, { 1,0 });
			ImGui::PopStyleColor();

			if (ImGui::BeginDragDropSource())
			{
				const wchar_t* itempath = path.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itempath, (wcslen(itempath) + 1) * sizeof(wchar_t));
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginPopupContextItem(("##"+std::to_string(itemID)).c_str()))
			{
				if (ImGui::MenuItem("Remove"))
					std::filesystem::remove(path);
				ImGui::EndPopup();
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

			if (!m_OldFileName.empty() && m_OldFileName == filename)
			{
				if (ImGui::InputText("##CangeFileName", &m_NewFileName, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)
					|| Input::GetKeyPressed(KeyCode::ENTER))
				{
					std::filesystem::rename(path, m_CurrentDirectory/m_NewFileName.c_str());
					m_OldFileName.clear();
				}
				
			}
			else
			{
				ImGui::Text(filename.c_str());
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					m_OldFileName = filename;
					m_NewFileName = filename;
				}
			}

			ImGui::NextColumn();
			itemID++;
		}

		ImGui::Columns(1);

		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::BeginMenu("Add"))
			{
				if (ImGui::MenuItem("Folder"))
				{
					m_OldFileName = CreateNewFile("New Folder", "");
					m_NewFileName = m_OldFileName;
				}
				
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}

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

	std::string ContentBrowserPanel::CreateNewFile(const std::string& name, const std::string& ext)
	{
		std::string newName = name;
		if (ext.empty()) // create directory
		{
			uint32_t i = 1;
			while (!std::filesystem::create_directory(m_CurrentDirectory / newName))
				newName = name + " (" + std::to_string(i++) + ")";
		}
		else
		{
			// todo
		}

		return newName;
	}


}