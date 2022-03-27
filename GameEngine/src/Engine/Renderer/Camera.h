#pragma once
#include <Engine/Math/Math.h>
#include <glm/gtc/quaternion.hpp>

namespace Engine
{
	class Camera
	{
	protected:
		typedef Engine::Camera Super;

	public:
		Camera() = default;
		Camera(const Math::Mat4& projectionMatrix) :
			m_ProjectionMatrix(projectionMatrix)
		{}

		virtual ~Camera() = default;

		const Math::Mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

	protected:
		Math::Mat4 m_ProjectionMatrix = Math::Mat4(1.0f);
	};
}
