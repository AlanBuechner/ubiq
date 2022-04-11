#include <Engine.h>
#include "ContentBrowserPanel.h"
#include "../EditorLayer.h"
#include "EditorAssets.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <fstream>
#include <filesystem>

namespace Engine
{
	// get from project file
	static const fs::path s_AssetsDirectory = "Assets";

	ContentBrowserPanel::ContentBrowserPanel() :
		m_CurrentDirectory(s_AssetsDirectory)
	{
		Application::Get().GetAssetManager().GetAsset<Texture2D>(0);
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		fs::path dir = m_CurrentDirectory;
		std::vector<fs::path> m_Directorys;
		while (dir.has_parent_path())
			m_Directorys.push_back(dir = dir.parent_path());

		for (int i = m_Directorys.size()-1; i >= 0; i--)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0,0});
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {2,2});
			if (ImGui::Button(m_Directorys[i].filename().string().c_str()))
				m_CurrentDirectory = m_Directorys[i];
			FileDropTarget(m_Directorys[i]);
			ImGui::SameLine();
			ImGui::Text("/");
			ImGui::SameLine();
			ImGui::PopStyleVar(2);
		}

		ImGui::Text("%s", m_CurrentDirectory.filename().string().c_str());
		ImGui::SameLine(ImGui::GetWindowWidth() - 180);
		ImGui::PushItemWidth(100);
		ImGui::SliderInt("Image Size", &m_ImageSize, 16, 512);
		ImGui::PopItemWidth();

		ImGui::Separator();

		const uint32 windowWidth = (uint32)ImGui::GetContentRegionAvail().x;
		int padding = 10;
		int columnCount = windowWidth / (m_ImageSize + padding);

		if (columnCount < 1)
			columnCount = 1;
		
		ImGui::Columns(columnCount, 0, false);

		ImVec2 imageSize = { (float)m_ImageSize, (float)m_ImageSize };

		if (m_CurrentDirectory != s_AssetsDirectory)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton((ImTextureID)EditorAssets::s_BackIcon->GetRendererID(), imageSize, { 0,1 }, { 1,0 });
			ImGui::PopStyleColor();
			FileDropTarget(m_CurrentDirectory.parent_path());
			if(ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			ImGui::Text("%s", "Back");
			ImGui::NextColumn();
		}

		int itemID = 1;
		for (auto& p : fs::directory_iterator(m_CurrentDirectory))
		{
			auto& path = p.path();
			std::string filename = path.filename().string();

			if(path.extension().string() == ".meta")
				continue;

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushID(itemID);
			Ref<Texture2D> icon = GetFileIcon(p);
			if(icon)
				ImGui::ImageButton((ImTextureID)icon->GetRendererID(), imageSize, { 0,1 }, { 1,0 });
			ImGui::PopStyleColor();

			if (ImGui::BeginDragDropSource())
			{
				const wchar_t* itempath = path.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itempath, (wcslen(itempath) + 1) * sizeof(wchar_t));
				if(Texture2D::ValidExtention(path.extension().string()))
					ImGui::Image((ImTextureID)Application::Get().GetAssetManager().GetAsset<Texture2D>(path)->GetRendererID(), {50,50}, { 0,1 }, { 1,0 });
				ImGui::EndDragDropSource();
			}

			if (p.is_directory())
				FileDropTarget(path);

			if (ImGui::BeginPopupContextItem(("##" + std::to_string(itemID)).c_str()))
			{
				if (ImGui::MenuItem("Remove"))
					Application::Get().GetAssetManager().DeleteAsset(path);
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
					else
						system(path.string().c_str());
				}
			}

			ImGui::PopID();

			if (!m_OldFileName.empty() && m_OldFileName == filename)
			{
				ImGui::SetKeyboardFocusHere(0);
				if (ImGui::InputText("##ChangeFileName", &m_NewFileName, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)
					|| Input::GetKeyPressed(KeyCode::ENTER) || (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemHovered()))
				{
					if (m_NewFileName != m_OldFileName)
						Application::Get().GetAssetManager().RenameAsset(path, m_NewFileName);
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
					m_OldFileName = Application::Get().GetAssetManager().CreateFolder(m_CurrentDirectory / "New Folder").string();
					m_NewFileName = m_OldFileName;
				}
				
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}

		ImGui::End();
	}

	Ref<Texture2D> ContentBrowserPanel::GetFileIcon(fs::directory_entry file)
	{
		if (file.is_directory())
			return EditorAssets::s_FolderIcon;

		std::string ext = file.path().extension().string();

		if (ext == ".glsl")
			return EditorAssets::s_ShaderFileIcon;

		if (ext == ".ubiq")
			return EditorAssets::s_SceneFileIcon;

		if (Texture2D::ValidExtention(ext))
			return Application::Get().GetAssetManager().GetAsset<Texture2D>(file.path());//EditorAssets::s_ImageFileIcon;

		return EditorAssets::s_DefaultFileIcon;
	}

	void ContentBrowserPanel::FileDropTarget(fs::path path)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const auto droppath = (fs::path)(const wchar_t*)payload->Data;
				if (droppath != path)
					Application::Get().GetAssetManager().MoveAsset(droppath, path);
			}
			ImGui::EndDragDropTarget();
		}
	}

}
