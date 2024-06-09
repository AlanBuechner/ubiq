#include "CameraComponent.h"
#include "Engine/Util/Performance.h"

namespace Game
{

	// camera component
	void CameraComponent::OnTransformChange(const Math::Mat4& transform)
	{
		Camera->SetTransform(transform);
	}

	void CameraComponent::OnComponentAdded()
	{
		Camera->SetViewportSize(Owner.GetScene()->GetViewportWidth(), Owner.GetScene()->GetViewportHeight());
	}

	void CameraComponent::OnViewportResize(uint32 width, uint32 height)
	{
		Camera->SetViewportSize(width, height);
	}

}



#pragma region Editor
#include "Panels/PropertiesPanel.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace Game
{
	ADD_EXPOSE_PROP_FUNC(CameraComponent) {
		bool changed = false;
		CameraComponent& cameraComponent = *(CameraComponent*)voidData;
		auto& camera = cameraComponent.Camera;

		ImGui::Checkbox("Primary", &cameraComponent.Primary);
		ImGui::Checkbox("Fixed Aspect Ratio", &cameraComponent.FixedAspectRatio);

		const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
		const char* currentProjectionTypeString = projectionTypeStrings[(int)camera->GetProjectionType()];
		if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
		{
			for (int i = 0; i < _countof(projectionTypeStrings); i++)
			{
				bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];

				if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
				{
					currentProjectionTypeString = projectionTypeStrings[i];
					camera->SetProjectionType((Engine::SceneCamera::ProjectionType)i);
					changed = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		if (camera->GetProjectionType() == Engine::SceneCamera::ProjectionType::Perspective)
		{
			float fov = glm::degrees(camera->GetPerspectiveVerticalFOV());
			if (Engine::PropertysPanel::DrawFloatControl("FOV", fov, 45.0f))
			{
				camera->SetPerspectiveVerticalFOV(glm::radians(fov));
				changed = true;
			}

			float nearClip = camera->GetPerspectiveNearClip();
			if (Engine::PropertysPanel::DrawFloatControl("Near Clip", nearClip, 0.01f))
			{
				camera->SetPerspectiveNearClip(nearClip);
				changed = true;
			}

			float farClip = camera->GetPerspectiveFarClip();
			if (Engine::PropertysPanel::DrawFloatControl("Far Clip", farClip, 1000.0f))
			{
				camera->SetPerspectiveFarClip(farClip);
				changed = true;
			}
		}

		if (camera->GetProjectionType() == Engine::SceneCamera::ProjectionType::Orthographic)
		{
			float Size = camera->GetOrthographicSize();
			if (Engine::PropertysPanel::DrawFloatControl("Size", Size, 10.0f))
			{
				camera->SetOrthographicSize(Size);
				changed = true;
			}

			float nearClip = camera->GetOrthographicNearClip();
			if (Engine::PropertysPanel::DrawFloatControl("Near Clip", nearClip, -1.0f))
			{
				camera->SetOrthographicNearClip(nearClip);
				changed = true;
			}

			float farClip = camera->GetOrthographicFarClip();
			if (Engine::PropertysPanel::DrawFloatControl("Far Clip", farClip, 1.0f))
			{
				camera->SetOrthographicFarClip(farClip);
				changed = true;
			}
		}
		return changed;
	});
}

#pragma endregion

#pragma region Serialization
#include "Engine/Core/Scene/SceneSerializer.h"
namespace Game
{
	class CameraSerializer : public Engine::ComponentSerializer
	{
		virtual void Serialize(Engine::Entity entity, YAML::Emitter& out) override
		{
			auto& cameraComponent = *entity.GetComponent<Game::CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;
			out << YAML::Key << "Camera";
			out << YAML::BeginMap;
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera->GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera->GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera->GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera->GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera->GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera->GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera->GetOrthographicFarClip();
			out << YAML::EndMap; // end camera
		}

		virtual void Deserialize(Engine::Entity entity, YAML::Node data) override
		{
			auto& cc = *entity.GetComponent<Game::CameraComponent>();

			auto camera = data["Camera"];
			cc.Camera->SetProjectionType((Engine::SceneCamera::ProjectionType)camera["ProjectionType"].as<int>());
			cc.Camera->SetPerspectiveVerticalFOV(camera["PerspectiveFOV"].as<float>());
			cc.Camera->SetPerspectiveNearClip(camera["PerspectiveNear"].as<float>());
			cc.Camera->SetOrthographicFarClip(camera["PerspectiveFar"].as<float>());

			cc.Camera->SetOrthographicSize(camera["OrthographicSize"].as<float>());
			cc.Camera->SetOrthographicNearClip(camera["OrthographicNear"].as<float>());
			cc.Camera->SetOrthographicFarClip(camera["OrthographicFar"].as<float>());

			cc.Primary = data["Primary"].as<bool>();
			cc.FixedAspectRatio = data["FixedAspectRatio"].as<bool>();
		}
	};
	ADD_COMPONENT_SERIALIZER(Game::CameraComponent, CameraSerializer);
}
#pragma endregion
