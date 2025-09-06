#include "pch.h"
#include "EditorCamera.h"

#include "Engine/Core/Input/Input.h"
#include "Engine/Core/Input/KeyCodes.h"
#include "Utils/Time.h"

#include "Engine/Events/Event.h"
#include "Engine/Events/MouseEvent.h"

#include "Engine/Math/Math.h"

#include "Engine/Core/Cursor.h"

namespace Editor
{

	EditorCamera::EditorCamera() :
		EditorCamera(45.0f, 1.778f, 0.1f, 10000.0f)
	{
		OnUpdate();
	}

	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip):
		m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip), 
		Engine::Camera(Math::Perspective(Math::Radians(fov), aspectRatio, nearClip, farClip))
	{
		OnUpdate();
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_CameraData.ProjectionMatrix = Math::Perspective(Math::Radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
		m_CameraData.InvProjection = Math::Inverse(m_CameraData.ProjectionMatrix);
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

		return { xFactor * m_Distance, yFactor * m_Distance };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.3f * 30;
	}

	float EditorCamera::ZoomSpeed() const
	{
		return 30;
	}

	void EditorCamera::CalcDist()
	{
		m_Distance = Math::Pow(1.1f, m_Zoom);
	}

	void EditorCamera::OnUpdate()
	{
		CalcDist();
		UpdateView();
		UpdateCameraBuffer();
	}

	void EditorCamera::OnEvent(Engine::Event* e)
	{
		Engine::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Engine::MouseScrolledEvent>(BIND_EVENT_FN(&EditorCamera::OnMouseScroll));
		dispatcher.Dispatch<Engine::MouseMovedEvent>(BIND_EVENT_FN(&EditorCamera::OnMouseDelta));
		dispatcher.Dispatch<Engine::KeyDownEvent>(BIND_EVENT_FN(&EditorCamera::OnKeyDown));
	}

	bool EditorCamera::OnMouseScroll(Engine::MouseScrolledEvent* e)
	{
		if (Input::GetMouseButtonDown(Engine::MouseCode::RIGHT_MOUSE))
		{
			m_SpeedMultiplyer += e->GetYOffset() * 0.001f * m_SpeedMultiplyer;
			m_SpeedMultiplyer = Math::Max(0.00001f, m_SpeedMultiplyer);
		}
		else
		{
			float delta = e->GetYOffset() * 0.1f;
			MouseZoom(delta);
		}
		return true;
	}

	bool EditorCamera::OnMouseDelta(Engine::MouseMovedEvent* e)
	{
		bool ctrl = Input::GetKeyDown(Engine::KeyCode::CONTROL);
		bool alt = Input::GetKeyDown(Engine::KeyCode::ALT);
		bool shift = Input::GetKeyDown(Engine::KeyCode::SHIFT);

		bool rMouse = Input::GetMouseButtonDown(Engine::MouseCode::RIGHT_MOUSE);
		bool lMouse = Input::GetMouseButtonDown(Engine::MouseCode::LEFT_MOUSE);
		bool mMouse = Input::GetMouseButtonDown(Engine::MouseCode::MIDDLE_MOUSE);

		const float rotSpeed = 0.3f;
		const float panSpeed = 0.08f;
		Math::Vector2 delta = Math::Vector2{ e->GetDeltaX(), e->GetDeltaY() };

		if (mMouse)
		{
			if (shift) MousePan(delta * panSpeed);
			else if (ctrl) MouseZoom(delta.y);
			else MouseRotateAboutFocal(delta * rotSpeed);
			return true;
		}

		return false;
	}

	bool EditorCamera::OnKeyDown(Engine::KeyDownEvent* e)
	{
		bool alt = Input::GetKeyDown(Engine::KeyCode::ALT);
		bool shift = Input::GetKeyDown(Engine::KeyCode::SHIFT);
		bool rMouse = Input::GetMouseButtonDown(Engine::MouseCode::RIGHT_MOUSE);
		if (!(rMouse && !alt))
			return false;

		const float moveSpeed = 4.0f;
		const float runSpeed = 8.0f;
		const float rotSpeed = 0.3f;
		float speed = moveSpeed;

		if (shift)
			speed = runSpeed;
		speed *= m_SpeedMultiplyer;

		if(!alt)
		if (e->GetKeyCode() == Engine::KeyCode::W) MoveFB(speed);
		if (e->GetKeyCode() == Engine::KeyCode::S) MoveFB(-speed);
		if (e->GetKeyCode() == Engine::KeyCode::A) MoveRL(-speed);
		if (e->GetKeyCode() == Engine::KeyCode::D) MoveRL(speed);
		if (e->GetKeyCode() == Engine::KeyCode::E) MoveUD(speed);
		if (e->GetKeyCode() == Engine::KeyCode::Q) MoveUD(-speed);

		return true;
	}

	void EditorCamera::MousePan(const Math::Vector2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * Time::GetDeltaTime();
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * Time::GetDeltaTime();
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
		m_Zoom -= delta * ZoomSpeed() * Time::GetDeltaTime();
		CalcDist();
	}

	void EditorCamera::MoveFB(float speed)
	{
		m_FocalPoint += GetForwardDirection() * speed * Time::GetDeltaTime();
	}

	void EditorCamera::MoveRL(float speed)
	{
		m_FocalPoint += GetRightDirection() * speed * Time::GetDeltaTime();
	}

	void EditorCamera::MoveUD(float speed)
	{
		m_FocalPoint += GetUpDirection() * speed * Time::GetDeltaTime();
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
		return GetOrientation() * Math::Vector3(0.0f, 0.0f, 1.0f);
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
		return Math::Quaternion(Math::Vector3(m_Pitch, m_Yaw, 0.0f));
	}
}
