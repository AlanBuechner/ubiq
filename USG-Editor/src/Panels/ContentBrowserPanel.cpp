#include <Engine.h>
#include "ContentBrowserPanel.h"
#include <imgui/imgui.h>

#include <filesystem>

namespace Engine
{
	// get from project file
	static const std::filesystem::path s_AssetsDirectory = "Assets";

	ContentBrowserPanel::ContentBrowserPanel() :
		m_CurrentDirectory(s_AssetsDirectory)
	{
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
			ImGui::Button("<---", imageSize);
			if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			ImGui::Text("%s", "back");
			ImGui::NextColumn();
		}

		for (auto& p : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			auto& path = p.path();
			std::string fileName = path.filename().string();
			ImGui::Button(fileName.c_str(), imageSize);

			if (ImGui::BeginDragDropSource())
			{
				const wchar_t* itempath = path.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itempath, (wcslen(itempath) + 1) * sizeof(wchar_t));
				ImGui::EndDragDropSource();
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (p.is_directory())
					m_CurrentDirectory = path;

			}
			ImGui::Text(fileName.c_str());

			ImGui::NextColumn();
		}

		ImGui::Columns(1);
		//ImGui::SliderInt("Image Size", &m_ImageSize, 16, 512);

		ImGui::End();
	}


}