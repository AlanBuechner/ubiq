#include "pch.h"
#include "Performance.h"
#include "imgui.h"
#include "UString.h"

namespace Engine
{
	UArray<ProfileResult> Performance::m_ProfileResults;

	void Instrumentor::BeginSession(const UString& name, const UString& filepath)
	{
		m_OutputStream.open(filepath.RawString());
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

	Instrumentor& Instrumentor::Get()
	{
		static Instrumentor* instance = new Instrumentor();
		return *instance;
	}

	void Performance::Render()
	{
		if (m_ProfileResults.Length() != 0)
		{
			ImGui::Begin("Profiler");

			for (int i = 0; i < m_ProfileResults.Length(); i++)
			{
				auto& result = m_ProfileResults[i];
				UString buffer = "%.3fms  ";
				buffer += result.name;
				ImGui::Text(buffer.RawString(), ((float)result.end - (float)result.start) / 1000.0f);
			}

			m_ProfileResults.Clear();

			ImGui::End();
		}
	}

}