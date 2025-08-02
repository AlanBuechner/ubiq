#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include "Utils/Types.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include "imgui_threaded_rendering.h"


namespace ImGui {
	bool Begin(const char* name, uint32 z_order, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
}
