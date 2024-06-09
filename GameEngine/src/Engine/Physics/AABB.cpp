#include "AABB.h"

namespace Engine
{

	void AABB::AddPosition(Math::Vector3 position)
	{
		if (position.x < m_Min.x) m_Min.x = position.x;
		if (position.y < m_Min.y) m_Min.y = position.y;
		if (position.z < m_Min.z) m_Min.z = position.z;

		if (position.x > m_Max.x) m_Max.x = position.x;
		if (position.y > m_Max.y) m_Max.y = position.y;
		if (position.z > m_Max.z) m_Max.z = position.z;
	}

	void AABB::MinThickness(float min)
	{
		float xDist = m_Max.x - m_Min.x;
		if (xDist < min) 
		{
			float fudge = (min - xDist)/2.0f;
			m_Min.x -= fudge;
			m_Max.x += fudge;
		}

		float yDist = m_Max.y - m_Min.y;
		if (yDist < min)
		{
			float fudge = (min - yDist) / 2.0f;
			m_Min.y -= fudge;
			m_Max.y += fudge;
		}

		float zDist = m_Max.z - m_Min.z;
		if (zDist < min)
		{
			float fudge = (min - zDist) / 2.0f;
			m_Min.z -= fudge;
			m_Max.z += fudge;
		}
	}

	PlainVolume AABB::GetPlainVolume()
	{
		Utils::Vector<Plain> plains;
		plains.Resize(6);

		plains[0] = Plain{ m_Min, {-1,0,0} };
		plains[1] = Plain{ m_Min, {0,-1,0} };
		plains[2] = Plain{ m_Min, {0,0,-1} };
		plains[3] = Plain{ m_Max, {1,0,0} };
		plains[4] = Plain{ m_Max, {0,1,0} };
		plains[5] = Plain{ m_Max, {0,0,1} };

		return PlainVolume{ plains };
	}

	AABB AABB::GetExtents(Utils::Vector<AABB> aabbs)
	{
		if (aabbs.Empty())
			return AABB();

		Math::Vector3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
		Math::Vector3 max = { FLT_MIN, FLT_MIN, FLT_MIN };

		for (AABB aabb : aabbs)
		{
			if (aabb.m_Min.x < min.x) min.x = aabb.m_Min.x;
			if (aabb.m_Min.y < min.y) min.y = aabb.m_Min.y;
			if (aabb.m_Min.z < min.z) min.z = aabb.m_Min.z;

			if (aabb.m_Max.x > max.x) max.x = aabb.m_Max.x;
			if (aabb.m_Max.y > max.y) max.y = aabb.m_Max.y;
			if (aabb.m_Max.z > max.z) max.z = aabb.m_Max.z;
		}

		return AABB{ min, max };
	}

}

