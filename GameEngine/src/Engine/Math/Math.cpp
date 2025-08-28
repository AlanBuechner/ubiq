#include "pch.h"
#include "Math.h"
#include "GLMInclude.h"

namespace Math
{


	// Random
	float Perlin(float p)
	{
		//return glm::perlin(glm::vec1(p));
		return 0;
	}

	float Perlin(const Vector2& p)
	{
		//return glm::perlin(p);
		return 0;
	}

	float Perlin(const Vector3& p)
	{
		//return glm::perlin(p);
		return 0;
	}

	float Perlin(const Vector4& p)
	{
		//return glm::perlin(p);
		return 0;
	}

	float Perlin(float p, float rep)
	{
		//return glm::perlin(glm::vec1(p), glm::vec1(rep));
		return 0;
	}

	float Perlin(const Vector2& p, const Vector2& rep)
	{
		//return glm::perlin(p, rep);
		return 0;
	}

	float Perlin(const Vector3& p, const Vector3& rep)
	{
		//return glm::perlin(p, rep);
		return 0;
	}

	float Perlin(const Vector4& p, const Vector4& rep)
	{
		//return glm::perlin(p, rep);
		return 0;
	}

	float Simplex(float p)
	{
		//return glm::simplex(glm::vec1(p));
		return 0;
	}

	float Simplex(const Vector2& p)
	{
		//return glm::simplex(p);
		return 0;
	}

	float Simplex(const Vector3& p)
	{
		//return glm::simplex(p);
		return 0;
	}

	float Simplex(const Vector4& p)
	{
		//return glm::simplex(p);
		return 0;
	}

	float Angle(const Quaternion& x)
	{
		return glm::angle(x);
	}

	float Length(const Quaternion& q)
	{
		return 0.0f;
	}
}
