#pragma once
#include "Engine/Math/Math.h"
#include "Utils/Vector.h"

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

		static AABB GetExtents(Utils::Vector<AABB> aabbs);
	};
}
