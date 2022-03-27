#include "pch.h"
#include "Math.h"

#include <glm/gtc/noise.hpp>
#include <Engine/Math/Math.h>
#include <math.h>

namespace Math
{
	// trig
	float Sin(float x)
	{
		return glm::sin(x);
	}

	float Cos(float x)
	{
		return glm::cos(x);
	}

	float Tan(float x)
	{
		return glm::tan(x);
	}

	float Cot(float x)
	{
		return 1 / glm::tan(x);
	}

	float Sec(float x)
	{
		return 1.0f / glm::cos(x);
	}

	float Csc(float x)
	{
		return 1.0f / glm::sin(x);
	}

	float Asin(float x)
	{
		return glm::asin(x);
	}

	float Acos(float x)
	{
		return glm::acos(x);
	}

	float Atan(float x)
	{
		return glm::atan(x);
	}

	float Atan2(float y, float x)
	{
		return glm::atan(y / x);
	}

	float Sinh(float x)
	{
		return glm::sinh(x);
	}

	float Cosh(float x)
	{
		return glm::cosh(x);
	}

	float Tanh(float x)
	{
		return glm::tanh(x);
	}

	float Degrees(float x)
	{
		return glm::degrees(x);
	}

	float Radians(float x)
	{
		return glm::radians(x);
	}

	// float functions
	float Sqrt(float x)
	{
		return glm::sqrt(x);
	}

	float Pow(float x, float y)
	{
		return glm::pow(x, y);
	}

	float Exp(float x)
	{
		return glm::exp(x);
	}

	float Log(float x)
	{
		return glm::log(x);
	}

	float Floor(float x)
	{
		return glm::floor(x);
	}

	float Ceil(float x)
	{
		return glm::ceil(x);
	}

	float Abs(float x)
	{
		return glm::abs(x);
	}

	float Clamp(float x, float min, float max)
	{
		return glm::clamp(x, min, max);
	}

	float Min(float x, float y)
	{
		return glm::min(x, y);
	}

	float Max(float x, float y)
	{
		return glm::max(x, y);
	}

	float Mod(float x, float y)
	{
		return glm::mod(x, y);
	}

	// Random
	float Perlin(float p)
	{
		return glm::perlin(glm::vec1(p));
	}

	float Perlin(const Vector2& p)
	{
		return glm::perlin(p);
	}

	float Perlin(const Vector3& p)
	{
		return glm::perlin(p);
	}

	float Perlin(const Vector4& p)
	{
		return glm::perlin(p);
	}

	float Perlin(float p, float rep)
	{
		return glm::perlin(glm::vec1(p), glm::vec1(rep));
	}

	float Perlin(const Vector2& p, const Vector2& rep)
	{
		return glm::perlin(p, rep);
	}

	float Perlin(const Vector3& p, const Vector3& rep)
	{
		return glm::perlin(p, rep);
	}

	float Perlin(const Vector4& p, const Vector4& rep)
	{
		return glm::perlin(p, rep);
	}

	float Simplex(float p)
	{
		return glm::simplex(glm::vec1(p));
	}

	float Simplex(const Vector2& p)
	{
		return glm::simplex(p);
	}

	float Simplex(const Vector3& p)
	{
		return glm::simplex(p);
	}

	float Simplex(const Vector4& p)
	{
		return glm::simplex(p);
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
