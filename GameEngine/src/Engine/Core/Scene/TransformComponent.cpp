#include "TransformComponent.h"

namespace Engine
{
	TransformComponent::TransformComponent(const Math::Vector3& position) :
		m_Position(position)
	{}

	Math::Mat4 TransformComponent::GetTransform() const
	{
		return Math::Translate(m_Position) *
			Math::Mat4Cast(Math::Quaternion(m_Rotation)) *
			Math::Scale(m_Scale);
	}

	Math::Mat4 TransformComponent::GetGlobalTransform() const
	{
		return ChashedGloableTransform;
	}

	void TransformComponent::AddChild(Entity child)
	{
		TransformComponent& tc = child.GetTransform();
		if (tc.Parent != Owner)
		{
			if (tc.Parent)
				tc.Parent.GetTransform().RemoveChild(child);

			tc.Parent = Owner;
			Children.push_back(child);
			tc.Dirty();
		}
	}

	void TransformComponent::RemoveChild(Entity child)
	{
		for (uint32_t i = 0; i < Children.size(); i++)
		{
			if (Children[i] == child)
			{
				std::swap(Children[i], Children.back());
				Children.pop_back();
				return;
			}
		}
	}

	void TransformComponent::SetParentToRoot()
	{
		if (Parent)
		{
			Parent.GetTransform().RemoveChild(Owner);
			Parent = Entity::null;
		}
	}

	void TransformComponent::Dirty()
	{
		m_Dirty = true;
		for (uint32 i = 0; i < Children.size(); i++)
			Children[i].GetTransform().Dirty();
	}

	void TransformComponent::UpdateHierarchyGlobalTransform()
	{
		if (m_Dirty)
		{
			if (Parent)
			{
				TransformComponent& parentTransform = Parent.GetTransform();
				if (parentTransform.m_Dirty)
					parentTransform.UpdateHierarchyGlobalTransform();
				ChashedGloableTransform = GetTransform() * parentTransform.GetTransform();
			}
			else
				ChashedGloableTransform = GetTransform();

			// iterate over all components on entity
			std::vector<Component*> components = Owner.GetComponents();
			for (Component* comp : components)
				comp->OnTransformChange(ChashedGloableTransform);

			m_Dirty = false;
		}
	}
}
