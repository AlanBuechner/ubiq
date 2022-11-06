#pragma once
#include <Engine/Math/Math.h>

namespace Engine
{

	struct alignas(void*) DirectionalLight
	{
		DirectionalLight() = default;
		DirectionalLight(Math::Vector3 dir, Math::Vector3 color, float intensity) :
			direction(dir), color(color), intensity(intensity)
		{}

		Math::Vector3 direction = {0, -1, 0};
		float padding1;
		Math::Vector3 color = { 1,1,1 };
		float padding2;
		float intensity = 1;
	};

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
