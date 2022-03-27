#pragma once
#include "Types.h"

namespace Math
{
	float Length(const Vector2& v);
	float Length(const Vector3& v);
	float Length(const Vector4& v);
	float LengthSqr(const Vector2& v);
	float LengthSqr(const Vector3& v);
	float LengthSqr(const Vector4& v);

	float Distance(const Vector2& v1, const Vector2& v2);
	float Distance(const Vector3& v1, const Vector3& v2);
	float Distance(const Vector4& v1, const Vector4& v2);
	float DistanceSqr(const Vector2& v1, const Vector2& v2);
	float DistanceSqr(const Vector3& v1, const Vector3& v2);
	float DistanceSqr(const Vector4& v1, const Vector4& v2);

	float Dot(const Vector2& v1, const Vector2& v2);
	float Dot(const Vector3& v1, const Vector3& v2);
	float Dot(const Vector4& v1, const Vector4& v2);

	Vector3 Cross(const Vector3& v1, const Vector3& v2);

	Vector2 Normalized(const Vector2& v);
	Vector3 Normalized(const Vector3& v);
	Vector4 Normalized(const Vector4& v);
	Vector2& Normalize(Vector2& v);
	Vector3& Normalize(Vector3& v);
	Vector4& Normalize(Vector4& v);

	Vector2 Lerp(const Vector2& v1, const Vector2& v2, float a);
	Vector3 Lerp(const Vector3& v1, const Vector3& v2, float a);
	Vector4 Lerp(const Vector4& v1, const Vector4& v2, float a);
}
