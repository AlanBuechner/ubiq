#pragma once
#include <glm\glm.hpp>

namespace Engine
{
	struct PointLight
	{
		glm::vec3 position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 color = { 1.0f, 1.0f, 1.0f };
		float diffuseIntensity = 1;
		float attConst = 1;
		float attLin = 1;
		float attQuad = 1;
	};
}
