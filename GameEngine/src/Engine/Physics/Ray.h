#pragma once
#include "Engine/Math/Math.h"

namespace Engine
{
	struct Ray
	{
		Math::Vector3 m_Origin;
		Math::Vector3 m_Direction;
	};

	struct RayHit
	{
		Math::Vector3 m_HitPoint;
		Math::Vector3 m_Normal;
		float m_Distance;
		bool m_HitBack;
	};
}
