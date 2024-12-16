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

	float Cross(const Vector2& v1, const Vector2& v2)
	{
		return (v1.x * v2.y) - (v2.x * v1.y);
	}

	Vector2 Degrees(Vector2 rad)
	{
		return { Degrees(rad.x), Degrees(rad.y) };
	}

	Vector2 Radians(Vector2 deg)
	{
		return { Radians(deg.x), Radians(deg.y) };
	}

	Vector3 Degrees(Vector3 rad)
	{
		return { Degrees(rad.x), Degrees(rad.y), Degrees(rad.z) };
	}

	Vector3 Radians(Vector3 deg)
	{
		return { Radians(deg.x), Radians(deg.y), Radians(deg.z) };
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
		return glm::mix(v1, v2, a);
	}

	Vector3 Lerp(const Vector3& v1, const Vector3& v2, float a)
	{
		return glm::mix(v1, v2, a);
	}

	Vector4 Lerp(const Vector4& v1, const Vector4& v2, float a)
	{
		return glm::mix(v1, v2, a);
	}

	Math::Vector3 SphericalToCartesian(Vector2 rot)
	{
		Vector3 pos;
		float c = Cos(rot.y);
		pos.x = Cos(rot.x) * c;
		pos.y = Sin(rot.y);
		pos.z = Sin(rot.x) * c;
		return pos;
	}

	Vector2 CartesianToSpherical(Vector3 dir)
	{
		Vector2 angles;
		angles.y = Math::Degrees(Math::Asin(dir.y));
		float sy = Math::Sin(angles.y);
		float c = Math::Acos(dir.x/sy);
		float s = Math::Degrees(Math::Asin(dir.z/sy));
		if (c > 0)
			angles.x = s;
		else
			angles.x = 180 - s;
		return angles;
	}

}
