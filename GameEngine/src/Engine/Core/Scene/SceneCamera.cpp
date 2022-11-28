#include "pch.h"
#include "SceneCamera.h"

namespace Engine
{
	SceneCamera::SceneCamera() :
		Camera()
	{
		RecalculateProjection();
	}

	void SceneCamera::SetOrthgraphic(float size, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Orthographic;

		m_OrthgraphicSize = size;
		m_OrthgraphicNear = nearClip;
		m_OrthgraphicFar = farClip;

		RecalculateProjection();
	}

	void SceneCamera::SetPerspective(float fov, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Perspective;

		m_PerspectiveFOV = fov;
		m_PerspectiveNear = nearClip;
		m_PerspectiveFar = farClip;

		RecalculateProjection();
	}

	void SceneCamera::SetTransform(const Math::Mat4& transform)
	{
		m_CameraData.ViewMatrix = Math::Inverse(transform);
		m_CameraData.VPMatrix = m_CameraData.ProjectionMatrix * m_CameraData.ViewMatrix;
		UpdateCameraBuffer();
	}

	void SceneCamera::SetViewportSize(uint32 width, uint32 height)
	{
		m_AspectRatio = (float)width / (float)height;
		RecalculateProjection();
	}

	void SceneCamera::RecalculateProjection()
	{
		if (m_ProjectionType == ProjectionType::Perspective)
		{
			m_CameraData.ProjectionMatrix = Math::Perspective(m_PerspectiveFOV, m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
		}
		else
		{
			float orthLeft = -0.5f * m_AspectRatio * m_OrthgraphicSize;
			float orthRight = 0.5f * m_AspectRatio * m_OrthgraphicSize;
			float orthBottom = -0.5f * m_OrthgraphicSize;
			float orthTop = 0.5f * m_OrthgraphicSize;

			m_CameraData.ProjectionMatrix = Math::Ortho(orthLeft, orthRight, orthBottom, orthTop, m_OrthgraphicNear, m_OrthgraphicFar);
		}
	}

}
