#pragma once
#include "Types.h"

namespace Engine::Math
{
	Mat4 Ortho(float left, float right, float bottom, float top, float zNear, float zFar);
	Mat4 Perspective(float fovy, float aspect, float zNear, float zFar);
	Mat4 LookAt(const Vector3& eye, const Vector3& center, const Vector3& up);
	Mat4 Rotate(float angle, const Vector3& axis);
	Mat4 Scale(const Vector3& scale);
	Mat4 Translate(const Vector3& position);

	Mat2 Inverse(const Mat2& mat);
	Mat3 Inverse(const Mat3& mat);
	Mat4 Inverse(const Mat4& mat);

	bool DecomposeTransform(const Mat4& transform, Vector3& outPosition, Vector3& outRotation, Vector3& outScale);

}
