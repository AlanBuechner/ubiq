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
	template <typename T> T Sin(T x) { return glm::sin(x); }
	template <typename T> T Cos(T x) { return glm::cos(x); }
	template <typename T> T Tan(T x) { return glm::tan(x); }
	template <typename T> T Cot(T x) { return 1.0f / Tan(x); }
	template <typename T> T Sec(T x) { return 1.0f / Cos(x); }
	template <typename T> T Csc(T x) { return 1.0f / Sin(x); }

	template <typename T> T Asin(T x) { return glm::asin(x); }
	template <typename T> T Acos(T x) { return glm::acos(x); }
	template <typename T> T Atan(T x) { return glm::atan(x); }
	template <typename T> T Acot(T x) { return 1.0f / Atan(x); }
	template <typename T> T Asec(T x) { return 1.0f / Acos(x); }
	template <typename T> T Acsc(T x) { return 1.0f / Asin(x); }

	template <typename T> T Atan2(T y, T x) { return glm::atan(x, y); }

	template <typename T> T Sinh(T x) { return glm::sinh(x); }
	template <typename T> T Cosh(T x) { return glm::cosh(x); }
	template <typename T> T Tanh(T x) { return glm::tanh(x); }


	template <typename T> T Degrees(T x) { return glm::degrees((float)x); }
	template <typename T> T Radians(T x) { return glm::radians((float)x); }
	//template <> double Degrees(double x) { return glm::degrees(x); }
	//template <> double Radians(double x) { return glm::radians(x); }

	// float functions
	template <typename T> T Sqrt(T x) { return glm::sqrt(x); }
	template <typename T> T InvSqrt(T x) { return glm::inversesqrt(x); }
	template <typename T> T Pow(T x, T y) { return glm::pow(x, y); }
	template <typename T> T Exp(T x) { return glm::exp(x); }

	template <typename T> T Log(T x) { return glm::log(x); }

	template <typename T> T Floor(T x) { return glm::floor(x); }
	template <typename T> T Ceil(T x) { return glm::ceil(x); }

	template <typename T> T Abs(T x) { return glm::abs(x); }
	template <typename T> T Clamp(T x, T min, T max) { return glm::clamp(x, min, max); }
	template <typename T> T Min(T x, T y) { return glm::min(x, y); }
	template <typename T> T Max(T x, T y) { return glm::max(x, y); }
	template <typename T> T Mod(T x, T y) { return glm::mod(x, y); }

	template <typename T> T Lerp(T a, T b, T t) { return glm::lerp(a, b, t); }
	template <typename T> T Slerp(T a, T b, T t) { return glm::slerp(a, b, t); }

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
