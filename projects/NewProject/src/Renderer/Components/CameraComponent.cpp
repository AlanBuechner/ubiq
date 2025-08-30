#include "CameraComponent.h"
#include "Utils/Performance.h"

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
#if defined(EDITOR)

#include "Editor/Panels/PropertiesPanel.h"
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
			if (Editor::PropertysPanel::DrawFloatControl("FOV", fov, 45.0f))
			{
				camera->SetPerspectiveVerticalFOV(glm::radians(fov));
				changed = true;
			}

			float nearClip = camera->GetPerspectiveNearClip();
			if (Editor::PropertysPanel::DrawFloatControl("Near Clip", nearClip, 0.01f))
			{
				camera->SetPerspectiveNearClip(nearClip);
				changed = true;
			}

			float farClip = camera->GetPerspectiveFarClip();
			if (Editor::PropertysPanel::DrawFloatControl("Far Clip", farClip, 1000.0f))
			{
				camera->SetPerspectiveFarClip(farClip);
				changed = true;
			}
		}

		if (camera->GetProjectionType() == Engine::SceneCamera::ProjectionType::Orthographic)
		{
			float Size = camera->GetOrthographicSize();
			if (Editor::PropertysPanel::DrawFloatControl("Size", Size, 10.0f))
			{
				camera->SetOrthographicSize(Size);
				changed = true;
			}

			float nearClip = camera->GetOrthographicNearClip();
			if (Editor::PropertysPanel::DrawFloatControl("Near Clip", nearClip, -1.0f))
			{
				camera->SetOrthographicNearClip(nearClip);
				changed = true;
			}

			float farClip = camera->GetOrthographicFarClip();
			if (Editor::PropertysPanel::DrawFloatControl("Far Clip", farClip, 1.0f))
			{
				camera->SetOrthographicFarClip(farClip);
				changed = true;
			}
		}
		return changed;
	});
}

#endif
#pragma endregion

#pragma region Serialization


#include "Engine/Core/ObjectDescription/ObjectDescription.h"

namespace Engine
{
	template<>
	struct Convert<Game::CameraComponent>
	{
		CONVERTER_BASE(Game::CameraComponent);
		static ObjectDescription Encode(const Game::CameraComponent& val)
		{
			auto& camera = val.Camera;

			ObjectDescription desc(ObjectDescription::Type::Object);
			desc["Primary"] = ObjectDescription::CreateFrom(val.Primary);
			desc["FixedAspectRatio"] = ObjectDescription::CreateFrom(val.FixedAspectRatio);
			ObjectDescription& cameraDesc = desc["Camera"].SetType(ObjectDescription::Type::Object);
			cameraDesc["ProjectionType"] = ObjectDescription::CreateFrom((uint32)camera->GetProjectionType());
			// perspective
			cameraDesc["PerspectiveFOV"] = ObjectDescription::CreateFrom(camera->GetPerspectiveVerticalFOV());
			cameraDesc["PerspectiveNear"] = ObjectDescription::CreateFrom(camera->GetPerspectiveNearClip());
			cameraDesc["PerspectiveFar"] = ObjectDescription::CreateFrom(camera->GetPerspectiveFarClip());
			// orthographic
			cameraDesc["OrthographicSize"] = ObjectDescription::CreateFrom(camera->GetOrthographicSize());
			cameraDesc["OrthographicNear"] = ObjectDescription::CreateFrom(camera->GetOrthographicNearClip());
			cameraDesc["OrthographicFar"] = ObjectDescription::CreateFrom(camera->GetOrthographicFarClip());

			return desc;
		}

		static bool Decode(Game::CameraComponent& cc, const ObjectDescription& data)
		{
			auto camera = data["Camera"];
			cc.Camera->SetProjectionType((Engine::SceneCamera::ProjectionType)camera["ProjectionType"].Get<uint32>());
			cc.Camera->SetPerspectiveVerticalFOV(camera["PerspectiveFOV"].Get<float>());
			cc.Camera->SetPerspectiveNearClip(camera["PerspectiveNear"].Get<float>());
			cc.Camera->SetOrthographicFarClip(camera["PerspectiveFar"].Get<float>());

			cc.Camera->SetOrthographicSize(camera["OrthographicSize"].Get<float>());
			cc.Camera->SetOrthographicNearClip(camera["OrthographicNear"].Get<float>());
			cc.Camera->SetOrthographicFarClip(camera["OrthographicFar"].Get<float>());

			cc.Primary = data["Primary"].Get<bool>();
			cc.FixedAspectRatio = data["FixedAspectRatio"].Get<bool>();
			return true;
		}
	};
	ADD_OBJECT_CONVERTER(Game::CameraComponent);
}
#pragma endregion
