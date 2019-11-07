#pragma once
#include <chrono>
#include "Engine/Core/Log.h"
#include <iostream>

namespace Engine
{
	class Timer
	{
		typedef std::chrono::high_resolution_clock Clock;
		typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;
	public:
		Timer()
		{
			Start();
		}

		void Start()
		{
			m_Start = Clock::now();
		}

		void End()
		{
			m_End = Clock::now();
			std::chrono::duration<double> elapsed = (m_End - m_Start);
			m_Elapsed = elapsed.count();
		}

		double GetSeconds()
		{
			return m_Elapsed;
		}

		double GetMilliseconds()
		{
			return m_Elapsed * 1000.0f;
		}

		double GetMicroseconds()
		{
			return m_Elapsed * 1000000.0f;
		}
	private:
		TimePoint m_Start;
		TimePoint m_End;

		double m_Elapsed;
	};
}