#pragma once
#include <iostream>
#include "Engine/Core/Log.h"
#include "Engine/Core/Time.h"
#include "UArray.h"

namespace Engine
{
	template<typename Fn>
	class Timer
	{
	public:
		Timer(Fn&& func) :
			m_Func(func)
		{
			
		}

		void Start(const char* name)
		{
			m_Name = name;
			m_Start = Time::GetTime();
		}

		void End()
		{
			m_End = Time::GetTime();
			m_Elapsed = m_End - m_Start;
			m_Func({m_Name, (float)GetMilliseconds()});
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

		void PrintTime()
		{
			DEBUG_INFO("{0} : {1}", GetMilliseconds(), m_Name);
		}
	private:
		double m_Start;
		double m_End;

		double m_Elapsed;

		const char* m_Name;

		Fn m_Func;
	};

	class Performance
	{
	public:
		struct ProfileResult
		{
			const char* name;
			float time;
		};

		Performance()
		{

		}

		static void PushResult(ProfileResult result)
		{
			m_ProfileResults.PushBack(result);
		}

		static void Render();

	private:
		static UArray<ProfileResult> m_ProfileResults;
		
	};
}

#define CREATE_PROFILE() Engine::Timer([&](Engine::Performance::ProfileResult profileResult) { Engine::Performance::PushResult(profileResult); });