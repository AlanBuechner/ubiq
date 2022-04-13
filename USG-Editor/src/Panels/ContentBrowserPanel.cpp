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
		m_RootDirectory(s_AssetsDirectory), m_CurrentDirectory(s_AssetsDirectory)
	{
		m_OpenFolders[m_RootDirectory] = true;
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			m_SelectedAsset = "";

		ImGui::Begin("Content Browser");
		{
			const ImGuiWindow* window = ImGui::GetCurrentWindow();
			const ImRect titleBarRect = window->TitleBarRect();
			ImGui::PushClipRect(titleBarRect.Min, titleBarRect.Max, false);
			ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 30, 0.0f));
			ImGui::Button("...");
			ImRect buttonRect = { ImGui::GetItemRectMin(), ImGui::GetItemRectMax() };
			ImVec2 mousePos = ImGui::GetMousePos();
			bool ishovered = mousePos.x > buttonRect.Min.x && mousePos.x < buttonRect.Max.x&& mousePos.y > buttonRect.Min.y && mousePos.y < buttonRect.Max.y;

			if (ishovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				ImGui::OpenPopup("settings");
			
			if (ImGui::BeginPopup("settings", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
			{
				if (ImGui::RadioButton("Show Tree", m_DrawTreeView))
					m_DrawTreeView = !m_DrawTreeView;
				ImGui::EndPopup();
			}
			ImGui::PopClipRect();
		}

		float height = ImGui::GetContentRegionAvail().y;

		float dividLoc = m_DrawTreeView ? m_DivideLoc : 10;
		ImGui::BeginChild("HierarchyView", { dividLoc, height });
		if (dividLoc > 10 && m_DrawTreeView)
			DrawDirectory(m_RootDirectory);
		
		ImGui::EndChild();

		ImGui::SameLine();
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine(dividLoc + 5);
		ImGui::InvisibleButton("vsplitter", ImVec2(20.0f, height));
		if (m_DrawTreeView)
		{
			if (ImGui::IsItemActive())
				m_DivideLoc += ImGui::GetIO().MouseDelta.x;
			m_DivideLoc = Math::Clamp(m_DivideLoc, 10, ImGui::GetWindowWidth() - 200);
			if (ImGui::IsItemHovered())
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		}
		ImGui::SameLine();

		ImGui::BeginChild("FolderView", { ImGui::GetContentRegionAvail().x, height });
		fs::path dir = m_CurrentDirectory;
		std::vector<fs::path> directorys;
		while (dir.has_parent_path())
			directorys.push_back(dir = dir.parent_path());

		for (int i = (int)directorys.size() - 1; i >= 0; i--)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,0 });
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 2,2 });
			if (ImGui::Button(directorys[i].filename().string().c_str()))
				m_CurrentDirectory = directorys[i];
			FileDropTarget(directorys[i]);
			ImGui::SameLine();
			ImGui::Text("/");
			ImGui::SameLine();
			ImGui::PopStyleVar(2);
		}

		ImGui::Text("%s", m_CurrentDirectory.filename().string().c_str());
		ImGui::SameLine(ImGui::GetWindowWidth() - 200);
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
			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			ImGui::Text("%s", "Back");
			ImGui::NextColumn();
		}

		int itemID = 1;
		for (auto& p : fs::directory_iterator(m_CurrentDirectory))
		{
			auto& path = p.path();
			std::string filename = path.filename().string();

			if (path.extension().string() == ".meta")
				continue;

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushID(itemID);
			Ref<Texture2D> icon = GetFileIcon(p);
			if (icon)
				ImGui::ImageButton((ImTextureID)icon->GetRendererID(), imageSize, { 0,1 }, { 1,0 });
			ImGui::PopStyleColor();

			if (path == m_SelectedAsset)
				ImGui::GetForegroundDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255,255,0,255), 0, 0, 3);

			FileDragSorce(path);

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
						Application::Get().GetAssetManager().OpenAsset(path);

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

		ImGui::EndChild();

		ImGui::End();
	}

	void ContentBrowserPanel::SelectAsset(fs::path path) 
	{
		if (fs::exists(path))
		{
			m_CurrentDirectory = path.parent_path(); 
			m_SelectedAsset = path; 
			while (path.has_parent_path())
			{
				path = path.parent_path();
				m_OpenFolders[path] = true;
			}
		}
	}

	void ContentBrowserPanel::SetDirectory(const fs::path& path) 
	{ 
		if(fs::exists(path))
			m_CurrentDirectory = path; 
	}

	void ContentBrowserPanel::DrawDirectory(const fs::path& path)
	{
		auto p = fs::directory_entry(path);
		std::string filename = path.filename().string();

		bool subPathOfCurrentDir = m_CurrentDirectory.string().rfind(path.string(), 0) == 0;

		if (path.extension().string() == ".meta")
			return;

		ImGui::SetNextItemOpen(m_OpenFolders[path]);
		if (!p.is_directory())
		{
			ImGui::Image((ImTextureID)GetFileIcon(p)->GetRendererID(), { 20,20 }, { 0,1 }, { 1,0 });
			ImGui::SameLine();
		}
		ImGuiTreeNodeFlags flags = (path == m_CurrentDirectory ? ImGuiTreeNodeFlags_Selected : 0) |
			(p.is_directory() || subPathOfCurrentDir ? ImGuiTreeNodeFlags_OpenOnArrow : ImGuiTreeNodeFlags_Leaf) |
			ImGuiTreeNodeFlags_SpanAvailWidth;
		bool open = ImGui::TreeNodeEx(path.c_str(), flags, path.filename().string().c_str());
		if (path == m_SelectedAsset)
			ImGui::GetForegroundDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 0, 255), 0, 0, 3);

		FileDragSorce(path);

		if (p.is_directory())
		{
			FileDropTarget(path);

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				m_CurrentDirectory = path;

			m_OpenFolders[path] = open;
			if (open)
			{
				for (auto& sub : fs::directory_iterator(path))
					DrawDirectory(sub.path());
			}
		}
		else
		{
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				Application::Get().GetAssetManager().OpenAsset(path);
		}

		if (open)
			ImGui::TreePop();

	}

	Ref<Texture2D> ContentBrowserPanel::GetFileIcon(const fs::directory_entry& file)
	{
		if (file.is_directory())
			return EditorAssets::s_FolderIcon;

		std::string ext = file.path().extension().string();

		if (ext == ".glsl")
			return EditorAssets::s_ShaderFileIcon;

		if (ext == ".ubiq")
			return EditorAssets::s_SceneFileIcon;

		if (Texture2D::ValidExtention(ext))
			return Application::Get().GetAssetManager().GetAsset<Texture2D>(file.path());

		return EditorAssets::s_DefaultFileIcon;
	}

	void ContentBrowserPanel::FileDragSorce(const fs::path& path)
	{
		if (ImGui::BeginDragDropSource())
		{
			const wchar_t* itempath = path.c_str();
			ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itempath, (wcslen(itempath) + 1) * sizeof(wchar_t));
			ImGui::Image((ImTextureID)GetFileIcon(fs::directory_entry(path))->GetRendererID(), { 50,50 }, { 0,1 }, { 1,0 });
			ImGui::Text(path.filename().string().c_str());
			ImGui::EndDragDropSource();
		}
	}

	void ContentBrowserPanel::FileDropTarget(const fs::path& path)
	{
		if (ImGui::BeginDragDropTarget())
		{
			CORE_INFO("DragDrop Target");
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const auto droppath = (fs::path)(const wchar_t*)payload->Data;
				if (droppath.string().rfind(path.string(), 0) != 0)
					Application::Get().GetAssetManager().MoveAsset(droppath, path);
			}
			ImGui::EndDragDropTarget();
		}
	}

}
