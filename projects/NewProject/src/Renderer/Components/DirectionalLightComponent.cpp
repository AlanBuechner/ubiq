#include "DirectionalLightComponent.h"
#include "Renderer/Lighting/DirectionalLight.h"
#include "Renderer/SceneRenderer/SceneRenderer.h"

namespace Game
{
	void DirectionalLightComponent::OnComponentAdded()
	{
		m_Light = Engine::CreateRef<DirectionalLight>(Math::Vector3{ 0,-1,0 }, Math::Vector3{ 1,1,1 }, 1);

		Owner.GetScene()->GetSceneRendererAs<SceneRenderer>()->SetDirectionalLight(m_Light);
	}

	void DirectionalLightComponent::OnPreRender()
	{
		if (m_Dirty)
		{
			m_Light->Apply();
			m_Dirty = false;
		}

		CREATE_PROFILE_SCOPEI("Update Directional Light Component")
		if (Owner.GetScene()->DidCameraChange())
		{
			// remove all cameras from directional light
			GetDirectinalLight()->ClearCameras();
			// re add cameras to directional light
			GetDirectinalLight()->AddCamera(Owner.GetScene()->GetSceneRenderer()->GetMainCamera());
		}
		UpdateShadowMaps();
	}

	void DirectionalLightComponent::OnComponentRemoved()
	{
	}

	void DirectionalLightComponent::SetAngles(Math::Vector2 rot)
	{
		m_Light->SetAngles(rot);
		m_Dirty = true;
	}

	void DirectionalLightComponent::SetDirection(Math::Vector3 direction)
	{
		m_Light->SetDirection(direction);
		m_Dirty = true;
	}

	void DirectionalLightComponent::SetTint(Math::Vector3 color)
	{
		m_Light->SetTint(color);
		m_Dirty = true;
	}

	void DirectionalLightComponent::SetTemperature(float temp)
	{
		m_Light->SetCCT(temp);
		m_Dirty = true;
	}

	void DirectionalLightComponent::SetIntensity(float intensity)
	{
		m_Light->SetIntensity(intensity);
		m_Dirty = true;
	}

	void DirectionalLightComponent::SetSize(float size)
	{
		m_Light->SetSize(size);
		m_Dirty = true;
	}

	void DirectionalLightComponent::UpdateShadowMaps()
	{
		CREATE_PROFILE_FUNCTIONI();
		m_Light->UpdateShadowMaps();
	}
}

#pragma region Editor
#if defined(EDITOR)

#include "Editor/Panels/PropertiesPanel.h"

namespace Game
{
	ADD_EXPOSE_PROP_FUNC(DirectionalLightComponent) {
		bool changed = false;
		DirectionalLightComponent& component = *(DirectionalLightComponent*)voidData;
		Math::Vector3 direction = component.GetDirectinalLight()->GetDirection();
		Math::Vector2 rot = component.GetDirectinalLight()->GetAngles();

		if (Editor::PropertysPanel::DrawVec2Control("Direction", rot))
			component.SetAngles(rot); changed = true;

		float temp = component.GetDirectinalLight()->GetCCT();
		if (Editor::PropertysPanel::DrawFloatSlider("Temperature", temp, 1700, 20000, 6600))
		{
			component.SetTemperature(temp);
			changed = true;
		}

		Math::Vector3 color = component.GetDirectinalLight()->GetTint();
		if (Editor::PropertysPanel::DrawColorControl("Color", color))
		{
			component.SetTint(color);
			changed = true;
		}

		float intensity = component.GetDirectinalLight()->GetIntensity();
		if (Editor::PropertysPanel::DrawFloatControl("Intensity", intensity))
		{
			component.SetIntensity(intensity);
			changed = true;
		}

		float size = component.GetDirectinalLight()->GetSize();
		if (Editor::PropertysPanel::DrawFloatControl("Size", size))
		{
			component.SetSize(size);
			changed = true;
		}
		return changed;
	});
}

#endif
#pragma endregion

#pragma region Serialization
#include "Engine/Core/Scene/SceneSerializer.h"
namespace Game
{
	class DirectionalLightSerializer : public Engine::ComponentSerializer
	{
		virtual void Serialize(Engine::Entity entity, YAML::Emitter& out) override
		{
			auto& dirLightComponent = *entity.GetComponent<DirectionalLightComponent>();
			out << YAML::Key << "Direction" << YAML::Value << dirLightComponent.GetDirectinalLight()->GetDirection();
			out << YAML::Key << "Temperature" << YAML::Value << dirLightComponent.GetDirectinalLight()->GetCCT();
			out << YAML::Key << "Color" << YAML::Value << dirLightComponent.GetDirectinalLight()->GetTint();
			out << YAML::Key << "Intensity" << YAML::Value << dirLightComponent.GetDirectinalLight()->GetIntensity();
			out << YAML::Key << "Size" << YAML::Value << dirLightComponent.GetDirectinalLight()->GetSize();
		}

		virtual void Deserialize(Engine::Entity entity, YAML::Node data) override
		{
			auto& dlc = *entity.GetComponent<DirectionalLightComponent>();
			dlc.SetDirection(data["Direction"].as<Math::Vector3>());
			dlc.SetIntensity(data["Intensity"].as<float>());
			dlc.SetTemperature(data["Temperature"].as<float>());
			dlc.SetTint(data["Color"].as<Math::Vector3>());
			dlc.SetSize(data["Size"].as<float>());
		}
	};
	ADD_COMPONENT_SERIALIZER(DirectionalLightComponent, DirectionalLightSerializer);
}
#pragma endregion
