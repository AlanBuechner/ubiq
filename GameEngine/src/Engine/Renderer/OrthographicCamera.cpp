#include "pch.h"
#include "OrthographicCamera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Engine
{
	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		: Camera(glm::ortho(left, right, bottom, top, -1.0f, 1.0f))
	{
	}

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, float zNear, float zFar)
		: Camera(glm::ortho(left, right, bottom, top, zNear, zFar))
	{
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
	{
		//SetProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f));
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top, float zNear, float zFar)
	{
		//SetProjectionMatrix(glm::ortho(left, right, bottom, top, zNear, zFar));
	}

}