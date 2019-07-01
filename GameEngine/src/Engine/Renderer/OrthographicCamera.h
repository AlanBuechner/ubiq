#pragma once

#include "Camera.h"

namespace Engine
{
	class OrthographicCamera : public Camera
	{
	protected:
		typedef Engine::OrthographicCamera Super;

	public:
		OrthographicCamera(float left, float right, float bottom, float top);
		OrthographicCamera(float left, float right, float bottom, float top, float zNear, float zFar);
	};
}