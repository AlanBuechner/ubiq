#pragma once
#include <Engine/Math/Math.h>

namespace Engine
{
	struct PointLight
	{
		Math::Vector3 position = { 0.0f, 0.0f, 0.0f };
		Math::Vector3 color = { 1.0f, 1.0f, 1.0f };
		float diffuseIntensity = 1;
		float attConst = 1;
		float attLin = 1;
		float attQuad = 1;
	};
}
