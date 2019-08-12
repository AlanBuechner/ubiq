#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Engine
{
	class Camera
	{
	protected:
		typedef Engine::Camera Super;

	public:
		Camera(glm::mat4& projectionMatrix);

		const glm::vec3& GetPosition() const { return m_Position; }
		const glm::quat& GetRotation() const { return m_Rotation; }

		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }
		void SetRotation(const glm::quat& rotation) { m_Rotation = rotation;  RecalculateViewMatrix();  RecalculateDirectionVectors(); }

		void Translate(const glm::vec3& translation) { m_Position += translation; RecalculateViewMatrix(); }
		void Rotate(const glm::quat& rotation) { m_Rotation += rotation; RecalculateViewMatrix();  RecalculateDirectionVectors(); }

		const glm::mat4 GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4 GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4 GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

		inline const glm::vec3 ForwordVector() const { return m_ForwordVector; }
		inline const glm::vec3 UpVector() const { return m_UpVector; }
		inline const glm::vec3 RightVector() const { return m_RightVector; }

	private:
		void RecalculateViewMatrix();
		void RecalculateDirectionVectors();

	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;
		glm::mat4 m_RotationMatrix;

		glm::vec3 m_ForwordVector	= { 0.0f, 0.0f, 1.0f };
		glm::vec3 m_UpVector		= { 0.0f, 1.0f, 0.0f };
		glm::vec3 m_RightVector		= { 1.0f, 0.0f, 0.0f };

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::quat m_Rotation = { 0.0f, 1.0f, 0.0f, 0.0f };
	};
}