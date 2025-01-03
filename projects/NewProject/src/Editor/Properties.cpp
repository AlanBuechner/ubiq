#include "Properties.h"

#if defined(EDITOR)
#include "Editor/Panels/ContentBrowserPanel.h"
#include "Editor/Panels/PropertiesPanel.h"
#include "Editor/EditorLayer.h"

#include "imgui.h"
#include "imgui_internal.h"
#endif

namespace Game
{

#if defined(EDITOR)


	bool DrawModelControl(const std::string& label, Engine::Ref<Model>& mesh)
	{
		bool changed = false;

		ImGui::Text(label.c_str());
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				fs::path path = (const wchar_t*)payload->Data;
				if (Model::ValidExtension(path.extension().string())) {
					mesh = Engine::Application::Get().GetAssetManager().GetAsset<Model>(path);
					changed = true;
				}
			}
			ImGui::EndDragDropTarget();
		}
		if (mesh)
		{
			fs::path path = Engine::Application::Get().GetAssetManager().GetRelitiveAssetPath(mesh->GetAssetID());
			if (ImGui::Button(path.string().c_str()))
				Editor::EditorLayer::Get()->GetPanel<Editor::ContentBrowserPanel>()->SelectAsset(path);
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear")) {
			mesh = Engine::Ref<Model>();
			changed = true;
		}

		return changed;
	}

#endif
}

