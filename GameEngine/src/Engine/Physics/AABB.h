#pragma once
#include "Engine/Math/Math.h"


#include "Plain.h"

namespace Engine
{
	class AABB
	{
	public:
		Math::Vector3 m_Min = { FLT_MAX,FLT_MAX,FLT_MAX };
		Math::Vector3 m_Max = { FLT_MIN,FLT_MIN,FLT_MIN };

		Math::Vector3 Center() { return (m_Min + m_Max) / 2.0f; }
		Math::Vector3 Extent() { return m_Max - m_Min; }
		Math::Vector3 HalfExtent() { return Extent() / 2.0f; }

		bool Valid();

		void AddPosition(Math::Vector3 position);

		void MinThickness(float min);

		PlainVolume GetPlainVolume();

		bool Intersects(const AABB& other) { return IntersectsX(other) && IntersectsY(other) && IntersectsZ(other); }
		bool IntersectsXY(const AABB& other) { return IntersectsX(other) && IntersectsY(other); }
		bool IntersectsXZ(const AABB& other) { return IntersectsX(other) && IntersectsZ(other); }
		bool IntersectsYZ(const AABB& other) { return IntersectsY(other) && IntersectsZ(other); }
		bool IntersectsX(const AABB& other) { return m_Min.x <= other.m_Max.x && m_Max.x >= other.m_Min.x; }
		bool IntersectsY(const AABB& other) { return m_Min.y <= other.m_Max.y && m_Max.y >= other.m_Min.y; }
		bool IntersectsZ(const AABB& other) { return m_Min.z <= other.m_Max.z && m_Max.z >= other.m_Min.z; }

		static AABB GetExtents(Utils::Vector<AABB> aabbs);
	};
}
