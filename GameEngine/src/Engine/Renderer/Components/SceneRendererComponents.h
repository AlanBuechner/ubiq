#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Model.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Renderer/Light.h"
#include "Engine/Core/Scene/Components.h"
#include "Engine/Core/Scene/SceneCamera.h"

namespace Engine
{
	CLASS(GROUP = Component) CameraComponent : public Component
	{
	public:
		REFLECTED_BODY(Engine::CameraComponent);

	public:
		CameraComponent() {
			Camera = CreateRef<SceneCamera>();
		}

		Ref<SceneCamera> Camera;
		PROPERTY(HideInInspector) bool Primary = true;
		PROPERTY(HideInInspector) bool FixedAspectRatio = false;

		virtual void OnTransformChange(const Math::Mat4& transform) override;
	};

	CLASS(GROUP = Component, SceneStatic) DirectionalLightComponent : public Component
	{
	public:
		REFLECTED_BODY(Engine::DirectionalLightComponent);

	public:

		virtual void OnComponentAdded() override;
		virtual void OnInvalid() override;
		virtual void OnComponentRemoved() override;
		
		void SetAngles(Math::Vector2 rot);
		void SetDirection(Math::Vector3 direction);
		void SetTint(Math::Vector3 color);
		void SetTemperature(float temp);
		void SetIntensity(float intensity);
		void SetSize(float size);
		Ref<DirectionalLight> GetDirectinalLight() { return m_Light; }

		void UpdateShadowMaps();

	private:
		Ref<DirectionalLight> m_Light;

		bool m_Dirty = true;
	};

	CLASS(GROUP = Component, SceneStatic) SkyboxComponent : public Component
	{
	public:
		REFLECTED_BODY(Engine::SkyboxComponent);

	public:
		virtual void OnComponentAdded() override;
		virtual void OnComponentRemoved() override;

		Ref<Texture2D> GetSkyboxTexture() { return m_SkyboxTexture; }
		void SetSkyboxTexture(Ref<Texture2D> texture);

	private:
		PROPERTY(HideInInspector) Ref<Texture2D> m_SkyboxTexture;
	};
}
