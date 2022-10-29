#include "pch.h"
#include "SceneRendererComponents.h"

namespace Engine
{


	// camera component
	void CameraComponent::ObjectMovecallback(const Math::Mat4& transform)
	{
		Camera.SetTransform(transform);
	}

	CameraComponent::CameraComponent(CameraComponent&& other)
	{
		Owner = other.Owner;
		//Owner.GetTransform().AddMoveCallback(BIND_EVENT_FN_EXTERN(&CameraComponent::ObjectMovecallback, this));

		other.Owner = Entity{};
	}

	CameraComponent& CameraComponent::operator=(CameraComponent&& other)
	{
		return *this;
	}

	CameraComponent::~CameraComponent()
	{
		if (Owner)
		{
			//Owner.GetTransform().RemoveMoveCallback(BIND_EVENT_FN_EXTERN(&CameraComponent::ObjectMovecallback, this));
		}
	}

	void CameraComponent::OnComponentAdded()
	{
		Owner.GetTransform().AddMoveCallback(BIND_EVENT_FN_EXTERN(&CameraComponent::ObjectMovecallback, this));
	}


	// mesh component
	MeshRendererComponent::MeshRendererComponent(MeshRendererComponent&& other)
	{
		Owner = other.Owner;
		m_Mesh = other.m_Mesh;
		m_Mat = other.m_Mat;
		m_Object = other.m_Object;

		// add function call back for new object
		Owner.GetTransform().AddMoveCallback(BIND_EVENT_FN_EXTERN(&MeshRendererComponent::ObjectMovedCallback, this));

		other.Owner = Entity{}; // empty entity
		other.m_Mesh = nullptr;
		other.m_Mat = nullptr;
		other.m_Object = nullptr;
	}

	MeshRendererComponent& MeshRendererComponent::operator=(MeshRendererComponent&& other)
	{
		return *this;
	}

	MeshRendererComponent::~MeshRendererComponent()
	{
		// remove callback
		if (Owner)
		{
			if (Owner.HasComponent<TransformComponent>())
				Owner.GetTransform().RemoveMoveCallback(BIND_EVENT_FN_EXTERN(&MeshRendererComponent::ObjectMovedCallback, this));

			// remove object from the scene
			if (Owner.GetScene() != nullptr)
				Owner.GetScene()->GetSceneRenderer()->RemoveObject(m_Object);
		}
	}

	void MeshRendererComponent::OnComponentAdded()
	{
		Owner.GetTransform().AddMoveCallback(BIND_EVENT_FN_EXTERN(&MeshRendererComponent::ObjectMovedCallback, this));
	}

	void MeshRendererComponent::Invalidate()
	{
		if (m_Mesh && m_Mat)
		{
			Owner.GetScene()->GetSceneRenderer()->RemoveObject(m_Object); // remove the object if it exists
			m_Object = Owner.GetScene()->GetSceneRenderer()->Submit(m_Mesh, m_Mat, Owner.GetTransform().GetTransform()); // re add it with the new shader and mesh
		}
	}

	void MeshRendererComponent::ObjectMovedCallback(const Math::Mat4& transform)
	{
		if(m_Object)
			m_Object->UpdateTransform(transform);
	}

}

