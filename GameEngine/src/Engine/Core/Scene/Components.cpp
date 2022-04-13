#include "pch.h"
#include "Components.h"
#include "Entity.h"


// Transform Component
Engine::TransformComponent::TransformComponent(const Math::Vector3& position) :
	Position(position)
{}

Math::Mat4 Engine::TransformComponent::GetTransform() const
{
	Math::Mat4 rotation = glm::toMat4(Math::Quaternion(Rotation));

	return glm::translate(Math::Mat4(1.0f), Position) *
		rotation *
		glm::scale(Math::Mat4(1.0f), Scale);
}

void Engine::TransformComponent::AddChild(Entity child)
{
	TransformComponent& tc = child.GetComponent<TransformComponent>();
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
