#pragma once
#include "Engine/Core/Application.h"

namespace Engine
{
	class Time
	{
		friend Application;

	public:
		Time() {}

		static inline float GetDeltaTime() { return (float)s_DeltaTime; }
		static inline float GetDeltaSeconds() { return (float)s_DeltaTime; }
		static inline float GetDeltaMilliseconds() { return (float)s_DeltaTime * 1000.0f; }

		static inline float GetFPS() { return (float)(1.0 / s_DeltaTime); }

		static double GetTime();

	private:
		static void UpdateDeltaTime();

	protected:
		static double s_DeltaTime;
		static double s_LastFrameTime;
	};
}

typedef Engine::Time Time;
