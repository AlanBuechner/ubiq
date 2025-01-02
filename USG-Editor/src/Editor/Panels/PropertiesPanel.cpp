#include "PropertiesPanel.h"
#include "ContentBrowserPanel.h"
#include "Editor/EditorLayer.h"
#include "Editor/EditorAssets.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <typeinfo>

namespace Editor
{

	bool PropertysPanel::DrawBoolControl(const std::string& lable, bool& value, bool resetValue )
	{
		return ImGui::Checkbox(lable.c_str(), &value);
	}

	bool PropertysPanel::DrawFloatControl(const std::string& label, float& value, float resetValue, float columnWidth)
	{
		bool changed = false;

		ImGui::PushID(label.c_str());
		ImGui::Columns(2);

		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.4f, 0.4f, 0.4f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.4f, 0.4f, 0.4f, 1.0f });
		if (ImGui::Button("V", buttonSize)) {
			value = resetValue;
			changed = true;
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &value, 0.01f, 0, 0, "%.2f"))
			changed = true;
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();

		ImGui::Spacing();

		return changed;
	}

	bool PropertysPanel::DrawFloatSlider(const std::string& label, float& value, float min, float max, float resetValue /*= 0.0f*/, float columnWidth /*= 100.0f*/)
	{
		bool changed = false;

		ImGui::PushID(label.c_str());
		ImGui::Columns(2);

		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.4f, 0.4f, 0.4f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.4f, 0.4f, 0.4f, 1.0f });
		if (ImGui::Button("V", buttonSize)) {
			value = resetValue;
			changed = true;
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::SliderFloat("##X", &value, min, max, "%.2f"))
			changed = true;
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();

		ImGui::Spacing();

		return changed;
	}

	bool PropertysPanel::DrawVec2Control(const std::string& label, Math::Vector2& values, float resetValue, float columnWidth)
	{
		bool changed = false;

		ImGui::PushID(label.c_str());
		ImGui::Columns(2);

		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		if (ImGui::Button("X", buttonSize)) {
			values.x = resetValue;
			changed = true;
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
			changed = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
		if (ImGui::Button("Y", buttonSize)) {
			values.y = resetValue;
			changed = true;
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
			changed = true;
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();

		ImGui::Spacing();

		return changed;
	}

	bool PropertysPanel::DrawVec3Control(const std::string& label, Math::Vector3& values, float resetValue, float columnWidth)
	{
		bool changed = false;

		ImGui::PushID(label.c_str());
		ImGui::Columns(2);

		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		if (ImGui::Button("X", buttonSize)) {
			values.x = resetValue;
			changed = true;
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
			changed = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
		if (ImGui::Button("Y", buttonSize)){
			values.y = resetValue;
			changed = true;
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
			changed = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		if (ImGui::Button("Z", buttonSize)){
			values.z = resetValue;
			changed = true;
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f"))
			changed = true;
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();

		ImGui::Spacing();

		return changed;
	}

	bool PropertysPanel::DrawColorControl(const std::string& label, Math::Vector3& values)
	{

		bool changed = false;

		ImGui::PushID(label.c_str());
		ImGui::Columns(2);

		ImGui::SetColumnWidth(0, 100);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::SameLine();
		if (ImGui::ColorEdit3("##color", (float*)&values))
			changed = true;

		ImGui::Columns(1);
		ImGui::PopID();

		ImGui::Spacing();

		return changed;
	}

	bool PropertysPanel::DrawTextureControl(const std::string& label, Engine::Ref<Engine::Texture2D>& texture)
	{
		bool changed = false;

		ImGui::PushID(label.c_str());
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 100);
		ImGui::Image((ImTextureID)(texture ? texture : EditorAssets::s_NoTextureIcon)->GetSRVDescriptor()->GetGPUHandlePointer(), { 70,70 });
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				fs::path path = (const wchar_t*)payload->Data;
				if (Engine::Texture2D::ValidExtension(path.extension().string())) {
					texture = Engine::Application::Get().GetAssetManager().GetAsset<Engine::Texture2D>(path);
					changed = true;
				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::NextColumn();
		ImGui::Text(label.c_str());
		if (texture)
		{
			fs::path path = Engine::Application::Get().GetAssetManager().GetRelitiveAssetPath(texture->GetAssetID());
			if (ImGui::Button(path.string().c_str()))
				EditorLayer::Get()->GetPanel<ContentBrowserPanel>()->SelectAsset(path);
			
		}
		if (ImGui::Button("Clear")) {
			texture = Engine::Ref<Engine::Texture2D>();
			changed = true;
		}
		ImGui::Columns(1);
		ImGui::PopID();

		return changed;
	}

	bool PropertysPanel::DrawMaterialControl(const std::string& label, Engine::Ref<Engine::Material>& mat)
	{
		bool changed = false;

		ImGui::Text(label.c_str());
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				fs::path path = (const wchar_t*)payload->Data;
				if (Engine::Material::ValidExtention(path.extension().string())) {
					mat = Engine::Application::Get().GetAssetManager().GetAsset<Engine::Material>(path);
					changed = true;
				}
			}
			ImGui::EndDragDropTarget();
		}
		if (mat)
		{
			Engine::UUID assetID = mat->GetAssetID();
			if (assetID == 0)
			{
				ImGui::Button("Material not associated with file");
			}
			else
			{
				fs::path path = Engine::Application::Get().GetAssetManager().GetRelitiveAssetPath(mat->GetAssetID());
				if (ImGui::Button(path.string().c_str()))
					EditorLayer::Get()->GetPanel<ContentBrowserPanel>()->SelectAsset(path);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear")) {
			mat = nullptr;
			changed = true;
		}

		return changed;
	}

	bool PropertysPanel::DrawPropertyControl(void* object, uint64 typeID, const Reflect::Property* prop)
	{
		bool changed = false;
		void* propLoc = prop ? (void*)((uint64)object + prop->GetOffset()) : object;
		auto func = PropertysPanel::s_ExposePropertyFunctions.find(typeID);
		if (func != PropertysPanel::s_ExposePropertyFunctions.end())
			changed = func->second(propLoc, object, typeID, prop);
		else if (prop)
			CORE_WARN("Could not draw controll for prperty {0} of type {1}", prop->GetName(), prop->GetTypeID());
		const Reflect::Class* propClass = Reflect::Registry::GetRegistry()->GetClass(typeID);
		if (propClass)
		{
			const std::vector<Reflect::Property>& props = propClass->GetProperties();
			for (const Reflect::Property& p : props)
			{
				if(!p.HasFlag("HideInInspector"))
					DrawPropertyControl(propLoc, p.GetTypeID(), &p);
			}
		}
		return changed;
	}

	std::unordered_map<uint64, PropertysPanel::ExposePropertyFunc> PropertysPanel::s_ExposePropertyFunctions;

	ADD_EXPOSE_PROP_FUNC(bool) {
		bool* data = (bool*)voidData;
		if (prop && prop->HasAttribute("set"))
		{
			bool val = *data;
			bool changed = PropertysPanel::DrawBoolControl(prop->GetName(), val);
			if (changed)
				prop->GetClass().GetFunction(prop->GetAttribute("set").GetValue()).Invoke(object, { &val });
			return changed;
		}
		else
			return PropertysPanel::DrawBoolControl(prop->GetName(), *data);
	});

	ADD_EXPOSE_PROP_FUNC(float) {
		float* data = (float*)voidData;
		float vec = *data;

		if (prop && prop->HasFlag("Degrees")) vec = Math::Degrees(vec);
		bool changed = PropertysPanel::DrawFloatControl(prop->GetName(), vec);
		if (prop && prop->HasFlag("Degrees")) vec = Math::Radians(vec);

		if (changed)
		{
			if (prop && prop->HasAttribute("set"))
				prop->GetClass().GetFunction(prop->GetAttribute("set").GetValue()).Invoke(object, { &vec });
			else
				*data = vec;
		}
		return changed;
	});

	ADD_EXPOSE_PROP_FUNC(Math::Vector2) {
		Math::Vector2* data = (Math::Vector2*)voidData;
		Math::Vector2 vec = *data;

		if (prop && prop->HasFlag("Degrees")) vec = Math::Degrees(vec);
		bool changed = PropertysPanel::DrawVec2Control(prop->GetName(), vec);
		if (prop && prop->HasFlag("Degrees")) vec = Math::Radians(vec);

		if (changed)
		{
			if (prop && prop->HasAttribute("set"))
				prop->GetClass().GetFunction(prop->GetAttribute("set").GetValue()).Invoke(object, { &vec });
			else
				*data = vec;
		}
		return changed;
	});

	ADD_EXPOSE_PROP_FUNC(Math::Vector3) {
		Math::Vector3* data = (Math::Vector3*)voidData; 
		Math::Vector3 vec = *data;

		if (prop && prop->HasFlag("Degrees")) vec = Math::Degrees(vec);
		bool changed = PropertysPanel::DrawVec3Control(prop->GetName(), vec);
		if (prop && prop->HasFlag("Degrees")) vec = Math::Radians(vec);

		if (changed)
		{
			if (prop && prop->HasAttribute("set"))
				prop->GetClass().GetFunction(prop->GetAttribute("set").GetValue()).Invoke(object, { &vec });
			else
				*data = vec;
		}
		return changed;
	});

	ADD_EXPOSE_PROP_FUNC(Engine::Ref<Engine::Texture2D>) {
		Engine::Ref<Engine::Texture2D>* data = (Engine::Ref<Engine::Texture2D>*)voidData;
		if (prop && prop->HasAttribute("set"))
		{
			Engine::Ref<Engine::Texture2D> texture = *data;
			bool changed = PropertysPanel::DrawTextureControl(prop->GetName(), texture);
			if(changed)
				prop->GetClass().GetFunction(prop->GetAttribute("set").GetValue()).Invoke(object, { &texture });
			return changed;
		}
		else
			return PropertysPanel::DrawTextureControl(prop->GetName(), *data);
	});

}
