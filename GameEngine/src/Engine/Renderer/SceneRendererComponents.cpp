#include "pch.h"
#include "SceneRendererComponents.h"

namespace Engine
{


	// camera component
	void CameraComponent::OnTransformChange(const Math::Mat4& transform)
	{
		Camera.SetTransform(transform);
	}

	// mesh component
	MeshRendererComponent::~MeshRendererComponent()
	{
		// remove callback
		if (Owner)
		{
			// remove object from the scene
			if (Owner.GetScene() != nullptr)
				Owner.GetScene()->GetSceneRenderer()->RemoveObject(m_Object);
		}
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

}

