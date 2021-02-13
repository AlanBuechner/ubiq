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

		if (m_ProfileCount++ > 0)
			m_OutputStream << ",";

		std::string name = result.name;
		std::replace(name.begin(), name.end(), '"', '\'');

		m_OutputStream << "{";
		m_OutputStream << "\"cat\":\"function\",";
		m_OutputStream << "\"dur\":" << (result.end - result.start) << ',';
		m_OutputStream << "\"name\":\"" << name << "\",";
		m_OutputStream << "\"ph\":\"X\",";
		m_OutputStream << "\"pid\":0,";
		m_OutputStream << "\"tid\":" << result.ThreadID << ",";
		m_OutputStream << "\"ts\":" << result.start;
		m_OutputStream << "}";

		m_OutputStream.flush();
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

	Instrumentor& Instrumentor::Get()
	{
		static Instrumentor* instance = new Instrumentor();
		return *instance;
	}

	void Performance::Render()
	{
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