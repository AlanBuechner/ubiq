#include "SceneHierarchyPanel.h"
#include "Engine/AssetManager/AssetManager.h"
#include "EditorAssets.h"
#include "EditorLayer.h"
#include "Engine/Core/UUID.h"

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

		std::vector<Entity> rootEntitys;
		m_Context->m_Registry.Each([&](auto entityID) {
			Entity entity{ entityID, m_Context.get() };
			if (entity.GetTransform().GetParent() == Entity::null)
				rootEntitys.push_back(entity);
		});

		for (uint32_t i = 0; i < rootEntitys.size(); i++)
			DrawEntityNode(rootEntitys[i]);

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_Selected = Entity::null;

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
				Entity dropedEntity = *(Entity*)payload->Data;
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
				ADD_COMPONENT(Camera, CameraComponent);
				ADD_COMPONENT(Directional Light, DirectionalLightComponent);
				ADD_COMPONENT(Mesh Renderer, MeshRendererComponent);
				ADD_COMPONENT(Skybox, SkyboxComponent);

#undef ADD_COMPONENT

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

		if (ImGui::MenuItem("Create Camera"))
		{
			(createdEntity = m_Context->CreateEntity("Camera")).AddComponent<CameraComponent>();
			entityAdded = true;
		}

		if (ImGui::MenuItem("Create Directional Light"))
		{
			(createdEntity = m_Context->CreateEntity("Directional Light")).AddComponent<DirectionalLightComponent>();
			entityAdded = true;
		}

		if (ImGui::MenuItem("Create Mesh"))
		{
			(createdEntity = m_Context->CreateEntity("Mesh")).AddComponent<MeshRendererComponent>();
			entityAdded = true;
		}

		if (ImGui::MenuItem("Create Skybox"))
		{
			(createdEntity = m_Context->CreateEntity("Skybox")).AddComponent<SkyboxComponent>();
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
		auto& children = entity.GetTransform().GetChildren();
		ImGuiTreeNodeFlags flags = ( m_Selected == entity ? ImGuiTreeNodeFlags_Selected : 0) |
			(children.size() == 0 ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_OpenOnArrow) |
			ImGuiTreeNodeFlags_SpanAvailWidth;

		bool open = ImGui::TreeNodeEx((void*)(uint64)(uint32)entity, flags, entity.GetName().c_str());

		if (ImGui::IsItemClicked())
			m_Selected = entity;

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("ENTITY", &entity, sizeof(Entity));
			ImGui::Text(entity.GetName().c_str());
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
			{
				Entity dropedEntity = *(Entity*)payload->Data;
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

		auto& name = entity.GetName();

		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		strcpy_s(buffer, sizeof(buffer), name.c_str());
		if (ImGui::InputText("Name", buffer, sizeof(buffer)))
			name = std::string(buffer);
		

		DrawComponent<TransformComponent>(entity, "Transform", [&]() {
			auto& tc = entity.GetComponent<TransformComponent>();
			Math::Vector3 posiiton = tc.GetPosition();
			if (PropertysPanel::DrawVec3Control("m_Position", posiiton, 0.0f))
				tc.SetPosition(posiiton);

			Math::Vector3 rotation = glm::degrees(tc.GetRotation());
			if (PropertysPanel::DrawVec3Control("Rotation", rotation, 0.0f))
				tc.SetRotation(glm::radians(rotation));

			Math::Vector3 scale = tc.GetScale();
			if (PropertysPanel::DrawVec3Control("Scale", scale, 1.0f))
				tc.SetScale(scale);

		}, false);

		DrawComponent<CameraComponent>(entity, "Camera", [&]() {
			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			ImGui::Checkbox("Primary", &cameraComponent.Primary);
			ImGui::Checkbox("Fixed Aspect Ratio", &cameraComponent.FixedAspectRatio);

			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProjectionTypeString = projectionTypeStrings[(int)camera->GetProjectionType()];
			if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
			{
				for (int i = 0; i < 2; i++)
				{
					bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];

					if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
					{
						currentProjectionTypeString = projectionTypeStrings[i];
						camera->SetProjectionType((SceneCamera::ProjectionType)i);
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			if (camera->GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				float fov = glm::degrees(camera->GetPerspectiveVerticalFOV());
				if (PropertysPanel::DrawFloatControl("FOV", fov, 45.0f))
					camera->SetPerspectiveVerticalFOV(glm::radians(fov));

				float nearClip = camera->GetPerspectiveNearClip();
				if (PropertysPanel::DrawFloatControl("Near Clip", nearClip, 0.01f))
					camera->SetPerspectiveNearClip(nearClip);

				float farClip = camera->GetPerspectiveFarClip();
				if (PropertysPanel::DrawFloatControl("Far Clip", farClip, 1000.0f))
					camera->SetPerspectiveFarClip(farClip);
			}

			if (camera->GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				float Size = camera->GetOrthographicSize();
				if (PropertysPanel::DrawFloatControl("Size", Size, 10.0f))
					camera->SetOrthographicSize(Size);

				float nearClip = camera->GetOrthographicNearClip();
				if (PropertysPanel::DrawFloatControl("Near Clip", nearClip, -1.0f))
					camera->SetOrthographicNearClip(nearClip);

				float farClip = camera->GetOrthographicFarClip();
				if (PropertysPanel::DrawFloatControl("Far Clip", farClip, 1.0f))
					camera->SetOrthographicFarClip(farClip);
			}
		});

		DrawComponent<DirectionalLightComponent>(entity, "Directional Light", [&]() {
			auto& component = entity.GetComponent<DirectionalLightComponent>();

			Math::Vector3 direction = component.GetDirectinalLight()->GetDirection();
			Math::Vector2 rot = component.GetDirectinalLight()->GetAngles();

			if (PropertysPanel::DrawVec2Control("Direction", rot))
				component.SetAngles(rot);

			float temp = component.GetDirectinalLight()->GetCCT();
			if (PropertysPanel::DrawFloatSlider("Temperature", temp, 1700, 20000, 6600))
				component.SetTemperature(temp);

			Math::Vector3 color = component.GetDirectinalLight()->GetTint();
			if (PropertysPanel::DrawColorControl("Color", color))
				component.SetTint(color);

			float intensity = component.GetDirectinalLight()->GetIntensity();
			if (PropertysPanel::DrawFloatControl("Intensity", intensity))
				component.SetIntensity(intensity);

			float size = component.GetDirectinalLight()->GetSize();
			if (PropertysPanel::DrawFloatControl("Size", size))
				component.SetSize(size);
		});

		DrawComponent<MeshRendererComponent>(entity, "Mesh Renderer", [&](){
			auto& component = entity.GetComponent<MeshRendererComponent>();
			
			Ref<Mesh> mesh = component.GetMesh();
			if (PropertysPanel::DrawMeshControl("Mesh", mesh))
				component.SetMesh(mesh);


			Ref<Material> mat = component.GetMaterial();
			if (PropertysPanel::DrawMaterialControl("Material", mat))
				component.SetMaterial(mat);
		});

		DrawComponent<SkyboxComponent>(entity, "Skybox", [&]() {
			auto& component = entity.GetComponent<SkyboxComponent>();

			Ref<Texture2D> texture = component.GetSkyboxTexture();
			if (PropertysPanel::DrawTextureControl("Texture", texture))
				component.SetSkyboxTexture(texture);
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

			PropertysPanel::DrawFloatControl("Density", component.Density, 1.0f);
			PropertysPanel::DrawFloatControl("Friction", component.Friction, 0.5f);
			PropertysPanel::DrawFloatControl("Restitution", component.Restitution, 0.0f);
			PropertysPanel::DrawFloatControl("Restitution Threshold", component.RestitutionThreshold, 0.5f);
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
