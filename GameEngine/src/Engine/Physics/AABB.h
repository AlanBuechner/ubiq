#pragma once
#include "Engine/Math/Math.h"
#include "Utils/Vector.h"

#include "Plain.h"

namespace Engine
{
	class AABB
	{
	public:
		Math::Vector3 m_Min = { 0,0,0 };
		Math::Vector3 m_Max = { 0,0,0 };

		void AddPosition(Math::Vector3 position);

		void MinThickness(float min);

		PlainVolume GetPlainVolume();

		static AABB GetExtents(Utils::Vector<AABB> aabbs);
	};
}
