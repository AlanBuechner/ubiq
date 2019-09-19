#pragma once
#include "Engine/Application.h"

namespace Engine
{
	class Time
	{
		friend Application;

	public:
		Time() {}

		static inline float GetDeltaTime() { return s_Instance->m_DeltaTime; }
		static inline float GetSeconds() { return s_Instance->m_DeltaTime; }
		static inline float GetMilliseconds() { return s_Instance->m_DeltaTime * 1000.0f; }

	private:
		static void UpdateDeltaTime() { s_Instance->UpdateDeltaTimeImpl(); }
		virtual void UpdateDeltaTimeImpl() {}

		static Time* Create();

	protected:
		float m_DeltaTime = 0.0f;
		float m_LastFrameTime = 0.0f;

	private:
		static Time* s_Instance;
	};
}

typedef Engine::Time Time;