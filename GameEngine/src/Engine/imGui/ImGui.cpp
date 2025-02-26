#include "pch.h"

#include "backends/imgui_impl_dx12.cpp"
#include "backends/imgui_impl_win32.cpp"
#include "misc/cpp/imgui_stdlib.h"

#include "Utils/Common.h"


namespace ImGui
{
	bool Begin(const char* name, uint32 z_order, bool* p_open, ImGuiWindowFlags flags)
	{
		bool r = ImGui::Begin(name, p_open, flags);
		ImGui::GetCurrentWindow()->BeginOrderWithinContext = z_order;
		return r;
	}
}
