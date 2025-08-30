#include "SkyboxComponent.h"
#include "Renderer/SceneRenderer/SceneRenderer.h"



namespace Game
{
	void SkyboxComponent::OnComponentAdded()
	{

	}

	void SkyboxComponent::OnComponentRemoved()
	{
		Owner.GetScene()->GetSceneRendererAs<SceneRenderer>()->SetSkyBox(nullptr);
	}

	void SkyboxComponent::SetSkyboxTexture(Engine::Ref<Engine::Texture2D> texture)
	{
		m_SkyboxTexture = texture;
		Owner.GetScene()->GetSceneRendererAs<SceneRenderer>()->SetSkyBox(m_SkyboxTexture);
	}
}


#pragma region Editor
#if defined(EDITOR)

#include "Editor/Panels/PropertiesPanel.h"

namespace Game
{
}

#endif
#pragma endregion

#pragma region Serialization
#include "Engine/Core/ObjectDescription/ObjectDescription.h"

namespace Engine
{
	template<>
	struct Convert<Game::SkyboxComponent>
	{
		CONVERTER_BASE(Game::SkyboxComponent);
		static ObjectDescription Encode(const Game::SkyboxComponent& val)
		{
			ObjectDescription desc(ObjectDescription::Type::Object);
			if (val.GetSkyboxTexture())
				desc["Texture"] = ObjectDescription::CreateFrom((uint64)val.GetSkyboxTexture()->GetAssetID());

			return desc;
		}

		static bool Decode(Game::SkyboxComponent& sbc, const ObjectDescription& data)
		{
			if (data.HasEntery("Texture"))
				sbc.SetSkyboxTexture(Engine::Application::Get().GetAssetManager().GetAsset<Engine::Texture2D>(data["Texture"].Get<uint64>()));

			return true;
		}
	};
	ADD_OBJECT_CONVERTER(Game::SkyboxComponent);
}
#pragma endregion
