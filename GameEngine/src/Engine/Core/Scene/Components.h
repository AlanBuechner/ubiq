#pragma once
#include <glm/glm.hpp>
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

namespace Engine
{
	class ScriptableEntity;
}

namespace Engine
{
	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};


	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) :
			Tag(tag)
		{}
	};

	struct TransformComponent
	{
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };


		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& position) :
			Position(position)
		{}

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(glm::mat4(1.0f), Position) * 
				rotation * 
				glm::scale(glm::mat4(1.0f), Scale);
				
		}
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f,1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color) :
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

			Ref<VertexBuffer> vbo = Engine::VertexBuffer::Create((float*)mesh->vertices.data(), mesh->vertices.size() * sizeof(Mesh::Vertex));

			Engine::BufferLayout layout = {
				{Engine::ShaderDataType::Float3, "a_Position"},
				{Engine::ShaderDataType::Float2, "a_TexCoord"},
				{Engine::ShaderDataType::Float3, "a_Normal"},
				{Engine::ShaderDataType::Float3, "a_Tangent"}
			};

			vbo->SetLayout(layout);

			vao->AddVertexBuffer(vbo);

			Ref<IndexBuffer> ibo = Engine::IndexBuffer::Create(mesh->indices.data(), mesh->indices.size());

			vao->SetIndexBuffer(ibo);

			mat = std::make_shared<Material>();
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
