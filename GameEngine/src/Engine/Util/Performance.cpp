#include "pch.h"
#include "Performance.h"
#include "imgui.h"

namespace Profiler
{
	void Instrumentor::BeginSession(const std::string& name, const std::string& filepath)
	{
		m_OutputStream.open(filepath.c_str());
		m_CurrentSession = new InstrumentationSession{ name };
		m_ProfileCount = 0;

		// write header
		m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
		m_OutputStream.flush();

		// write threads that were registered before hand
		for (auto t : m_RegisteredThreads)
			WriteThread(t.second.name, t.first, t.second.order);
	}

	void Instrumentor::EndSession()
	{
		// write footer
		m_OutputStream << "]}";
		m_OutputStream.flush();

		m_OutputStream.close();
		delete m_CurrentSession;
		m_CurrentSession = nullptr;
	}

	void Instrumentor::WriteProfile(const ProfileResult& result)
	{
		if (!m_RecordData)
			return;
		m_Mutex.lock();

		if (m_ProfileCount++ > 0)
			m_OutputStream << ",";

		std::string name = result.name;
		std::replace(name.begin(), name.end(), '"', '\'');

		m_OutputStream << "{";
		m_OutputStream << "\"cat\":\"function\",";
		m_OutputStream << "\"dur\":" << std::to_string(result.end - result.start) << ',';
		m_OutputStream << "\"name\":\"" << name << "\",";
		m_OutputStream << "\"ph\":\"X\",";
		m_OutputStream << "\"pid\":" << result.ProccessID << ",";
		m_OutputStream << "\"tid\":" << result.ThreadID << ",";
		m_OutputStream << "\"ts\":" << std::to_string(result.start);
		m_OutputStream << "}";

		m_OutputStream.flush();
		m_Mutex.unlock();
	}

	void Instrumentor::RegisterThread(const std::string& name, uint32 order)
	{
		uint32 threadID = (uint32)std::hash<std::thread::id>{}(std::this_thread::get_id());
		m_RegisteredThreads[threadID] = { name, order };

		WriteThread(name, threadID, order);
	}

	void Instrumentor::WriteThread(const std::string& name, uint32 threadID, uint32 order)
	{
		if (!m_RecordData)
			return;
		m_Mutex.lock();

		if (m_ProfileCount++ > 0)
			m_OutputStream << ",";

		m_OutputStream << "{";
		m_OutputStream << "\"name\":\"thread_name\",";
		m_OutputStream << "\"ph\":\"M\",";
		m_OutputStream << "\"pid\":0,";
		m_OutputStream << "\"tid\":" << threadID << ",";
		m_OutputStream << "\"args\":{";
		m_OutputStream << "\"name\":\"" << name << "\"";
		m_OutputStream << "}";
		m_OutputStream << "},";

		m_OutputStream << "{";
		m_OutputStream << "\"name\":\"thread_sort_index\",";
		m_OutputStream << "\"ph\":\"M\",";
		m_OutputStream << "\"pid\":0,";
		m_OutputStream << "\"tid\":" << threadID << ",";
		m_OutputStream << "\"args\":{";
		m_OutputStream << "\"sort_index \":" << order << "";
		m_OutputStream << "}";
		m_OutputStream << "}";

		m_OutputStream.flush();
		m_Mutex.unlock();
	}

	

	Instrumentor& Instrumentor::Get()
	{
		static Instrumentor* instance = new Instrumentor();
		return *instance;
	}
}


