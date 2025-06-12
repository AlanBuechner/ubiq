#include "TransformComponent.h"

namespace Engine
{
	TransformComponent::TransformComponent(const Math::Vector3& position) :
		m_Position(position)
	{}

	Math::Mat4 TransformComponent::GetTransform() const
	{
		CREATE_PROFILE_FUNCTIONI();
		return Math::Translate(m_Position) *
			Math::Mat4Cast(Math::Quaternion(m_Rotation)) *
			Math::Scale(m_Scale);
	}

	Math::Mat4 TransformComponent::GetGlobalTransform()
	{
		CREATE_PROFILE_FUNCTIONI();
		UpdateHierarchyGlobalTransform();
		return ChashedGloableTransform;
	}

	void TransformComponent::SetGlobalTransformFromMatrix(Math::Mat4 transform)
	{
		CREATE_PROFILE_FUNCTIONI();
		Math::Mat4 localTranform = transform;
		if (GetParent())
		{
			Math::Mat4 parentTransform = GetParent().GetTransform().GetGlobalTransform();
			Math::Mat4 invParentTransform = Math::Inverse(parentTransform);
			localTranform = invParentTransform * transform;
		}

		Math::DecomposeTransform(localTranform, m_Position, m_Rotation, m_Scale);
		Dirty();
	}
	
	void TransformComponent::AddChild(Entity child)
	{
		CREATE_PROFILE_FUNCTIONI();
		TransformComponent& tc = child.GetTransform();
		if (tc.Owner == Owner)
			return;

		Entity p = Parent;
		while (p.GetScene() != nullptr)
		{
			if (p == tc.Owner)
				return;
			p = p.GetParent();
		}

		if (tc.Parent)
			tc.Parent.GetTransform().RemoveChild(child);

		tc.Parent = Owner;
		Children.Push(child);
		tc.Dirty();
	}

	void TransformComponent::RemoveChild(Entity child)
	{
		CREATE_PROFILE_FUNCTIONI();
		for (uint32_t i = 0; i < Children.Count(); i++)
		{
			if (Children[i] == child)
			{
				std::swap(Children[i], Children.Back());
				Children.Pop();
				return;
			}
		}
	}

	void TransformComponent::SetParentToRoot()
	{
		CREATE_PROFILE_FUNCTIONI();
		if (Parent)
		{
			Parent.GetTransform().RemoveChild(Owner);
			Parent = Entity::null;
		}
	}

	void TransformComponent::SetParent(Entity parent)
	{
		CREATE_PROFILE_FUNCTIONI();
		if (parent)
			parent.AddChild(GetOwner());
		else
			SetParentToRoot();
	}

	void TransformComponent::SetParent(UUID uuid)
	{
		CREATE_PROFILE_FUNCTIONI();
		if (uuid == 0)
			SetParentToRoot();
		else
			SetParent(GetOwner().GetScene()->GetEntityWithUUID(uuid));
	}

	void TransformComponent::Dirty()
	{
		CREATE_PROFILE_FUNCTIONI();
		m_Dirty = true;
		m_DirtyThisFrame = true;
		Owner.DirtyVolume();
		for (uint32 i = 0; i < Children.Count(); i++)
			Children[i].GetTransform().Dirty();
	}

	void TransformComponent::UpdateHierarchyGlobalTransform()
	{
		CREATE_PROFILE_FUNCTIONI();
		if (m_Dirty)
		{
			if (Parent)
			{
				TransformComponent& parentTransform = Parent.GetTransform();
				if (parentTransform.m_Dirty)
					parentTransform.UpdateHierarchyGlobalTransform();
				ChashedGloableTransform = parentTransform.GetGlobalTransform() * GetTransform();
			}
			else
				ChashedGloableTransform = GetTransform();

			m_Dirty = false;
		}
	}

	void TransformComponent::UpdateTransformEvent()
	{
		CREATE_PROFILE_FUNCTIONI();
		if (m_DirtyThisFrame)
		{
			UpdateHierarchyGlobalTransform();

			{
				CREATE_PROFILE_SCOPEI("Report transform change");
				// iterate over all components on entity
				Utils::Vector<Component*> components = Owner.GetComponents();
				for (Component* comp : components)
					comp->OnTransformChange(ChashedGloableTransform);
			}
		}
	}

	void TransformComponent::ClearDirtyFlag()
	{
		m_DirtyThisFrame = false;
	}

	// ------------------- Converter ------------------- //
	template<>
	struct Convert<TransformComponent>
	{
		CONVERTER_BASE(TransformComponent);
		static ObjectDescription Encode(const TransformComponent& val)
		{
			CREATE_PROFILE_FUNCTIONI();
			ObjectDescription desc(ObjectDescription::Type::Object);
			desc["Position"] = ObjectDescription::CreateFrom(val.GetPosition());
			desc["Rotation"] = ObjectDescription::CreateFrom(val.GetRotation());
			desc["Scale"] = ObjectDescription::CreateFrom(val.GetScale());

			desc["Parent"] = ObjectDescription::CreateFrom((uint64)val.GetParent().GetUUID());

			return desc;
		}

		static bool Decode(TransformComponent& tc, const ObjectDescription& data)
		{
			CREATE_PROFILE_FUNCTIONI();
			tc.SetPosition(data["Position"].Get<Math::Vector3>());
			tc.SetRotation(data["Rotation"].Get<Math::Vector3>());
			tc.SetScale(data["Scale"].Get<Math::Vector3>());

			tc.SetParent(data["Parent"].Get<uint64>());

			return true;
		}
	};
	ADD_OBJECT_CONVERTER(TransformComponent);
}
