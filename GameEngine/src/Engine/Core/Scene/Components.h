#pragma once
#include <Engine/Math/Math.h>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Engine/Renderer/Camera.h"
#include "SceneCamera.h"

#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Material.h"

#include "Engine/Core/MeshBuilder.h"

#include "Engine/Core/UUID.h"

#include "Engine/Core/UUID.h"
#include "Entity.h"

namespace Engine
{
	class ScriptableEntity;
}

namespace Engine
{
	struct Component 
	{
		Entity Owner;

		virtual void OnComponentAdded() {};
		virtual void OnComponentRemoved() {};
	};

	struct EntityDataComponent : Component
	{
	public:
		std::string Name;

	private:
		UUID ID;

	public:
		EntityDataComponent() = default;
		EntityDataComponent(const EntityDataComponent&) = default;
		EntityDataComponent(const std::string& name) :
			Name(name), ID(0)
		{}
		EntityDataComponent(const std::string& name, UUID id) :
			Name(name), ID(id)
		{}

		UUID GetID() { return ID; }
	};

	struct TransformComponent : public Component
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

		void AddMoveCallback(Func func) { m_ChangeCallbacks.push_back(func); }
		void RemoveMoveCallback(Func func);

	private:
		void Dirty();
		void UpdateHierarchyGlobalTransform();

	private:
		Math::Vector3 m_Position = { 0.0f, 0.0f, 0.0f };
		Math::Vector3 m_Rotation = { 0.0f, 0.0f, 0.0f };
		Math::Vector3 m_Scale = { 1.0f, 1.0f, 1.0f };

		Entity Parent;
		std::vector<Entity> Children;

		bool m_Dirty = true;
		Math::Mat4 ChashedGloableTransform = Math::Mat4(1.0f);

		std::vector<Func> m_ChangeCallbacks;

		friend Scene;
		friend SceneSerializer;
	};

	struct SpriteRendererComponent : public Component
	{
		Math::Vector4 Color{ 1.0f,1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const Math::Vector4& color) :
			Color(color)
		{}
	};

	struct CameraComponent : public Component
	{
		Engine::SceneCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct NativeScriptComponent : public Component
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity*(*InstantiateScript)();
		void(*DestroyScript)(NativeScriptComponent*);

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

}
