#include "pch.h"
#include "EditorCamera.h"

#include "Engine/Core/Input/Input.h"
#include "Engine/Core/Input/KeyCodes.h"
#include "Engine/Core/Time.h"

#include <glfw/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Engine
{

	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
		: m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip), Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip))
	{
		UpdateView();
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorCamera::UpdateView()
	{
		// m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
		m_Position = CalculatePosition();

		glm::quat orientation = GetOrientation();
		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = (0.0366f * (x * x) - 0.1778f * x + 0.3021f) * 30;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = (0.0366f * (y * y) - 0.1778f * y + 0.3021f) * 30;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.8f * 30;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f * 30;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

	void EditorCamera::OnUpdate()
	{
		const float moveSpeed = 2.0f;
		const float runSpeed = 4.0f;
		float speed = moveSpeed;

		const glm::vec2& mouse{ Input::GetMousePosition().x, Input::GetMousePosition().y };
		glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
		m_InitialMousePosition = mouse;

		bool alt = Input::GetKeyDown(KeyCode::LEFT_ALT) || Input::GetKeyDown(KeyCode::RIGHT_ALT);
		bool shift = Input::GetKeyDown(KeyCode::LEFT_SHIFT) || Input::GetKeyDown(KeyCode::RIGHT_SHIFT);

		bool rMouse = Input::GetMouseButtonDown(KeyCode::RIGHT_MOUSE);
		bool lMouse = Input::GetMouseButtonDown(KeyCode::LEFT_MOUSE);
		bool mMouse = Input::GetMouseButtonDown(KeyCode::MIDDLE_MOUSE);

		bool wKey = Input::GetKeyDown(KeyCode::W);
		bool sKey = Input::GetKeyDown(KeyCode::S);
		bool aKey = Input::GetKeyDown(KeyCode::A);
		bool dKey = Input::GetKeyDown(KeyCode::D);

		if (rMouse)
			if (alt)
				MouseRotateAboutFocal(delta);
			else
				MouseRotate(delta);
		else if (mMouse)
			MousePan(delta);
		else if (lMouse && alt)
			MouseZoom(delta.y);

		if (shift)
			speed = runSpeed;

		if (rMouse && !alt) {
			if (wKey)
				MoveFB(speed);
			if (sKey)
				MoveFB(-speed);

			if (aKey)
				MoveRL(-speed);
			if (dKey)
				MoveRL(speed);
		}


		UpdateView();
	}

	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(&EditorCamera::OnMouseScroll));
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		float delta = e.GetYOffset() * 0.1f;
		MouseZoom(delta);
		UpdateView();
		return false;
	}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance * Time::GetDeltaTime();
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance * Time::GetDeltaTime();
	}

	void EditorCamera::MouseRotateAboutFocal(const glm::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed() * Time::GetDeltaTime();
		m_Pitch += delta.y * RotationSpeed() * Time::GetDeltaTime();
	}

	void EditorCamera::MouseRotate(const glm::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed() * Time::GetDeltaTime();
		m_Pitch += delta.y * RotationSpeed() * Time::GetDeltaTime();
		m_FocalPoint = CalculateFocal();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed() * Time::GetDeltaTime();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
	}

	void EditorCamera::MoveFB(float speed)
	{
		m_FocalPoint += GetForwardDirection() * speed * Time::GetDeltaTime();
	}

	void EditorCamera::MoveRL(float speed)
	{
		m_FocalPoint += GetRightDirection() * speed * Time::GetDeltaTime();
	}

	glm::vec3 EditorCamera::GetUpDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetRightDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetForwardDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	glm::vec3 EditorCamera::CalculateFocal() const
	{
		return m_Position + GetForwardDirection() * m_Distance;
	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}
}