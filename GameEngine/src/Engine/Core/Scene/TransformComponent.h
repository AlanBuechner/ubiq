#pragma once
#include "Components.h"

#include <Engine/Math/Math.h>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Engine/Renderer/Camera.h"
#include "SceneCamera.h"

namespace Engine
{
	CLASS(GROUP = Component, DontRemove) TransformComponent : public Component
	{
	public:
		REFLECTED_BODY(Engine::TransformComponent);

	public:
		using EachEntityFunc = std::function<void(const Math::Mat4&)>;

		TransformComponent() = default;
		TransformComponent(const TransformComponent& parentTransform) = default;
		TransformComponent(const Math::Vector3& position);

#define TRANSFORM_PROP(name, additiveName, v) \
		const Math::Vector3& Get##name() const { return v; }\
		FUNCTION() void Set##name(const Math::Vector3& val) { v = val; Dirty(); }\
		void Set##name##X(float val) { v.x = val; Dirty(); }\
		void Set##name##Y(float val) { v.y = val; Dirty(); }\
		void Set##name##Z(float val) { v.z = val; Dirty(); }\
		void additiveName(const Math::Vector3& val) { v += val; Dirty(); }

		TRANSFORM_PROP(Position, Translate, m_Position);
		TRANSFORM_PROP(Rotation, Rotate, m_Rotation);
		TRANSFORM_PROP(Scale, Scale, m_Scale);

#undef TRANSFORM_PROP

		Math::Mat4 GetTransform() const;
		Math::Mat4 GetGlobalTransform();
		void SetGlobalTransformFromMatrix(Math::Mat4 transform);

		void AddChild(Entity child);
		void RemoveChild(Entity child);
		const Utils::Vector<Entity>& GetChildren() const { return Children; }
		void SetParentToRoot();
		void SetParent(Entity parent);
		void SetParent(UUID uuid);

		Entity GetOwner() const { return Owner; }
		Entity GetParent() const { return Parent; }

		bool IsDirty() { return m_Dirty; }
		bool WasDirtyThisFrame() { return m_DirtyThisFrame; }

	public:

		void Dirty();
		void UpdateHierarchyGlobalTransform();
		FUNCTION()
		void UpdateTransformEvent();
		FUNCTION()
		void ClearDirtyFlag();

	private:
		PROPERTY(set = SetPosition) Math::Vector3 m_Position = { 0.0f, 0.0f, 0.0f };
		PROPERTY(set = SetRotation, Degrees) Math::Vector3 m_Rotation = { 0.0f, 0.0f, 0.0f };
		PROPERTY(set = SetScale) Math::Vector3 m_Scale = { 1.0f, 1.0f, 1.0f };

		Entity Parent = Entity::null;
		Utils::Vector<Entity> Children;

		bool m_Dirty = true;
		bool m_DirtyThisFrame = true;
		Math::Mat4 ChashedGloableTransform = Math::Mat4(1.0f);

		friend class Scene;
	};
}
