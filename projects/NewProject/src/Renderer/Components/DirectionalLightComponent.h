#pragma once
#include "Engine/Core/Scene/Components.h"

namespace Engine 
{
	class DirectionalLight;
}

namespace Game
{
	CLASS(GROUP = Component, SceneStatic) DirectionalLightComponent : public Engine::Component
	{
	public:
		REFLECTED_BODY(Game::DirectionalLightComponent);

	public:

		virtual void OnComponentAdded() override;
		virtual void OnPreRender() override;
		virtual void OnComponentRemoved() override;

		void SetAngles(Math::Vector2 rot);
		void SetDirection(Math::Vector3 direction);
		void SetTint(Math::Vector3 color);
		void SetTemperature(float temp);
		void SetIntensity(float intensity);
		void SetSize(float size);
		Engine::Ref<Engine::DirectionalLight> GetDirectinalLight() { return m_Light; }

		void UpdateShadowMaps();

	private:
		Engine::Ref<Engine::DirectionalLight> m_Light;

		bool m_Dirty = true;
	};
}
