#pragma once

#include "Utils/Types.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>


namespace ImGui {
	bool Begin(const char* name, uint32 z_order, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
}
