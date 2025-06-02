#include "Performance.h"
#include <algorithm>

namespace Profiler
{

	void Instrumentor::RegisterThread(const std::string& name, uint32 order)
	{
		uint32 threadID = (uint32)std::hash<std::thread::id>{}(std::this_thread::get_id());


	}

	Instrumentor& Instrumentor::Get()
	{
		static Instrumentor* instance = new Instrumentor();
		return *instance;
		ZoneScopedN("");
	}

	void InstrumentationTimer::Start(const tracy::SourceLocationData* sourceLocationData)
	{
		if (m_ScopedZone != nullptr)
			return;

		m_ScopedZone = new tracy::ScopedZone(sourceLocationData, TRACY_CALLSTACK, true);
	}

	void InstrumentationTimer::End()
	{
		delete m_ScopedZone;
		m_ScopedZone = nullptr;
	}

	void InstrumentationTimer::Anotate(const std::string& anotation)
	{
		m_ScopedZone->Text(anotation.c_str(), anotation.size());
	}

}


