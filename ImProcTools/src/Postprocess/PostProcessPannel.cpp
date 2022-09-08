#include "PostProcessPannel.hpp"
#include <imgui.h>

PostprocessTab::PostprocessTab()
{
}

PostprocessTab::~PostprocessTab()
{
}

void PostprocessTab::Render()
{
    if (ImGui::BeginTabItem("Post Processing"))
    {
        ImGui::Text("This is the Avocado tab!\nblah blah blah blah blah");
        ImGui::EndTabItem();
    }
}
