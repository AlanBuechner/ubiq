#pragma once
#include "Engine/Core/Scene/Components.h"

namespace Game 
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
		virtual void OnComponentRemoved() override;
		FUNCTION() void OnPreRender();

		void SetAngles(Math::Vector2 rot);
		void SetDirection(Math::Vector3 direction);
		void SetTint(Math::Vector3 color);
		void SetTemperature(float temp);
		void SetIntensity(float intensity);
		void SetSize(float size);
		Engine::Ref<DirectionalLight> GetDirectinalLight() { return m_Light; }

		void UpdateShadowMaps();

	private:
		Engine::Ref<DirectionalLight> m_Light;

		bool m_Dirty = true;
	};
}
