#include "pch.h"
#include "EditorCamera.h"

#include "Engine/Core/Input/Input.h"
#include "Engine/Core/Input/KeyCodes.h"
#include "Engine/Core/Time.h"

#include "Engine/Events/Event.h"
#include "Engine/Events/MouseEvent.h"

#include "Engine/Math/Math.h"

namespace Engine
{

	EditorCamera::EditorCamera() :
		EditorCamera(45.0f, 1.778f, 0.1f, 10000.0f)
	{}

	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip):
		m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip), 
		Camera(Math::Perspective(Math::Radians(fov), aspectRatio, nearClip, farClip))
	{
		UpdateView();
		OnUpdate();
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_CameraData.ProjectionMatrix = Math::Perspective(Math::Radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
		m_CameraData.VPMatrix = m_CameraData.ProjectionMatrix * m_CameraData.ViewMatrix;
	}

	void EditorCamera::UpdateView()
	{
		m_Position = CalculatePosition();
		m_CameraData.Position = m_Position;

		Math::Quaternion orientation = GetOrientation();
		m_CameraData.ViewMatrix = Math::Translate(m_Position) * Math::Mat4Cast(orientation);
		m_CameraData.ViewMatrix = Math::Inverse(m_CameraData.ViewMatrix);
		m_CameraData.VPMatrix = m_CameraData.ProjectionMatrix * m_CameraData.ViewMatrix;
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
		return 0.3f * 30;
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
		const float moveSpeed = 4.0f;
		const float runSpeed = 8.0f;
		const float rotSpeed = 0.3f;
		float speed = moveSpeed;

		const Math::Vector2& mouse{ Input::GetMousePosition().x, Input::GetMousePosition().y };
		Math::Vector2 delta = (mouse - m_InitialMousePosition) * rotSpeed;
		m_InitialMousePosition = mouse;

		bool alt = Input::GetKeyDown(KeyCode::ALT);
		bool shift = Input::GetKeyDown(KeyCode::SHIFT);

		bool rMouse = Input::GetMouseButtonDown(KeyCode::RIGHT_MOUSE);
		bool lMouse = Input::GetMouseButtonDown(KeyCode::LEFT_MOUSE);
		bool mMouse = Input::GetMouseButtonDown(KeyCode::MIDDLE_MOUSE);

		bool wKey = Input::GetKeyDown(KeyCode::W);
		bool aKey = Input::GetKeyDown(KeyCode::A);
		bool sKey = Input::GetKeyDown(KeyCode::S);
		bool dKey = Input::GetKeyDown(KeyCode::D);

		if (rMouse)
		{
			if (alt)	MouseRotateAboutFocal(delta);
			else		MouseRotate(delta);
		}
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

		UpdateCameraBuffer();
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

	void EditorCamera::MousePan(const Math::Vector2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance * Time::GetDeltaTime();
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance * Time::GetDeltaTime();
	}

	void EditorCamera::MouseRotateAboutFocal(const Math::Vector2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed() * Time::GetDeltaTime();
		m_Pitch += delta.y * RotationSpeed() * Time::GetDeltaTime();
	}

	void EditorCamera::MouseRotate(const Math::Vector2& delta)
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

	Math::Vector3 EditorCamera::GetUpDirection() const
	{
		return GetOrientation() * Math::Vector3(0.0f, 1.0f, 0.0f);
	}

	Math::Vector3 EditorCamera::GetRightDirection() const
	{
		return GetOrientation() * Math::Vector3(1.0f, 0.0f, 0.0f);
	}

	Math::Vector3 EditorCamera::GetForwardDirection() const
	{
		return GetOrientation() * Math::Vector3(0.0f, 0.0f, -1.0f);
	}

	Math::Vector3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	Math::Vector3 EditorCamera::CalculateFocal() const
	{
		return m_Position + GetForwardDirection() * m_Distance;
	}

	Math::Quaternion EditorCamera::GetOrientation() const
	{
		return Math::Quaternion(Math::Vector3(-m_Pitch, -m_Yaw, 0.0f));
	}
}
