#pragma once

#include "Engine/Renderer/Camera.h"

namespace Engine
{
	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType
		{
			Perspective = 0,
			Orthographic = 1
		};
	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		void SetViewportSize(uint32 width, uint32 height);

		void SetOrthgraphic(float size, float nearClip, float farClip);
		float GetOrthographicSize() { return m_OrthgraphicSize; }
		void SetOrthographicSize(float size) { m_OrthgraphicSize = size; RecalculateProjection(); }
		float GetOrthographicNearClip() { return m_OrthgraphicNear; }
		void SetOrthographicNearClip(float nearPlane) { m_OrthgraphicNear = nearPlane; RecalculateProjection(); }
		float GetOrthographicFarClip() { return m_OrthgraphicFar; }
		void SetOrthographicFarClip(float farPlane) { m_OrthgraphicFar = farPlane; RecalculateProjection(); }

		void SetPerspective(float fov, float nearClip, float farClip);
		float GetPerspectiveVerticalFOV() { return m_PerspectiveFOV; }
		void SetPerspectiveVerticalFOV(float fov) { m_PerspectiveFOV = fov; RecalculateProjection(); }
		float GetPerspectiveNearClip() { return m_PerspectiveNear; }
		void SetPerspectiveNearClip(float nearPlane) { m_PerspectiveNear = nearPlane; RecalculateProjection(); }
		float GetPerspectiveFarClip() { return m_PerspectiveFar; }
		void SetPerspectiveFarClip(float farPlane) { m_PerspectiveFar = farPlane; RecalculateProjection(); }

		ProjectionType GetProjectionType() const { return m_ProjectionType; }
		void SetProjectionType(ProjectionType type) { m_ProjectionType = type; RecalculateProjection(); }

		void SetTransform(const Math::Mat4& transform);
	private:
		void RecalculateProjection();

	private:
		ProjectionType m_ProjectionType = ProjectionType::Orthographic;

		float m_OrthgraphicSize = 10.0f;
		float m_OrthgraphicNear = -1.0f;
		float m_OrthgraphicFar = 1.0f;

		float m_PerspectiveFOV = Math::Radians(45.0f);
		float m_PerspectiveNear = 0.001f;
		float m_PerspectiveFar = 1000.0f;

		float m_AspectRatio = 1.0f;
	};
}
