#pragma once
#include "Engine/Core/Application.h"

namespace Engine
{
	class Time
	{
		friend Application;

	public:
		Time() {}

		static inline float GetDeltaTime() { return s_Instance->m_DeltaTime; }
		static inline float GetDeltaSeconds() { return s_Instance->m_DeltaTime; }
		static inline float GetDeltaMilliseconds() { return s_Instance->m_DeltaTime * 1000.0f; }

		static inline double GetTime() { return s_Instance->GetTimeImpl(); }

	private:
		static void UpdateDeltaTime() { s_Instance->UpdateDeltaTimeImpl(); }
		virtual void UpdateDeltaTimeImpl() {}

		virtual double GetTimeImpl() = 0;

		static Time* Create();

	protected:
		float m_DeltaTime = 0.0f;
		float m_LastFrameTime = 0.0f;

	private:
		static Time* s_Instance;
	};
}

typedef Engine::Time Time;