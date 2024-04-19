#include "pch.h"
#include "StaticModelRendererComponent.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Core/Scene/TransformComponent.h"


namespace Engine
{

	// mesh component
	void StaticModelRendererComponent::OnComponentRemoved()
	{
		for (uint32 i = 0; i < m_CollapsedModelTree.size(); i++)
		{
			MeshEntry& entry = m_CollapsedModelTree[i];
			Owner.GetScene()->GetSceneRenderer()->RemoveObject(entry.m_Object);
		}
	}

	void StaticModelRendererComponent::OnTransformChange(const Math::Mat4& transform)
	{
		for (uint32 i = 0; i < m_CollapsedModelTree.size(); i++)
		{
			MeshEntry& entry = m_CollapsedModelTree[i];
			if(entry.m_Object)
				entry.m_Object->UpdateTransform(transform);
		}
	}

	void StaticModelRendererComponent::SetModel(Ref<Model> model)
	{
		m_Model = model;

		// remove old model
		for (uint32 i = 0; i < m_CollapsedModelTree.size(); i++)
			Owner.GetScene()->GetSceneRenderer()->RemoveObject(m_CollapsedModelTree[i].m_Object);
		m_CollapsedModelTree.clear();

		if (model)
		{
			for (uint32 i = 0; i < model->GetBakedMeshes().size(); i++)
			{
				m_CollapsedModelTree.push_back({
					model->GetNames()[i],
					model->GetBakedMeshes()[i],
				});
			}
		}
	}

	void StaticModelRendererComponent::Invalidate()
	{
		for (uint32 i = 0; i < m_CollapsedModelTree.size(); i++)
		{
			MeshEntry& entry = m_CollapsedModelTree[i];
			Ref<Material> mat = Renderer::GetDefultMaterial();
			if (entry.m_Material) mat = entry.m_Material;

			if (mat)
			{
				Owner.GetScene()->GetSceneRenderer()->RemoveObject(entry.m_Object);
				entry.m_Object = Owner.GetScene()->GetSceneRenderer()->Submit(entry.m_Mesh, mat, Owner.GetTransform().GetTransform());
			}
		}
	}
}
