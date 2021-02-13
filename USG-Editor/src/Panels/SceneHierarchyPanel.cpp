#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>

namespace Engine
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
		m_Selected = {};
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Hierarchy");

		m_Context->m_Registry.each([&](auto entityID) {

			Entity entity{ entityID, m_Context.get() };

			DrawEntityNode(entity);
		});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			m_Selected = {};
		}

		// right click on blank space
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
			{
				m_Context->CreateEntity("Empty Entity");
			}
			ImGui::EndPopup();
		}
		
		ImGui::End();


		ImGui::Begin("Properties");

		if (m_Selected)
		{
			DrawComponents(m_Selected);

			if (ImGui::Button("Add Component"))
			{
				ImGui::OpenPopup("AddComponent");
			}

			if (ImGui::BeginPopup("AddComponent"))
			{

				if (ImGui::MenuItem("Camera"))
				{
					m_Selected.AddComponent<CameraComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::MenuItem("Sprite Renderer"))
				{
					m_Selected.AddComponent<SpriteRendererComponent>();
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tc = entity.GetComponent<TagComponent>();

		ImGuiTreeNodeFlags flags = ( m_Selected == entity ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

		bool open = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tc.Tag.c_str());

		if (ImGui::IsItemClicked())
		{
			m_Selected = entity;
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
			{
				entityDeleted = true;
			}
			ImGui::EndPopup();
		}

		if (open)
		{
			ImGui::TreePop();
		}

		if (entityDeleted) 
		{
			m_Context->DestroyEntity(entity);
			if (m_Selected == entity)
				m_Selected = {};
		}
	}

	template<typename T>
	static void DrawComponent(Entity& ent, const char* name, const std::function<void()> func, bool canRemove = true)
	{
		if (ent.HasComponent<T>())
		{

			ImVec2 contentRegion = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth, name);
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize = { lineHeight, lineHeight };
			ImGui::PopStyleVar();

			ImGui::SameLine(contentRegion.x - lineHeight * 0.5f);
			
			if (ImGui::Button("+", buttonSize))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (canRemove && ImGui::MenuItem("Remove Component"))
				{
					removeComponent = true;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			if (open)
			{
				func();
				ImGui::TreePop();
			}

			if (removeComponent)
				ent.RemoveComponent<T>();
		}
	}

	static void DrawVec3Control(const std::string label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGui::PushID(label.c_str());
		ImGui::Columns(2);

		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button,			ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,	ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,	ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button,			ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,	ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,	ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button,			ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,	ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,	ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());
			if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		DrawComponent<TransformComponent>(entity, "Transform", [&]() {
			auto& tc = entity.GetComponent<TransformComponent>();
			DrawVec3Control("Position", tc.Position, 0.0f);
			glm::vec3 rotation = glm::degrees(tc.Rotation);
			DrawVec3Control("Rotation", rotation, 0.0f);
			tc.Rotation = glm::radians(rotation);
			DrawVec3Control("Scale", tc.Scale, 1.0f);
		}, false);

		DrawComponent<CameraComponent>(entity, "Camera", [&]() {
			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			ImGui::Checkbox("Primary", &cameraComponent.Primary);
			ImGui::Checkbox("Fixed Aspect Ratio", &cameraComponent.FixedAspectRatio);

			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
			if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
			{

				for (int i = 0; i < 2; i++)
				{
					bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];

					if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
					{
						currentProjectionTypeString = projectionTypeStrings[i];
						camera.SetProjectionType((SceneCamera::ProjectionType)i);
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				float fov = glm::degrees(camera.GetPerspectiveVerticalFOV());
				if (ImGui::DragFloat("FOV", &fov))
					camera.SetPerspectiveVerticalFOV(glm::radians(fov));

				float nearClip = camera.GetPerspectiveNearClip();
				if (ImGui::DragFloat("Near Clip", &nearClip))
					camera.SetPerspectiveNearClip(nearClip);

				float farClip = camera.GetPerspectiveFarClip();
				if (ImGui::DragFloat("Far Clip", &farClip))
					camera.SetPerspectiveFarClip(farClip);
			}

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				float size = camera.GetOrthographicSize();
				if (ImGui::DragFloat("Size", &size))
					camera.SetOrthographicSize(size);

				float nearClip = camera.GetOrthographicNearClip();
				if (ImGui::DragFloat("Near Clip", &nearClip))
					camera.SetOrthographicNearClip(nearClip);

				float farClip = camera.GetOrthographicFarClip();
				if (ImGui::DragFloat("Far Clip", &farClip))
					camera.SetOrthographicFarClip(farClip);
			}
		});

		DrawComponent<SpriteRendererComponent>(entity, "Sprite Renderer", [&](){
			auto& color = entity.GetComponent<SpriteRendererComponent>().Color;
			ImGui::ColorEdit4("Color", glm::value_ptr(color));
		});
	}

}