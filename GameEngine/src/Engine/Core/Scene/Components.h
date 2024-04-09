#pragma once
#include <Engine/Math/Math.h>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Engine/Renderer/Camera.h"
#include "SceneCamera.h"

#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Abstractions/Resources/Texture.h"
#include "Engine/Renderer/Abstractions/Resources/Buffer.h"
#include "Engine/Renderer/Abstractions/Shader.h"
#include "Engine/Renderer/Material.h"

#include "Engine/Core/MeshBuilder.h"

#include "Engine/Core/UUID.h"

#include "Engine/Core/UUID.h"
#include "Entity.h"

#include <Reflection.h>

namespace Engine
{
	class Component
	{
	public:
		Entity Owner;

		virtual void OnComponentAdded() {};
		virtual void OnComponentRemoved() {};

		virtual void OnTransformChange(const Math::Mat4& transform) {};
		virtual void OnInvalid() {};
	};
	
	CLASS(GROUP = Component) TransformComponent : public Component
	{
	public:
		using Func = std::function<void(const Math::Mat4&)>;

		TransformComponent() = default;
		TransformComponent(const TransformComponent& parentTransform) = default;
		TransformComponent(const Math::Vector3& position);

#define TRANSFORM_PROP(name, additiveName, v) \
		const Math::Vector3& Get##name() const { return v; }\
		void Set##name(const Math::Vector3& val) { v = val; Dirty(); }\
		void Set##name##X(float val) { v.x = val; Dirty(); }\
		void Set##name##Y(float val) { v.y = val; Dirty(); }\
		void Set##name##Z(float val) { v.z = val; Dirty(); }\
		void additiveName(const Math::Vector3& val) { v += val; Dirty(); }

		TRANSFORM_PROP(Position, Translate, m_Position);
		TRANSFORM_PROP(Rotation, Rotate, m_Rotation);
		TRANSFORM_PROP(Scale, Scale, m_Scale);

#undef TRANSFORM_PROP

		Math::Mat4 GetTransform() const;
		Math::Mat4 GetGlobalTransform() const;

		void AddChild(Entity child);
		void RemoveChild(Entity child);
		const std::vector<Entity>& GetChildren() const { return Children; }
		void SetParentToRoot();

		Entity GetOwner() { return Owner; }
		Entity GetParent() { return Parent; }

		bool IsDirty() { return m_Dirty; }

	private:
		void Dirty();
		void UpdateHierarchyGlobalTransform();

	private:
		PROPERTY() Math::Vector3 m_Position = { 0.0f, 0.0f, 0.0f };
		PROPERTY() Math::Vector3 m_Rotation = { 0.0f, 0.0f, 0.0f };
		PROPERTY() Math::Vector3 m_Scale = { 1.0f, 1.0f, 1.0f };

		Entity Parent;
		std::vector<Entity> Children;

		bool m_Dirty = true;
		Math::Mat4 ChashedGloableTransform = Math::Mat4(1.0f);

		friend class Scene;
		friend class SceneSerializer;
	};

}
