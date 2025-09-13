#include "DirectionalLightComponent.h"
#include "RenderingUtils/Lighting/DirectionalLight.h"
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
		// get new cameras
		Utils::Vector<Engine::Ref<Engine::Camera>> newCameras = Owner.GetScene()->GetSceneRenderer()->GetCameras();
		Utils::Vector<Engine::Ref<Engine::Camera>> oldCameras = GetDirectinalLight()->GetCameras();

		// remove old cameras
		for (Engine::Ref<Engine::Camera> camera : oldCameras)
		{
			if (!newCameras.Contains(camera))
				GetDirectinalLight()->RemoveCamera(camera);
		}

		// add new cameras
		for (Engine::Ref<Engine::Camera> camera : newCameras)
		{
			if (!oldCameras.Contains(camera))
				GetDirectinalLight()->AddCamera(camera);
		}

		// update shadow maps
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


#include "Engine/Core/ObjectDescription/ObjectDescription.h"

namespace Engine
{
	template<>
	struct Convert<Game::DirectionalLightComponent>
	{
		CONVERTER_BASE(Game::DirectionalLightComponent);
		static ObjectDescription Encode(const Game::DirectionalLightComponent& val)
		{
			ObjectDescription desc(ObjectDescription::Type::Object);
			desc["Direction"] = ObjectDescription::CreateFrom(val.GetDirectinalLight()->GetDirection());
			desc["Temperature"] = ObjectDescription::CreateFrom(val.GetDirectinalLight()->GetCCT());
			desc["Color"] = ObjectDescription::CreateFrom(val.GetDirectinalLight()->GetTint());
			desc["Intensity"] = ObjectDescription::CreateFrom(val.GetDirectinalLight()->GetIntensity());
			desc["Size"] = ObjectDescription::CreateFrom(val.GetDirectinalLight()->GetSize());

			return desc;
		}

		static bool Decode(Game::DirectionalLightComponent& dlc, const ObjectDescription& data)
		{
			dlc.SetDirection(data["Direction"].Get<Math::Vector3>());
			dlc.SetIntensity(data["Intensity"].Get<float>());
			dlc.SetTemperature(data["Temperature"].Get<float>());
			dlc.SetTint(data["Color"].Get<Math::Vector3>());
			dlc.SetSize(data["Size"].Get<float>());

			return true;
		}
	};
	ADD_OBJECT_CONVERTER(Game::DirectionalLightComponent);
}
#pragma endregion
