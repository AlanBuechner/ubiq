#pragma once
#include "Camera.h"

namespace Engine
{
	class PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera(float fov, float aspect);
		PerspectiveCamera(float fov, float aspect, float zNear, float zFar);
	};
}