#include "pch.h"
#include "Performance.h"
#include "imgui.h"

namespace Engine
{
	std::vector<ProfileResult> Performance::m_ProfileResults;

	void Instrumentor::BeginSession(const std::string& name, const std::string& filepath)
	{
		m_OutputStream.open(filepath.c_str());
		WriteHeader();
		m_CurrentSession = new InstrumentationSession{ name };
		m_ProfileCount = 0;

		for (auto t : m_RegisteredThreads)
			WriteThread(t.first, t.second.name, t.second.order);
	}

	void Instrumentor::EndSession()
	{
		WriteFooter();
		m_OutputStream.close();
		delete m_CurrentSession;
		m_CurrentSession = nullptr;
		m_ProfileCount = 0;
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
		m_OutputStream << "\"dur\":" << (result.end - result.start) << ',';
		m_OutputStream << "\"name\":\"" << name << "\",";
		m_OutputStream << "\"ph\":\"X\",";
		m_OutputStream << "\"pid\":" << result.ProccessID << ",";
		m_OutputStream << "\"tid\":" << result.ThreadID  << ",";
		m_OutputStream << "\"ts\":" << result.start;
		m_OutputStream << "}";

		m_OutputStream.flush();

		m_Mutex.unlock();
	}

	void Instrumentor::InstantEvent(const std::string& name)
	{
		m_Mutex.lock();

		if (m_ProfileCount++ > 0)
			m_OutputStream << ",";


		m_OutputStream << "{";
		m_OutputStream << "\"cat\":\"function\",";
		m_OutputStream << "\"dur\":" << 0 << ',';
		m_OutputStream << "\"name\":\"" << name << "\",";
		m_OutputStream << "\"ph\":\"X\",";
		m_OutputStream << "\"pid\":" << 0 << ",";
		m_OutputStream << "\"tid\":" << (uint32)std::hash<std::thread::id>{}(std::this_thread::get_id()) << ",";
		m_OutputStream << "\"ts\":" << Time::GetTime();
		m_OutputStream << "}";

		m_OutputStream.flush();

		m_Mutex.unlock();
	}

	void Instrumentor::WriteHeader()
	{
		m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
		m_OutputStream.flush();
	}

	void Instrumentor::WriteFooter()
	{
		m_OutputStream << "]}";
		m_OutputStream.flush();
	}

	void Instrumentor::RecordData(bool record)
	{
		m_RecordData = record;
	}

	void Instrumentor::RegisterThread(const std::string& name, uint32 order)
	{
		uint32 threadID = (uint32)std::hash<std::thread::id>{}(std::this_thread::get_id());
		m_RegisteredThreads[threadID] = { name, order };

		WriteThread(threadID, name, order);
	}

	void Instrumentor::WriteThread(uint32 threadID, const std::string& name, uint32 order)
	{
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

	void Performance::Render()
	{
		return;
		if (m_ProfileResults.size() != 0)
		{
			ImGui::Begin("Profiler");

			for (int i = 0; i < m_ProfileResults.size(); i++)
			{
				auto& result = m_ProfileResults[i];
				std::string buffer = "%.3fms  ";
				buffer += result.name;
				ImGui::Text(buffer.c_str(), ((float)result.end - (float)result.start) / 1000.0f);
			}

			m_ProfileResults.clear();

			ImGui::End();
		}
	}

}
