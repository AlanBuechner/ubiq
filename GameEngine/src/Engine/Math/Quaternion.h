#pragma once
#include "Types.h"

namespace Math
{
	float Angle(const Quaternion& q);
	float Length(const Quaternion& q);
	Vector3 Axis(const Quaternion& q);
	Quaternion AngleAxis(float angle, float x, float y, float z);
	Quaternion AngleAxis(float angle, const Vector3& axis);
	Quaternion Inverse(const Quaternion& q);
	Quaternion Conjugate(const Quaternion& q);
	float Dot(const Quaternion& q1, const Quaternion& q2);
	Vector3 EulerAngles(const Quaternion& q);
	Quaternion Mix(const Quaternion& q1, const Quaternion& q2, float a);
	Quaternion Normalize(const Quaternion& q);
	Quaternion Rotate(const Quaternion& q, float angle, const Vector3& axis);
	float Pitch(const Quaternion& q);
	float Roll(const Quaternion& q);
	float Yaw(const Quaternion& q);
	Mat3 Mat3Cast(const Quaternion& q);
	Mat4 Mat4Cast(const Quaternion& q);
	Quaternion QuatCast(const Mat3& mat);
	Quaternion QuatCast(const Mat4& mat);
	Quaternion Lerp(const Quaternion& q1, const Quaternion& q2, float a);
	Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float a);
}
