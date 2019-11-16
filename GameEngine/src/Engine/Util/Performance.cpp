#include "pch.h"
#include "Performance.h"
#include "imgui.h"

namespace Engine
{
	UArray<Performance::ProfileResult> Performance::m_ProfileResults;

	void Performance::Render()
	{
		ImGui::Begin("Profiler");

		for (int i = 0; i < m_ProfileResults.Length(); i++)
		{
			auto& result = m_ProfileResults[i];
			char buffer[1000];
			strcpy(buffer, "%.3fms  ");
			strcat(buffer, result.name);
			ImGui::Text(buffer, result.time);
		}

		m_ProfileResults.Clear();

		ImGui::End();
	}
}