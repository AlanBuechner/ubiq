#include "pch.h"
#include "SceneRendererComponents.h"

namespace Engine
{


	// camera component
	void CameraComponent::OnTransformChange(const Math::Mat4& transform)
	{
		Camera->SetTransform(transform);
	}

	// directional Light component
	void DirectionalLightComponent::OnComponentAdded()
	{
		m_Light = CreateRef<DirectionalLight>(Math::Vector3{ 0,-1,0 }, Math::Vector3{ 1,1,1 }, 1 );

		Owner.GetScene()->GetSceneRenderer()->SetDirectionalLight(m_Light);
	}

	void DirectionalLightComponent::OnInvalid()
	{
		if (m_Dirty)
		{
			m_Light->Apply();
			m_Dirty = false;
		}
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
		m_Light->UpdateShadowMaps();
	}

	// mesh component
	void MeshRendererComponent::OnComponentRemoved()
	{

	}

	void MeshRendererComponent::OnTransformChange(const Math::Mat4& transform)
	{
		if (m_Object)
		{
			m_Object->UpdateTransform(transform);
		}
	}

	void MeshRendererComponent::Invalidate()
	{
		if (m_Mesh && m_Mat)
		{
			Owner.GetScene()->GetSceneRenderer()->RemoveObject(m_Object); // remove the object if it exists
			m_Object = Owner.GetScene()->GetSceneRenderer()->Submit(m_Mesh, m_Mat, Owner.GetTransform().GetTransform()); // re add it with the new shader and mesh
		}
	}

	// skybox component
	void SkyboxComponent::OnComponentAdded()
	{

	}

	void SkyboxComponent::OnComponentRemoved()
	{

	}

	void SkyboxComponent::SetSkyboxTexture(Ref<Texture2D> texture)
	{
		m_SkyboxTexture = texture;
		Owner.GetScene()->GetSceneRenderer()->SetSkyBox(m_SkyboxTexture);
	}

}

