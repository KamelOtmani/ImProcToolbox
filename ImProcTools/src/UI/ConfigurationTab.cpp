#include "ConfigurationTab.hpp"
#include <imgui.h>

namespace ImProc
{

ConfigurationTab::ConfigurationTab(bool *ShowWindow) : bShowWindow(ShowWindow) {
}

void ConfigurationTab::DrawWindow()
{

    ImGui::Begin("Config", bShowWindow);




    ImGui::End();
}
}