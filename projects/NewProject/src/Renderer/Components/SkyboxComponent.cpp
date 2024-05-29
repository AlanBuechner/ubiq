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
#include "Panels/PropertiesPanel.h"

namespace Game
{
}

#pragma endregion

#pragma region Serialization
#include "Engine/Core/Scene/SceneSerializer.h"

namespace Game
{
	class SkyboxSerializer : public Engine::ComponentSerializer
	{
		virtual void Serialize(Engine::Entity entity, YAML::Emitter& out) override
		{
			auto& skyboxComponent = *entity.GetComponent<SkyboxComponent>();
			if (skyboxComponent.GetSkyboxTexture())
				out << YAML::Key << "Texture" << YAML::Value << skyboxComponent.GetSkyboxTexture()->GetAssetID();
		}

		virtual void Deserialize(Engine::Entity entity, YAML::Node data) override
		{
			auto& sbc = *entity.GetComponent<SkyboxComponent>();
			if (data["Texture"])
				sbc.SetSkyboxTexture(Engine::Application::Get().GetAssetManager().GetAsset<Engine::Texture2D>(data["Texture"].as<uint64>()));
		}
	};
	ADD_COMPONENT_SERIALIZER(SkyboxComponent, SkyboxSerializer);

}
#pragma endregion
