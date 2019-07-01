#include "pch.h"
#include "PerspectiveCamera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Engine
{
	PerspectiveCamera::PerspectiveCamera(float fov, float aspect)
		: Camera(glm::perspective(glm::radians(fov), aspect, 0.1f, 100.f))
	{
	}

	PerspectiveCamera::PerspectiveCamera(float fov, float aspect, float zNear, float zFar)
		: Camera(glm::perspective(fov, aspect, zNear, zFar))
	{
	}
}