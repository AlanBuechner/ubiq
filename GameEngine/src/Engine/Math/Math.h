#pragma once

#include "Types.h"
#include "Vector.h"
#include "Matrix.h"
#include "Quaternion.h"
#include <math.h>

namespace Math
{

	constexpr float PI = 3.14592654f;

	// trig
	float Sin(float x);
	float Cos(float x);
	float Tan(float x);
	float Cot(float x);
	float Sec(float x);
	float Csc(float x);

	float Asin(float x);
	float Acos(float x);
	float Atan(float x);

	float Atan2(float y, float x);

	float Sinh(float x);
	float Cosh(float x);
	float Tanh(float x);

	float Degrees(float x);
	float Radians(float x);

	// float functions
	float Sqrt(float x);
	float Pow(float x, float y);
	float Exp(float x);

	float Log(float x);

	float Floor(float x);
	float Ceil(float x);
	float Abs(float x);
	float Clamp(float x, float min, float max);
	float Min(float x, float y);
	float Max(float x, float y);
	float Mod(float x, float y);

	uint32 Clamp(uint32 x, uint32 min, uint32 max);

	float Lerp(float a, float b, float t);

	// Random
	float Perlin(float p);
	float Perlin(const Vector2& p);
	float Perlin(const Vector3& p);
	float Perlin(const Vector4& p);

	float Perlin(float p, float rep);
	float Perlin(const Vector2& p, const Vector2& rep);
	float Perlin(const Vector3& p, const Vector3& rep);
	float Perlin(const Vector4& p, const Vector4& rep);

	float Simplex(float p);
	float Simplex(const Vector2& p);
	float Simplex(const Vector3& p);
	float Simplex(const Vector4& p);
}
