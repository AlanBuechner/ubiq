#pragma once

#include "Camera.h"
#include "Engine/Math/Math.h"

namespace Engine
{
	class Event;
	class MouseScrolledEvent;
}

namespace Engine
{
	class EditorCamera : public Camera
	{
	public:
		EditorCamera();
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		void OnUpdate();
		void OnEvent(Event* e);

		inline float GetDistance() const { return m_Distance; }
		inline void SetDistance(float distance) { m_Distance = distance; }

		inline void SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

		const Math::Mat4& GetViewMatrix() const { return m_CameraData.ViewMatrix; }
		Math::Mat4 GetViewProjection() const { return m_CameraData.VPMatrix; }

		Math::Vector3 GetUpDirection() const;
		Math::Vector3 GetRightDirection() const;
		Math::Vector3 GetForwardDirection() const;
		const Math::Vector3& GetPosition() const { return m_Position; }
		Math::Quaternion GetOrientation() const;

		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }

		void SetOrientation(Math::Vector2 orientation) { m_Pitch = orientation.y; m_Yaw = orientation.x; }
	private:
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(MouseScrolledEvent* e);

		void MousePan(const Math::Vector2& delta);
		void MouseRotateAboutFocal(const Math::Vector2& delta);
		void MouseRotate(const Math::Vector2& delta);
		void MouseZoom(float delta);
		
		void MoveFB(float speed);
		void MoveRL(float speed);
		void MoveUD(float speed);

		Math::Vector3 CalculatePosition() const;
		Math::Vector3 CalculateFocal() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

	private:
		float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 10000.0f;

		Math::Vector3 m_Position = { 0.0f, 0.0f, 0.0f };
		Math::Vector3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

		Math::Vector2 m_InitialMousePosition = { 0.0f, 0.0f };

		float m_Distance = 10.0f;
		float m_Pitch = 0, m_Yaw = 0;

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;
		float m_SpeedMultiplyer = 1.0f;
	};
}
