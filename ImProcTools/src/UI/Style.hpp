#pragma once
#include "imgui.h"

namespace ImProc
{
void setupStyle() { 
	auto& style = ImGui::GetStyle();
    style.FramePadding = ImVec2{6.0, 8.0};
}
}