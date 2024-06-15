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

		bool Valid();

		void AddPosition(Math::Vector3 position);

		void MinThickness(float min);

		PlainVolume GetPlainVolume();

		static AABB GetExtents(Utils::Vector<AABB> aabbs);
	};
}
