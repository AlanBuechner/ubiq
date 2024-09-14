#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/Scene/Components.h"
#include "Engine/Core/Scene/SceneCamera.h"


namespace Game
{
	CLASS(GROUP = Component) CameraComponent : public Engine::Component
	{
	public:
		REFLECTED_BODY(Game::CameraComponent);

	public:
		CameraComponent() {
			Camera = Engine::CreateRef<Engine::SceneCamera>();
		}

		Engine::Ref<Engine::SceneCamera> Camera;
		PROPERTY(HideInInspector) bool Primary = true;
		PROPERTY(HideInInspector) bool FixedAspectRatio = false;

		virtual void OnTransformChange(const Math::Mat4& transform) override;
		virtual void OnViewportResize(uint32 width, uint32 height) override;
		virtual void OnComponentAdded() override;
	};
}
