#pragma once
#include <Engine/Math/Math.h>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Engine/Renderer/Camera.h"
#include "SceneCamera.h"

#include "Engine/Renderer/Texture.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Renderer/VertexArray.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Material.h"

#include "Engine/Core/Mesh.h"

#include "Engine/Core/UUID.h"

#include "Engine/Core/UUID.h"
#include "Entity.h"

namespace Engine
{
	class ScriptableEntity;
}

namespace Engine
{
	struct EntityDataComponent
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

	struct TransformComponent
	{
	public:
		Math::Vector3 Position = { 0.0f, 0.0f, 0.0f };
		Math::Vector3 Rotation = { 0.0f, 0.0f, 0.0f };
		Math::Vector3 Scale = { 1.0f, 1.0f, 1.0f };

	private:
		Entity Owner;

		Entity Parent;
		std::vector<Entity> Children;

		Math::Mat4 ChashedGloableTransform = Math::Mat4(1.0f);

	public:
		TransformComponent() = default;
		TransformComponent(const TransformComponent& parentTransform) = default;
		TransformComponent(const Math::Vector3& position);

		Math::Mat4 GetTransform() const;
		Math::Mat4 GetGlobalTransform() const;

		void AddChild(Entity child);
		void RemoveChild(Entity child);
		const std::vector<Entity>& GetChildren() const { return Children; }
		void SetParentToRoot();

		Entity GetOwner() { return Owner; }
		Entity GetParent() { return Parent; }

	private:
		void UpdateHierarchyGlobalTransform(Math::Mat4 parentTransform);

		friend Scene;
		friend SceneSerializer;
	};

	struct SpriteRendererComponent
	{
		Math::Vector4 Color{ 1.0f,1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const Math::Vector4& color) :
			Color(color)
		{}
	};

	struct MeshRendererComponent
	{
		Ref<VertexArray> vao;
		Ref<Material> mat;

		MeshRendererComponent() = default;
		MeshRendererComponent(Ref<Mesh> mesh)
		{
			vao = Engine::VertexArray::Create();

			Ref<VertexBuffer> vbo = Engine::VertexBuffer::Create((float*)mesh->vertices.data(), (uint32)(mesh->vertices.size() * sizeof(Mesh::Vertex)));

			Engine::BufferLayout layout = {
				{Engine::ShaderDataType::Float3, "a_Position"},
				{Engine::ShaderDataType::Float2, "a_TexCoord"},
				{Engine::ShaderDataType::Float3, "a_Normal"},
				{Engine::ShaderDataType::Float3, "a_Tangent"}
			};

			vbo->SetLayout(layout);

			vao->AddVertexBuffer(vbo);

			Ref<IndexBuffer> ibo = Engine::IndexBuffer::Create(mesh->indices.data(), (uint32)mesh->indices.size());

			vao->SetIndexBuffer(ibo);

			mat = CreateRef<Material>();
		}
	};

	struct CameraComponent
	{
		Engine::SceneCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct NativeScriptComponent
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
