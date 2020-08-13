#pragma once

#include "Engine/Renderer/Camera.h"

namespace Engine
{
	class SceneCamera : public Camera
	{
	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		void SetOrthgraphic(float size, float nearClip, float farClip);

		void SetViewportSize(uint32_t width, uint32_t height);
	private:
		void RecalculateProjection();

	private:
		float m_OrthgraphicSize = 10.0f;
		float m_OrthgraphicNear = -1.0f;
		float m_OrthgraphicFar = 1.0f;

		float m_AspectRatio = 0.0f;
	};
}