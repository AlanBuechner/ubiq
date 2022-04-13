#include "SceneHierarchyPanel.h"
#include "Engine/AssetManager/AssetManager.h"
#include "EditorAssets.h"
#include "EditorLayer.h"
#include <Engine/Core/UUID.h>

#include "PropertiesPanel.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>

#include <filesystem>

namespace Engine
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
		m_Selected = Entity::null;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Hierarchy");

		m_Context->m_Registry.each([&](auto entityID) {
			Entity entity{ entityID, m_Context.get() };

			if(entity.GetTransform().GetParent() == Entity::null)
				DrawEntityNode(entity);
		});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			m_Selected = Entity::null;
		}

		// right click on blank space
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			CreateNewEntity();
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

				if (!m_Selected.HasComponent<CameraComponent>())
				{
					if (ImGui::MenuItem("Camera"))
					{
						m_Selected.AddComponent<CameraComponent>();
						ImGui::CloseCurrentPopup();
					}
				}

				if (!m_Selected.HasComponent<SpriteRendererComponent>())
				{
					if (ImGui::MenuItem("Sprite Renderer"))
					{
						m_Selected.AddComponent<SpriteRendererComponent>();
						ImGui::CloseCurrentPopup();
					}
				}

				if (!m_Selected.HasComponent<Rigidbody2DComponent>())
				{
					if (ImGui::MenuItem("Rigidbody 2D"))
					{
						m_Selected.AddComponent<Rigidbody2DComponent>();
						ImGui::CloseCurrentPopup();
					}
				}

				if (!m_Selected.HasComponent<BoxCollider2DComponent>())
				{
					if (ImGui::MenuItem("Box Collider 2D"))
					{
						m_Selected.AddComponent<BoxCollider2DComponent>();
						ImGui::CloseCurrentPopup();
					}
				}
				
				if (!m_Selected.HasComponent<CircleColliderComponent>())
				{
					if (ImGui::MenuItem("Circle Collider 2D"))
					{
						m_Selected.AddComponent<CircleColliderComponent>();
						ImGui::CloseCurrentPopup();
					}
				}

				ImGui::EndPopup();
			}
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::SelectEntity(Entity e) 
	{ 
		m_Selected = e;
	}

	void SceneHierarchyPanel::CreateNewEntity(Entity parent)
	{
		bool entityAdded = false;
		Entity createdEntity;
		if (ImGui::MenuItem("Create Empty Entity"))
		{
			createdEntity = m_Context->CreateEntity("Empty Entity");
			entityAdded = true;
		}

		if (ImGui::MenuItem("Create Sprite"))
		{
			(createdEntity = m_Context->CreateEntity("Sprite")).AddComponent<SpriteRendererComponent>();
			entityAdded = true;
		}

		if (ImGui::MenuItem("Create Camera"))
		{
			(createdEntity = m_Context->CreateEntity("Camera")).AddComponent<CameraComponent>();
			entityAdded = true;
		}

		if (entityAdded && parent)
		{
			auto& tc = parent.GetTransform();
			tc.AddChild(createdEntity);
		
		}
		
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto children = entity.GetTransform().GetChildren();
		ImGuiTreeNodeFlags flags = ( m_Selected == entity ? ImGuiTreeNodeFlags_Selected : 0) |
			(children.size() == 0 ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_OpenOnArrow) |
			ImGuiTreeNodeFlags_SpanAvailWidth;

		bool open = ImGui::TreeNodeEx((void*)(uint64)(uint32)entity, flags, entity.GetTag().c_str());

		if (ImGui::IsItemClicked())
		{
			m_Selected = entity;
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;
			
			if (ImGui::BeginMenu("Create"))
			{
				CreateNewEntity(entity);
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}

		children = entity.GetTransform().GetChildren(); // update children array

		if (open)
		{
			for (uint32_t i = 0; i < children.size(); i++)
			{
				Entity child = children[i];
				DrawEntityNode(child);
			}
			ImGui::TreePop();
		}

		if (entityDeleted) 
		{
			m_Context->DestroyEntity(entity);
			if (m_Selected == entity)
				m_Selected = Entity::null;
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
				ImGui::Spacing();
			}

			if (removeComponent)
				ent.RemoveComponent<T>();
		}
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
			PropertysPanel::DrawVec3Control("Position", tc.Position, 0.0f);
			Math::Vector3 rotation = glm::degrees(tc.Rotation);
			PropertysPanel::DrawVec3Control("Rotation", rotation, 0.0f);
			tc.Rotation = glm::radians(rotation);
			PropertysPanel::DrawVec3Control("Scale", tc.Scale, 1.0f);
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
				if (PropertysPanel::DrawFloatControl("FOV", fov, 45.0f))
					camera.SetPerspectiveVerticalFOV(glm::radians(fov));

				float nearClip = camera.GetPerspectiveNearClip();
				if (PropertysPanel::DrawFloatControl("Near Clip", nearClip, 0.01f))
					camera.SetPerspectiveNearClip(nearClip);

				float farClip = camera.GetPerspectiveFarClip();
				if (PropertysPanel::DrawFloatControl("Far Clip", farClip, 1000.0f))
					camera.SetPerspectiveFarClip(farClip);
			}

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				float Size = camera.GetOrthographicSize();
				if (PropertysPanel::DrawFloatControl("Size", Size, 10.0f))
					camera.SetOrthographicSize(Size);

				float nearClip = camera.GetOrthographicNearClip();
				if (PropertysPanel::DrawFloatControl("Near Clip", nearClip, -1.0f))
					camera.SetOrthographicNearClip(nearClip);

				float farClip = camera.GetOrthographicFarClip();
				if (PropertysPanel::DrawFloatControl("Far Clip", farClip, 1.0f))
					camera.SetOrthographicFarClip(farClip);
			}
		});

		DrawComponent<SpriteRendererComponent>(entity, "Sprite Renderer", [&](){
			auto& component = entity.GetComponent<SpriteRendererComponent>();
			auto& color = component.Color;
			ImGui::ColorEdit4("Color", glm::value_ptr(color));

			PropertysPanel::DrawTextureControl("Texture", component.Texture);
			
		});

		DrawComponent<Rigidbody2DComponent>(entity, "Rigidbody 2D", [&]() {
			auto& component = entity.GetComponent<Rigidbody2DComponent>();

			const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
			const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];
			if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
			{
				for (uint32 i = 0; i < 3; i++)
				{
					bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
					if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
					{
						currentBodyTypeString = bodyTypeStrings[i];
						component.Type = (Rigidbody2DComponent::BodyType)i;
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
		});

		DrawComponent<BoxCollider2DComponent>(entity, "Box Collider 2D", [&]() {
			auto& component = entity.GetComponent<BoxCollider2DComponent>();

			PropertysPanel::DrawVec2Control("Offset", component.Offset, 0.0f);
			PropertysPanel::DrawVec2Control("Size", component.Size, 0.0f);

			ImGui::DragFloat("Density", &component.Density, 0.1f);
			ImGui::DragFloat("Friction", &component.Friction, 0.1f);
			ImGui::DragFloat("Restitution", &component.Restitution, 0.1f);
			ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.1f);

		});

		DrawComponent<CircleColliderComponent>(entity, "Circle Collider 2D", [&]() {
			auto& component = entity.GetComponent<CircleColliderComponent>();

			PropertysPanel::DrawVec2Control("Offset", component.Offset, 0.0f);

			PropertysPanel::DrawFloatControl("Size", component.Size, 0.5f);
			PropertysPanel::DrawFloatControl("Density", component.Density, 1.0f);
			PropertysPanel::DrawFloatControl("Friction", component.Friction, 0.5f);
			PropertysPanel::DrawFloatControl("Restitution", component.Restitution, 0.0f);
			PropertysPanel::DrawFloatControl("Restitution Threshold", component.RestitutionThreshold, 0.5f);

		});

	}

}
