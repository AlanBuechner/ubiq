#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Engine/Renderer/Camera.h"
#include "SceneCamera.h"
#include "ScriptableEntity.h"

#include "Engine/Renderer/Texture.h"

namespace Engine
{
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