#include "PropertiesPanel.h"
#include "EditorLayer.h"
#include "EditorAssets.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <typeinfo>

namespace Engine
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

	bool PropertysPanel::DrawTextureControl(const std::string& label, Ref<Texture2D>& texture)
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
				if (Texture2D::ValidExtension(path.extension().string())) {
					texture = Application::Get().GetAssetManager().GetAsset<Texture2D>(path);
					changed = true;
				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::NextColumn();
		ImGui::Text(label.c_str());
		if (texture)
		{
			fs::path path = Application::Get().GetAssetManager().GetRelitiveAssetPath(texture->GetAssetID());
			if (ImGui::Button(path.string().c_str()))
				EditorLayer::Get()->GetContantBrowser().SelectAsset(path);
			
		}
		if (ImGui::Button("Clear")) {
			texture = Ref<Texture2D>();
			changed = true;
		}
		ImGui::Columns(1);
		ImGui::PopID();

		return changed;
	}

	bool PropertysPanel::DrawModelControl(const std::string& label, Ref<Model>& mesh)
	{
		bool changed = false;

		ImGui::Text(label.c_str());
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				fs::path path = (const wchar_t*)payload->Data;
				if (Model::ValidExtension(path.extension().string())) {
					mesh = Application::Get().GetAssetManager().GetAsset<Model>(path);
					changed = true;
				}
			}
			ImGui::EndDragDropTarget();
		}
		if (mesh)
		{
			fs::path path = Application::Get().GetAssetManager().GetRelitiveAssetPath(mesh->GetAssetID());
			if (ImGui::Button(path.string().c_str()))
				EditorLayer::Get()->GetContantBrowser().SelectAsset(path);
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear")) {
			mesh = Ref<Model>();
			changed = true;
		}

		return changed;
	}

	bool PropertysPanel::DrawMaterialControl(const std::string& label, Ref<Material>& mat)
	{
		bool changed = false;

		ImGui::Text(label.c_str());
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				fs::path path = (const wchar_t*)payload->Data;
				if (Material::ValidExtention(path.extension().string())) {
					mat = Application::Get().GetAssetManager().GetAsset<Material>(path);
					changed = true;
				}
			}
			ImGui::EndDragDropTarget();
		}
		if (mat)
		{
			fs::path path = Application::Get().GetAssetManager().GetRelitiveAssetPath(mat->GetAssetID());
			if (ImGui::Button(path.string().c_str()))
				EditorLayer::Get()->GetContantBrowser().SelectAsset(path);
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear")) {
			mat = Ref<Material>();
			changed = true;
		}

		return changed;
	}

	bool PropertysPanel::DrawPropertyControl(void* object, uint64 typeID, const Reflect::Property* prop)
	{
		bool changed = false;
		void* propLoc = prop ? (void*)((uint64)object + prop->GetOffset()) : object;
		if (!(prop && prop->HasFlag("HideInInspector")))
		{
			auto func = PropertysPanel::s_ExposePropertyFunctions.find(typeID);
			if (func != PropertysPanel::s_ExposePropertyFunctions.end())
				changed = func->second(propLoc, typeID, prop);
			else if (prop)
				CORE_WARN("Could not draw controll for prperty {0} of type {1}", prop->GetName(), prop->GetTypeID());
		}
		const Reflect::Class* propClass = Reflect::Registry::GetRegistry()->GetClass(typeID);
		if (propClass)
		{
			const std::vector<Reflect::Property>& props = propClass->GetProperties();
			for(const Reflect::Property& p : props)
				DrawPropertyControl(propLoc, p.GetTypeID(), &p);
		}
		return changed;
	}

	std::unordered_map<uint64, PropertysPanel::ExposePropertyFunc> PropertysPanel::s_ExposePropertyFunctions;

	Engine::PropertysPanel::AddExposePropertyFunc BDraw(
		typeid(bool).hash_code(),
		[](void* voidData, uint64 typeID, const Reflect::Property* prop) {
			bool* data = (bool*)voidData;
			return PropertysPanel::DrawBoolControl(prop->GetName(), *data);
		}
	);

	Engine::PropertysPanel::AddExposePropertyFunc FDraw(
		typeid(float).hash_code(),
		[](void* voidData, uint64 typeID, const Reflect::Property* prop) {
			float* data = (float*)voidData;
			return PropertysPanel::DrawFloatControl(prop->GetName(), *data);
		}
	);

	Engine::PropertysPanel::AddExposePropertyFunc V2Draw(
		typeid(Math::Vector2).hash_code(),
		[](void* voidData, uint64 typeID, const Reflect::Property* prop) {
			Math::Vector2* data = (Math::Vector2*)voidData;
			return PropertysPanel::DrawVec2Control(prop->GetName(), *data);
		}
	);

	Engine::PropertysPanel::AddExposePropertyFunc V3Draw(
		typeid(Math::Vector3).hash_code(),
		[](void* voidData, uint64 typeID, const Reflect::Property* prop) {
			Math::Vector3* data = (Math::Vector3*)voidData; 
			return PropertysPanel::DrawVec3Control(prop->GetName(), *data);
		}
	);

}
