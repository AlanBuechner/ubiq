#include "SceneHierarchyPanel.h"
#include "Engine/AssetManager/AssetManager.h"
#include "Editor/EditorAssets.h"
#include "Editor/EditorLayer.h"
#include "Engine/Core/UUID.h"

#include "Engine/Core/Scene/TransformComponent.h"

#include "PropertiesPanel.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>

#include <filesystem>

namespace Editor
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Engine::Ref<Engine::Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Engine::Ref<Engine::Scene>& context)
	{
		m_Context = context;
		m_Selected = Engine::Entity::null;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Hierarchy");

		Utils::Vector<Engine::Entity> rootEntitys;
		m_Context->GetRegistry().EachEntity([&](auto entityID) {
			Engine::Entity entity{ entityID, m_Context.get() };
			if (entity.GetTransform().GetParent() == Engine::Entity::null)
				rootEntitys.Push(entity);
		});

		for (uint32 i = 0; i < rootEntitys.Count(); i++)
			DrawEntityNode(rootEntitys[i]);

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_Selected = Engine::Entity::null;

		// right click on blank space
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			CreateNewEntity();
			ImGui::EndPopup();
		}

		ImRect windowRect = {ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax()};
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

		if (m_Selected)
		{
			DrawComponents(m_Selected);

			if (ImGui::Button("Add Component"))
			{
				ImGui::OpenPopup("AddComponent");
			}

			if (ImGui::BeginPopup("AddComponent"))
			{
#define ADD_COMPONENT(name, component)\
if(!m_Selected.HasComponent<component>()){\
	if (ImGui::MenuItem(#name)){\
		m_Selected.AddComponent<component>();\
		ImGui::CloseCurrentPopup();\
	}\
}
				//ADD_COMPONENT(Camera, CameraComponent);
				//ADD_COMPONENT(Directional Light, DirectionalLightComponent);
				//ADD_COMPONENT(Static Model Renderer, StaticModelRendererComponent);
				//ADD_COMPONENT(Skybox, SkyboxComponent);

#undef ADD_COMPONENT

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

	void SceneHierarchyPanel::SelectEntity(Engine::Entity e)
	{ 
		m_Selected = e;
	}

	void SceneHierarchyPanel::CreateNewEntity(Engine::Entity parent)
	{
		bool entityAdded = false;
		Engine::Entity createdEntity;
		if (ImGui::MenuItem("Create Empty Entity"))
		{
			createdEntity = m_Context->CreateEntity("Empty Entity");
			entityAdded = true;
		}

		if (entityAdded && parent)
		{
			auto& tc = parent.GetTransform();
			tc.AddChild(createdEntity);
		}
		
	}

	void SceneHierarchyPanel::DrawEntityNode(Engine::Entity entity)
	{
		auto& children = entity.GetTransform().GetChildren();
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
				CreateNewEntity(entity);
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

	/*ADD_EXPOSE_PROP_FUNC(TransformComponent) {
		bool changed = false;
		TransformComponent& tc = *(TransformComponent*)voidData;
		Math::Vector3 posiiton = tc.GetPosition();
		if (changed |= PropertysPanel::DrawVec3Control("Position", posiiton, 0.0f))
			tc.SetPosition(posiiton);

		Math::Vector3 rotation = glm::degrees(tc.GetRotation());
		if (changed |= PropertysPanel::DrawVec3Control("Rotation", rotation, 0.0f))
			tc.SetRotation(glm::radians(rotation));

		Math::Vector3 scale = tc.GetScale();
		if (changed |= PropertysPanel::DrawVec3Control("Scale", scale, 1.0f))
			tc.SetScale(scale);
		return changed;
	});*/

}
