#pragma once
#include "Engine/Math/Math.h"
#include "Ray.h"
#include "Utils/Vector.h"

namespace Engine
{
	class Plain
	{
	public:
		Math::Vector3 m_Center;
		Math::Vector3 m_Normal;

		bool TestRay(const Ray& ray, RayHit& outHit);

		void Transform(const Math::Mat4& transform);
	};

	class PlainVolume
	{
	public:
		Utils::Vector<Plain> m_Plains;

		bool TestRay(const Ray& ray, RayHit& outHit);

		void Transform(const Math::Mat4& transform);
	};

}
