#include "pch.h"
#include "Quaternion.h"

namespace Math
{
	Vector3 Axis(const Quaternion& q)
	{
		return glm::axis(q);
	}

	Quaternion AngleAxis(float angle, float x, float y, float z)
	{
		return glm::angleAxis(angle, Vector3{ x,y,z });
	}

	Quaternion AngleAxis(float angle, const Vector3& axis)
	{
		return glm::angleAxis(angle, axis);
	}

	Quaternion Inverse(const Quaternion& q)
	{
		return glm::inverse(q);
	}

	Quaternion Conjugate(const Quaternion& q)
	{
		return glm::conjugate(q);
	}

	float Dot(const Quaternion& q1, const Quaternion& q2)
	{
		return glm::dot(q1, q2);
	}

	Vector3 EulerAngles(const Quaternion& q)
	{
		return glm::eulerAngles(q);
	}

	Quaternion Mix(const Quaternion& q1, const Quaternion& q2, float a)
	{
		return glm::mix(q1, q2, a);
	}

	Quaternion Abs(const Quaternion& q)
	{
		return q.w < 0.0 ? -q : q;
	}

	Quaternion Normalize(const Quaternion& q)
	{
		return glm::normalize(q);
	}

	Quaternion Rotate(const Quaternion& q, float angle, const Vector3& axis)
	{
		return glm::rotate(q, angle, axis);
	}

	float Pitch(const Quaternion& q)
	{
		return glm::pitch(q);
	}

	float Roll(const Quaternion& q)
	{
		return glm::roll(q);
	}

	float Yaw(const Quaternion& q)
	{
		return glm::yaw(q);
	}

	Mat3 Mat3Cast(const Quaternion& q)
	{
		return glm::mat3_cast(q);
	}

	Mat4 Mat4Cast(const Quaternion& q)
	{
		return glm::mat4_cast(q);
	}

	Quaternion QuatCast(const Mat3& mat)
	{
		return glm::quat_cast(mat);
	}

	Quaternion QuatCast(const Mat4& mat)
	{
		return glm::quat_cast(mat);
	}

	Quaternion Lerp(const Quaternion& q1, const Quaternion& q2, float a)
	{
		return glm::lerp(q1, q2, a);
	}

	Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float a)
	{
		return glm::slerp(q1, q2, a);
	}
}
