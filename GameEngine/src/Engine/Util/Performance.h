// https://gist.github.com/TheCherno/31f135eea6ee729ab5f26a6908eb3a5e#file-instrumentor-h

#pragma once
#include <iostream>
#include <fstream>
#include "Engine/Core/Logging/Log.h"
#include "Engine/Core/Time.h"
#include <string>
#include <mutex>

namespace Profiler
{
	struct ProfileResult
	{
		const char* name;
		double start, end;
		uint32 ThreadID;
		uint32 ProccessID;
	};

	struct Event
	{
		std::string name;
		uint32 threadID;
		double time;
	};

	struct InstrumentationSession
	{
		std::string Name;
	};

	// ---------------------- Instrumentor ---------------------- //
	class Instrumentor
	{
	private:
		InstrumentationSession* m_CurrentSession;
		std::ofstream m_OutputStream;
		uint64 m_ProfileCount;
		bool m_RecordData;
		
		std::mutex m_Mutex;

		struct ThreadData
		{
			std::string name;
			uint32 order = UINT32_MAX;
		};

		std::unordered_map<uint32, ThreadData> m_RegisteredThreads;

	public:
		Instrumentor(bool record = false)
			: m_CurrentSession(nullptr), m_RecordData(record)
		{}

		void RecordData(bool record) { m_RecordData = record; }

		void BeginSession(const std::string& name, const std::string& filepath = "results.json");
		void EndSession();

		void WriteProfile(const ProfileResult& result);


		void RegisterThread(const std::string& name, uint32 order);
		void WriteThread(const std::string& name, uint32 threadID, uint32 order);

		static Instrumentor& Get();
	};


#pragma region InstrumentationTimers

	class InstrumentationTimer
	{
	public:
		InstrumentationTimer()
		{}

		//void Start(const std::string& name)
		//{
		//	if (m_TimerRunning)
		//	{
		//		CORE_WARN("Cant start instrumentation timer already running");
		//		return;
		//	}
		//	m_Name = name;
		//	Instrumentor::Get().WriteStartEvent(GetEvent());
		//	m_TimerRunning = true;
		//}
		//
		//void End()
		//{
		//	if (!m_TimerRunning)
		//	{
		//		CORE_WARN("Cant stop instrumentation timer already stoped");
		//		return;
		//	}
		//	Instrumentor::Get().WriteEndEvent(GetEvent());
		//	m_TimerRunning = false;
		//}
		//
		//Event GetEvent()
		//{
		//	Event e;
		//	e.name = m_Name;
		//	e.threadID = (uint32)std::hash<std::thread::id>{}(std::this_thread::get_id());
		//	e.time = Time::GetTime();
		//	return e;
		//}

		void Start(const std::string& name, uint32 pid = 0)
		{
			m_Name = name;
			m_Start = Time::GetTime();
		}

		void End()
		{
			m_End = Time::GetTime();
			m_Elapsed = m_End - m_Start;

			double start = (double)(m_Start * 1000000.0f);
			double end = (double)(m_End * 1000000.0f);
			uint32 threadID = (uint32)std::hash<std::thread::id>{}(std::this_thread::get_id());
			ProfileResult result = { m_Name.c_str(), start, end, threadID, 0 };
			
			Instrumentor::Get().WriteProfile(result);
		}

	private:
		double m_Start, m_End;
		double m_Elapsed;
		bool m_TimerRunning = false;
		std::string m_Name;
	};

	class InstrumentationTimerScoped : protected InstrumentationTimer
	{
	public:
		InstrumentationTimerScoped(const std::string& name)
		{ Start(name); }

		~InstrumentationTimerScoped()
		{ End(); }
	};

#pragma endregion



#pragma region Timers

	template<typename Fn>
	class Timer
	{
	public:
		Timer(Fn&& func) :
			m_Func(func)
		{
		}

		void Start(const std::string& name, uint32 pid = 0)
		{
			m_Name = name;
			m_Start = Time::GetTime();
			m_PID = pid;
		}

		void End()
		{
			m_End = Time::GetTime();
			m_Elapsed = m_End - m_Start;

			double start = (double)(m_Start * 1000000.0f);
			double end = (double)(m_End * 1000000.0f);
			uint32 threadID = (uint32)std::hash<std::thread::id>{}(std::this_thread::get_id());
			ProfileResult result = { m_Name.c_str(), start, end, threadID, m_PID };
			m_Func(result);
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

		uint32 m_PID;

		std::string m_Name;

		Fn m_Func;
	};

	template<typename Fn>
	class TimerScoped : public Timer<Fn>
	{
	public:
		TimerScoped(const std::string& name, Fn&& func, uint32 pid = 0) :
			Timer(func)
		{
			Timer::Start(name, pid);
		}

		~TimerScoped()
		{
			End();
		}
	};

#pragma endregion


}

#define CREATE_PROFILE() Profiler::Timer([&](Profiler::ProfileResult profileResult)
#define CREATE_PROFILE_SCOPE(name) Profiler::TimerScoped timer##__LINE__(name, [&](Profiler::ProfileResult profileResult)
#define CREATE_PROFILE_FUNCTION() CREATE_PROFILE_SCOPE(__FUNCSIG__)

#define CREATE_PROFILEI() Profiler::InstrumentationTimer();
#define CREATE_PROFILE_SCOPEI(name) Profiler::InstrumentationTimerScoped timer##__LINE__(name);
#define CREATE_PROFILE_FUNCTIONI() CREATE_PROFILE_SCOPEI(__FUNCSIG__)
