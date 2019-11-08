#pragma once
#include <iostream>
#include "Engine/Core/Log.h"
#include "Engine/Core/Time.h"

namespace Engine
{
	class Timer
	{
	public:
		Timer()
		{
			Start();
		}

		void Start()
		{
			m_Start = Time::GetTime();
		}

		void End()
		{
			m_End = Time::GetTime();
			m_Elapsed = m_End - m_Start;
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
		double m_Start;
		double m_End;

		double m_Elapsed;
	};
}