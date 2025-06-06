// https://gist.github.com/TheCherno/31f135eea6ee729ab5f26a6908eb3a5e#file-instrumentor-h

#pragma once
#include "Common.h"
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <unordered_map>
#include "Types.h"
#include "Time.h"

namespace Profiler
{

	// ---------------------- Instrumentor ---------------------- //
	class Instrumentor
	{
	private:
		std::string m_CurrentSessionName;
		bool m_RecordData;

	public:
		Instrumentor(bool record = false)
			: m_CurrentSessionName(""), m_RecordData(record)
		{}

		void RecordData(bool record) { m_RecordData = record; }

		void RegisterThread(const std::string& name, uint32 order);

		static Instrumentor& Get();
	};


#pragma region InstrumentationTimers

	class InstrumentationTimer
	{
	public:
		void Start(const tracy::SourceLocationData* sourceLocationData);
		void End();

		void Anotate(const std::string& anotation);

	private:
		tracy::ScopedZone* m_ScopedZone = nullptr;
		bool m_TimerRunning = false;
	};

	class InstrumentationTimerScoped : public InstrumentationTimer
	{
	public:
		InstrumentationTimerScoped(const tracy::SourceLocationData* sourceLocationData)
		{ Start(sourceLocationData); }

		~InstrumentationTimerScoped()
		{ End(); }
	};

#pragma endregion

}

#define SOURCE_LOC_NAME TracyConcat(__tracy_source_location,TracyLine)
#define CREATE_SOURCE_LOC(name) static constexpr tracy::SourceLocationData SOURCE_LOC_NAME { name, __FUNCTION__,  __FILE__, (uint32_t)TracyLine, 0 };
#define CREATE_PROFILE_SCOPEI(name) CREATE_SOURCE_LOC(name); Profiler::InstrumentationTimerScoped __timer(&SOURCE_LOC_NAME);
#define CREATE_PROFILE_FUNCTIONI() CREATE_PROFILE_SCOPEI(__FUNCSIG__)

#define ANOTATE_PROFILEI(name, anotation) name.Anotate(anotation);
#define ANOTATE_PROFILEI(anotation) __timer.Anotate(anotation);

#define CREATE_PROFILEI() Profiler::InstrumentationTimer();
#define START_PROFILEI(profiler, name) CREATE_SOURCE_LOC(name); profiler.Start(&SOURCE_LOC_NAME);
#define END_PROFILEI(profiler) profiler.End();
