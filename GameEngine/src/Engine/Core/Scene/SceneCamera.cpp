#include "pch.h"
#include "SceneCamera.h"

namespace Engine
{
	SceneCamera::SceneCamera()
	{
		RecalculateProjection();
	}

	void SceneCamera::SetOrthgraphic(float size, float nearClip, float farClip)
	{
		m_OrthgraphicSize = size;
		m_OrthgraphicNear = nearClip;
		m_OrthgraphicFar = farClip;

		RecalculateProjection();
	}

	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_AspectRatio = (float)width / (float)height;
		RecalculateProjection();
	}

	void SceneCamera::RecalculateProjection()
	{
		float orthLeft = -0.5 * m_AspectRatio * m_OrthgraphicSize;
		float orthRight = 0.5 * m_AspectRatio * m_OrthgraphicSize;
		float orthBottom = -0.5 * m_OrthgraphicSize;
		float orthTop = 0.5 * m_OrthgraphicSize;

		m_ProjectionMatrix = glm::ortho(orthLeft, orthRight, orthBottom, orthTop, m_OrthgraphicNear, m_OrthgraphicFar);
	}

}