#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Model.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Renderer/Light.h"
#include "Engine/Core/Scene/Components.h"

namespace Engine
{
	struct CameraComponent : public Component
	{
		CameraComponent() {
			Camera = CreateRef<SceneCamera>();
		}

		Ref<SceneCamera> Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		virtual void OnTransformChange(const Math::Mat4& transform) override;
	};

	class DirectionalLightComponent : public Component, SceneStatic
	{
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
		Ref<const DirectionalLight> GetDirectinalLight() { return m_Light; }

		void UpdateShadowMaps();

	private:
		Ref<DirectionalLight> m_Light;

		bool m_Dirty = true;
	};

	class SkyboxComponent : public Component, SceneStatic
	{
	public:
		virtual void OnComponentAdded() override;
		virtual void OnComponentRemoved() override;

		Ref<Texture2D> GetSkyboxTexture() { return m_SkyboxTexture; }
		void SetSkyboxTexture(Ref<Texture2D> texture);

	private:
		Ref<Texture2D> m_SkyboxTexture;
	};
}
