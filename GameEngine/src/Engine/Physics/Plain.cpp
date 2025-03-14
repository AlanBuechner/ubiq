#include "Plain.h"
#include "Utils/Common.h"

namespace Engine
{

	bool Plain::TestRay(const Ray& ray, RayHit& outHit)
	{
		bool res = TestLine(ray, outHit);
		if (outHit.m_Distance < 0)
			return false;
		return res;
	}

	bool Plain::TestLine(const Ray& ray, RayHit& outHit)
	{
		Math::Vector3 dir = Math::Normalized(ray.m_Direction);

		float s = Math::Dot(m_Normal, dir);
		if (s == 0) {
			return false; // No intersection, the line is parallel to the plane
		}

		// get d value
		float d = Math::Dot(m_Normal, m_Center);
		// Compute the X value for the directed line ray intersecting the plane
		float x = (d - Math::Dot(m_Normal, ray.m_Origin)) / Math::Dot(m_Normal, dir);

		// output contact point
		outHit.m_HitPoint = ray.m_Origin + dir * x; //Make sure your ray vector is normalized
		outHit.m_Normal = m_Normal;
		outHit.m_Distance = x;
		outHit.m_HitBack = s > 0;
		return true;
	}

	void Plain::Transform(const Math::Mat4& transform)
	{
		m_Normal = (Math::Mat3)transform * m_Normal;
		m_Center = transform * Math::Vector4(m_Center, 1);
	}

	bool PlainVolume::TestRay(const Ray& ray, RayHit& outHit)
	{
		uint32 numFaceingPlains = 0;
		Utils::Vector<RayHit> hits;
		hits.Reserve(m_Plains.Count());
		for (Plain& plain : m_Plains)
		{
			RayHit hit;
			if (plain.TestRay(ray, hit))
				hits.Push(hit);

			if (Math::Dot(plain.m_Normal, ray.m_Origin - plain.m_Center) > 0) numFaceingPlains++;
		}

		if (hits.Empty()) return false;

		Utils::Vector<RayHit> sortedHits;
		while (!hits.Empty())
		{
			uint32 closestIndex = 0;
			float closestDistance = FLT_MAX;
			for (uint32 i = 0; i < hits.Count(	); i++)
			{
				if (hits[i].m_Distance < closestDistance)
				{
					closestDistance = hits[i].m_Distance;
					closestIndex = i;
				}
			}

			sortedHits.Push(hits[closestIndex]);
			hits.Remove(closestIndex);
		}


		if (numFaceingPlains == 0)
		{
			outHit = sortedHits[0];
			return true;
		}

		for (uint32 i = 0; i < numFaceingPlains; i++)
		{
			if (sortedHits[i].m_HitBack)
				return false;
		}

		outHit = sortedHits[numFaceingPlains - 1];
		return true;
	}

	void PlainVolume::Transform(const Math::Mat4& transform)
	{
		for (Plain& plain : m_Plains)
			plain.Transform(transform);
	}

}

