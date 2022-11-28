#include "pch.h"
#include "Math.h"
#include "Vector.h"

namespace Math
{
	float Length(const Vector2& v)
	{
		return glm::length(v);
	}

	float Length(const Vector3& v)
	{
		return glm::length(v);
	}

	float Length(const Vector4& v)
	{
		return glm::length(v);
	}

	float LengthSqr(const Vector2& v)
	{
		return (v.x * v.x) + (v.y * v.y);
	}

	float LengthSqr(const Vector3& v)
	{
		return (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
	}

	float LengthSqr(const Vector4& v)
	{
		return (v.x * v.x) + (v.y * v.y) + (v.z * v.z) + (v.w * v.w);
	}

	float Distance(const Vector2& v1, const Vector2& v2)
	{
		return Length(v2 - v1);
	}

	float Distance(const Vector3& v1, const Vector3& v2)
	{
		return Length(v2 - v1);
	}

	float Distance(const Vector4& v1, const Vector4& v2)
	{
		return Length(v2 - v1);
	}

	float DistanceSqr(const Vector2& v1, const Vector2& v2)
	{
		return LengthSqr(v2 - v1);
	}

	float DistanceSqr(const Vector3& v1, const Vector3& v2)
	{
		return LengthSqr(v2 - v1);
	}

	float DistanceSqr(const Vector4& v1, const Vector4& v2)
	{
		return LengthSqr(v2 - v1);
	}

	float Dot(const Vector2& v1, const Vector2& v2)
	{
		return glm::dot(v1, v2);
	}

	float Dot(const Vector3& v1, const Vector3& v2)
	{
		return glm::dot(v1, v2);
	}

	float Dot(const Vector4& v1, const Vector4& v2)
	{
		return glm::dot(v1, v2);
	}

	Vector3 Cross(const Vector3& v1, const Vector3& v2)
	{
		return glm::cross(v1, v2);
	}

	Vector2 Normalized(const Vector2& v)
	{
		return glm::normalize(v);
	}

	Vector3 Normalized(const Vector3& v)
	{
		return glm::normalize(v);
	}

	Vector4 Normalized(const Vector4& v)
	{
		return glm::normalize(v);
	}

	Vector2& Normalize(Vector2& v)
	{
		return v = glm::normalize(v);
	}

	Vector3& Normalize(Vector3& v)
	{
		return v = glm::normalize(v);
	}

	Vector4& Normalize(Vector4& v)
	{
		return v = glm::normalize(v);
	}

	Vector2 Lerp(const Vector2& v1, const Vector2& v2, float a)
	{
		return (v1 * a) + v2 * (1.f - a);
	}

	Vector3 Lerp(const Vector3& v1, const Vector3& v2, float a)
	{
		return (v1 * a) + v2 * (1.f - a);
	}

	Vector4 Lerp(const Vector4& v1, const Vector4& v2, float a)
	{
		return (v1 * a) + v2 * (1.f - a);
	}

	Math::Vector3 SphericalToCartesian(float t1, float t2)
	{
		Vector3 pos;
		pos.x = Sin(t2);
		pos.z = Cos(t2);
		pos *= Cos(t1);
		pos.y = Sin(t1);
		return pos;
	}

}
