#include "pch.h"
#include "SceneRendererComponents.h"

namespace Engine
{


	// camera component
	void CameraComponent::OnTransformChange(const Math::Mat4& transform)
	{
		Camera.SetTransform(transform);
	}

	// directional Light component
	void DirectionalLightComponent::OnComponentAdded()
	{
		if (s_Instance)
		{
			Owner.RemoveComponent<DirectionalLightComponent>();
			return;
		}

		s_Instance = this;

		m_Light = { {0,-1,0}, {1,1,1}, 1 };
		m_LightBuffer = ConstantBuffer::Create(sizeof(DirectionalLight));

		Owner.GetScene()->GetSceneRenderer()->SetDirectionalLight(m_LightBuffer);
	}

	void DirectionalLightComponent::OnInvalid()
	{
		if (m_Dirty)
		{
			m_LightBuffer->SetData(&m_Light);
			m_Dirty = false;
		}
	}

	void DirectionalLightComponent::OnComponentRemoved()
	{
		if (this == s_Instance)
			s_Instance = nullptr;
	}

	void DirectionalLightComponent::SetDirection(Math::Vector3 direction)
	{
		m_Light.direction = direction;
		m_Dirty = true;
	}

	void DirectionalLightComponent::SetColor(Math::Vector3 color)
	{
		m_Light.color = color;
		m_Dirty = true;
	}

	void DirectionalLightComponent::SetIntensity(float intensity)
	{
		m_Light.intensity = intensity;
		m_Dirty = true;
	}

	// mesh component
	void MeshRendererComponent::OnComponentRemoved()
	{
		// remove object from the scene
		if (Owner.GetScene() != nullptr)
			Owner.GetScene()->GetSceneRenderer()->RemoveObject(m_Object);
	}

	void MeshRendererComponent::OnTransformChange(const Math::Mat4& transform)
	{
		if (m_Object)
			m_Object->UpdateTransform(transform);
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
	SkyboxComponent* SkyboxComponent::s_Instance;
	void SkyboxComponent::OnComponentAdded()
	{
		if (s_Instance)
		{
			Owner.RemoveComponent<SkyboxComponent>();
			return;
		}

		s_Instance = this;
	}

	void SkyboxComponent::OnComponentRemoved()
	{
		if (this == s_Instance)
			s_Instance = nullptr;
	}

	void SkyboxComponent::SetSkyboxTexture(Ref<Texture2D> texture)
	{
		m_SkyboxTexture = texture;
		Owner.GetScene()->GetSceneRenderer()->SetSkyBox(m_SkyboxTexture);
	}

}

