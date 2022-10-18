#include "pch.h"
#include "Components.h"
#include "Entity.h"


// Transform Component
Engine::TransformComponent::TransformComponent(const Math::Vector3& position) :
	m_Position(position)
{}

Math::Mat4 Engine::TransformComponent::GetTransform() const
{
	Math::Mat4 rotation = glm::toMat4(Math::Quaternion(m_Rotation));

	return glm::translate(Math::Mat4(1.0f), m_Position) *
		rotation *
		glm::scale(Math::Mat4(1.0f), m_Scale);
}

Math::Mat4 Engine::TransformComponent::GetGlobalTransform() const
{
	return ChashedGloableTransform;
}

void Engine::TransformComponent::AddChild(Entity child)
{
	TransformComponent& tc = child.GetTransform();
	if (tc.Parent != Owner)
	{
		if (tc.Parent)
			tc.Parent.GetTransform().RemoveChild(child);

		tc.Parent = Owner;
		Children.push_back(child);
	}
}

void Engine::TransformComponent::RemoveChild(Entity child)
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

void Engine::TransformComponent::SetParentToRoot()
{
	if (Parent)
	{
		Parent.GetTransform().RemoveChild(Owner);
		Parent = Entity::null;
	}
}

void Engine::TransformComponent::RemoveMoveCallback(Func func)
{
	for (uint32 i = 0; i < m_ChangeCallbacks.size(); i++)
	{
		using T = void(*)(const Math::Mat4&);
		if (m_ChangeCallbacks[i].target<T>() == func.target<T>())
		{
			std::swap(m_ChangeCallbacks[i], m_ChangeCallbacks.back());
			m_ChangeCallbacks.pop_back();
			return;
		}
	}
}

void Engine::TransformComponent::Dirty()
{
	m_Dirty = true;
	for (uint32 i = 0; i < Children.size(); i++)
		Children[i].GetTransform().Dirty();
}

void Engine::TransformComponent::UpdateHierarchyGlobalTransform()
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

		for (uint32 i = 0; i < m_ChangeCallbacks.size(); i++)
			m_ChangeCallbacks[i](ChashedGloableTransform);

		m_Dirty = false;
	}
}
