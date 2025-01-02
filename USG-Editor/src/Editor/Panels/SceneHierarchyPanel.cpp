#include "SceneHierarchyPanel.h"
#include "PropertiesPanel.h"

#include "Engine/AssetManager/AssetManager.h"
#include "Engine/Core/Scene/TransformComponent.h"
#include "Engine/Core/UUID.h"

#include "Editor/EditorAssets.h"
#include "Editor/EditorLayer.h"
#include "Utils/Common.h"

#include <glm/gtc/type_ptr.hpp>
#include <filesystem>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <ImGuizmo/ImGuizmo.h>

namespace Editor
{
	void SceneHierarchyPanel::OnSceneChange(Engine::Ref<Engine::Scene> context)
	{
		m_Context = context;
		m_Selected = Engine::Entity::null;
	}

	void SceneHierarchyPanel::OnScreenClick(Math::Vector2 pos)
	{
		pos.y = 1 - pos.y;

		Engine::GameBase* game = Engine::GameBase::Get();
		Engine::Ref<Engine::EditorCamera> editorCamera = Editor::EditorLayer::Get()->GetEditorCamera();

		// calculate ray
		Engine::Ray ray;
		ray.m_Origin = editorCamera->GetPosition();
		Math::Vector4 ndc = Math::Vector4(pos * 2.0f - 1.0f, 1, 1);
		Math::Vector4 worldSpace = Math::Inverse(editorCamera->GetViewProjection()) * ndc;
		worldSpace = worldSpace / worldSpace.w;
		ray.m_Direction = Math::Vector3(worldSpace) - editorCamera->GetPosition();

		// find closest entity
		Engine::Entity hitEntity;
		float closestHit = FLT_MAX;

		game->GetScene()->GetRegistry().EachEntity([&](Engine::EntityType et) {
			Engine::Entity e{ et, game->GetScene().get() };
			Engine::AABB aabb = e.GetLocalAABB();
			if (aabb.m_Min != aabb.m_Max)
			{
				Engine::PlainVolume volume = e.GetPlainVolume();
				Engine::RayHit hit;
				if (volume.TestRay(ray, hit))
				{
					if (hit.m_Distance < closestHit)
					{
						closestHit = hit.m_Distance;
						hitEntity = e;
					}
				}
			}
		});

		SelectEntity(hitEntity);
	}


	void SceneHierarchyPanel::OnEvent(Engine::Event* e)
	{
		Engine::EventDispatcher dispacher(e);
		dispacher.Dispatch<Engine::KeyPressedEvent>(BIND_EVENT_FN(&SceneHierarchyPanel::OnKeyPressed));
	}

	bool SceneHierarchyPanel::OnKeyPressed(Engine::KeyPressedEvent* e)
	{
		bool controlPressed = Engine::Input::GetKeyDown(Engine::KeyCode::CONTROL);
		bool shiftPressed = Engine::Input::GetKeyDown(Engine::KeyCode::SHIFT);
		bool rightClick = Engine::Input::GetMouseButtonDown(Engine::MouseCode::RIGHT_MOUSE);

		if (!rightClick)
		{
			// imguizmo keyboard shortcuts
			switch (e->GetKeyCode())
			{
			case Engine::KeyCode::Q:
				m_GizmoType = -1;
				break;
			case Engine::KeyCode::G:
			case Engine::KeyCode::T:
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			case Engine::KeyCode::S:
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
				break;
			case Engine::KeyCode::R:
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			default:
				break;
			}
		}

		return true;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Hierarchy");

		// get all root entities
		Utils::Vector<Engine::Entity> rootEntities;
		m_Context->GetRegistry().EachEntity([&](auto entityID) {
			Engine::Entity entity{ entityID, m_Context.get() };
			if (entity.GetTransform().GetParent() == Engine::Entity::null)
				rootEntities.Push(entity);
		});

		// draw all root entities
		for (uint32 i = 0; i < rootEntities.Count(); i++)
			DrawEntityNode(rootEntities[i]);

		// clear selected entity when clicking on empty
		if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
			m_Selected = Engine::Entity::null;

		// right click on blank space
		if (ImGui::BeginPopupContextWindow(0, ImGuiMouseButton_Right, false))
		{
			DrawCreateNewEntity();
			ImGui::EndPopup();
		}

		// drag and drop to clear entity parent
		ImRect windowRect = { ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax() };
		if (ImGui::BeginDragDropTargetCustom(windowRect, ImGui::GetWindowDockID()))
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
			{
				Engine::Entity dropedEntity = *(Engine::Entity*)payload->Data;
				dropedEntity.GetTransform().SetParentToRoot();
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::End();



		ImGui::Begin("Properties");

		// draw selected entities components
		if (m_Selected)
		{
			DrawComponents(m_Selected);

			if (ImGui::Button("Add Component"))
				ImGui::OpenPopup("AddComponent");

			if (ImGui::BeginPopup("AddComponent"))
			{
				std::vector<const Reflect::Class*> components = Reflect::Registry::GetRegistry()->GetGroup("Component");
				for (const Reflect::Class* componetClass : components)
				{
					if (!m_Selected.GetScene()->GetRegistry().HasComponent(m_Selected, *componetClass))
					{
						if (ImGui::MenuItem(componetClass->GetName().c_str()))
						{
							m_Selected.GetScene()->GetRegistry().AddComponent(m_Selected, m_Selected.GetScene(), *componetClass);
							ImGui::CloseCurrentPopup();
						}
					}
				}

				ImGui::EndPopup();
			}
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::OnDrawGizmos()
	{
		Engine::Entity selected = GetSelectedEntity();
		if (selected)
		{
			// TODO : Fix for child entity's
			// Gizmo's
			if (m_GizmoType != -1)
			{
				// get editor camera transform
				Engine::Ref<Engine::EditorCamera> editorCamera = EditorLayer::Get()->GetEditorCamera();
				const Math::Mat4& cameraProjection = editorCamera->GetProjectionMatrix();
				const Math::Mat4& cameraView = editorCamera->GetViewMatrix();

				// get position rotation and scale from global transform
				auto& tc = selected.GetTransform(); // get the transform component
				Math::Mat4 transform = tc.GetGlobalTransform(); // get the transform matrix
				Math::Vector3 OldPosition, OldRotation, OldScale;
				Math::DecomposeTransform(transform, OldPosition, OldRotation, OldScale);

				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();

				float windowWidth = (float)ImGui::GetWindowWidth();
				float windowHeight = (float)ImGui::GetWindowHeight();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
					(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform));

				if (ImGuizmo::IsUsing())
				{
					Math::Vector3 position, rotation, scale;
					Math::DecomposeTransform(transform, position, rotation, scale);

					tc.Translate(position - OldPosition);
					tc.Rotate(rotation - OldRotation);
					tc.Scale(scale - OldScale);
				}
			}
		}
	}

	void SceneHierarchyPanel::DrawCreateNewEntity(Engine::Entity parent)
	{
		if (ImGui::MenuItem("Create Empty Entity"))
		{
			Engine::Entity createdEntity = m_Context->CreateEntity("Empty Entity");
			if (parent)
				parent.GetTransform().AddChild(createdEntity);
		}
	}

	void SceneHierarchyPanel::DrawEntityNode(Engine::Entity entity)
	{
		const Utils::Vector<Engine::Entity>& children = entity.GetTransform().GetChildren();
		ImGuiTreeNodeFlags flags = ( m_Selected == entity ? ImGuiTreeNodeFlags_Selected : 0) |
			(children.Empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_OpenOnArrow) |
			ImGuiTreeNodeFlags_SpanAvailWidth;

		bool open = ImGui::TreeNodeEx((void*)(uint64)(uint32)entity, flags, entity.GetName().c_str());

		if (ImGui::IsItemClicked())
			m_Selected = entity;

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("ENTITY", &entity, sizeof(Engine::Entity));
			ImGui::Text(entity.GetName().c_str());
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
			{
				Engine::Entity dropedEntity = *(Engine::Entity*)payload->Data;
				if (dropedEntity != entity)
					entity.GetTransform().AddChild(dropedEntity);
			}
			ImGui::EndDragDropTarget();
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;
			
			if (ImGui::BeginMenu("Create"))
			{
				DrawCreateNewEntity(entity);
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}

		if (open)
		{
			for (uint32_t i = 0; i < children.Count(); i++)
			{
				Engine::Entity child = children[i];
				DrawEntityNode(child);
			}
			ImGui::TreePop();
		}

		if (entityDeleted) 
		{
			m_Context->DestroyEntity(entity);
			if (m_Selected == entity)
				m_Selected = Engine::Entity::null;
		}
	}

	void SceneHierarchyPanel::DrawComponents(Engine::Entity entity)
	{
		auto& name = entity.GetName();

		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		strcpy_s(buffer, sizeof(buffer), name.c_str());
		if (ImGui::InputText("Name", buffer, sizeof(buffer)))
			name = std::string(buffer);
		
		Utils::Vector<Engine::Component*> components = entity.GetComponents();
		for (Engine::Component* comp : components)
		{
			ImVec2 contentRegion = ImGui::GetContentRegionAvail();
			const Reflect::Class& componentClass = comp->GetClass();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			bool open = ImGui::TreeNodeEx(
				(void*)componentClass.GetTypeID(),
				ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth,
				componentClass.GetName().c_str()
			);
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize = { lineHeight, lineHeight };
			ImGui::PopStyleVar();

			ImGui::SameLine(contentRegion.x - lineHeight * 0.5f);

			if (ImGui::Button("+", buttonSize))
				ImGui::OpenPopup("ComponentSettings");

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (!componentClass.HasFlag("DontRemove") && ImGui::MenuItem("Remove Component"))
				{
					removeComponent = true;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			if (open)
			{
				PropertysPanel::DrawPropertyControl(comp, componentClass.GetTypeID(), nullptr);

				ImGui::TreePop();
				ImGui::Spacing();
			}

			if (removeComponent)
				entity.RemoveComponent(comp);
		}
	}

}
