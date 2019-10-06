#include "pch.h"
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Engine
{
	Camera::Camera(glm::mat4& projectionMatrix)
		: m_ProjectionMatrix(projectionMatrix), m_ViewMatrix(1.0f)
	{
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
		RecalculateDirectionVectors();
	}

	void Camera::SetProjectionMatrix(glm::mat4& projectionMatrix)
	{
		m_ProjectionMatrix = projectionMatrix;
		m_ViewMatrix = glm::mat4(1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
		RecalculateDirectionVectors();
	}

	void Camera::RecalculateViewMatrix()
	{
		m_RotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.w), glm::vec3(m_Rotation.x, m_Rotation.y, m_Rotation.z));
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) * m_RotationMatrix;

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void Camera::RecalculateDirectionVectors()
	{
		glm::mat3 rotation = m_RotationMatrix;

		m_ForwordVector =	glm::vec3({ 0.0f, 0.0f, 1.0f }) * rotation;
		m_UpVector =		glm::vec3({ 0.0f, 1.0f, 0.0f }) * rotation;
		m_RightVector =		glm::vec3({ 1.0f, 0.0f, 0.0f }) * rotation;
	}
}