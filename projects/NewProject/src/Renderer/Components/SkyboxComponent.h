#pragma once
#include "Engine/Core/Scene/Components.h"

namespace Game
{
	CLASS(GROUP = Component, SceneStatic) SkyboxComponent : public Engine::Component
	{
	public:
		REFLECTED_BODY(Game::SkyboxComponent);

	public:
		virtual void OnComponentAdded() override;
		virtual void OnComponentRemoved() override;

		Engine::Ref<Engine::Texture2D> GetSkyboxTexture() { return m_SkyboxTexture; }
		FUNCTION()
		void SetSkyboxTexture(Engine::Ref<Engine::Texture2D> texture);

	private:
		PROPERTY(set=SetSkyboxTexture) 
		Engine::Ref<Engine::Texture2D> m_SkyboxTexture;
	};
}
