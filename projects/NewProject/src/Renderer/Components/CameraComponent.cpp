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
