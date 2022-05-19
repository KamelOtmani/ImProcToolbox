#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include <filesystem>
#include <iostream>

#include "UI/ConfigurationTab.hpp"
#include "UI/Style.hpp"

using namespace Walnut;
namespace fs = std::filesystem;

class ExampleLayer : public Walnut::Layer
{
  public:
    virtual void OnAttach() override {}

    virtual void OnUpdate() override {}

    virtual void OnUIRender() override
    {
        ImProc::setupStyle();
        // mConfigurationTab.Render();
        // mConfigurationTab.DrawWindow();
        // mProcessingTab.Render();
        static float alignment = -1;
        //ImGui::SetNextWindowSizeConstraints({-1, 75}, {-1,100});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {-1, 75});
        //ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {-1, 75});
        ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_NoTitleBar);
        ImGui::BeginHorizontal("h1", {-1, 0});
        if (ImGui::Button("Configuration", {150, 50}))
        {
            bShowConfigurationWindow = !bShowConfigurationWindow;
        }
        if (ImGui::Button("Node Editor", {150, 50}))
        {
            bShowNodeEditorWindow = !bShowNodeEditorWindow;
        }
        if (ImGui::Button("Processing", {150, 50}))
        {
            bShowProcessingWindow = !bShowProcessingWindow;
        }
        ImGui::EndHorizontal();
        ImGui::End(); // main window
        ImGui::PopStyleVar();

        //ImGuiTabBarFlags tab_bar_flags =
        //    ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs;
        //if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
        //{
        //    if (ImGui::BeginTabItem("Configuration ", &bShowConfigurationWindow,
        //                            ImGuiTab))
        //    {

        //        ImGui::Text("config");

        //        ImGui::EndTabItem();
        //    }
        //    ImGui::EndTabBar();
        //}
        if (bShowConfigurationWindow)
        {
            mConfigurationTab.DrawWindow();
        }
        if (bShowNodeEditorWindow)
        {
            ImGui::Begin("Node Editor", &bShowNodeEditorWindow);
            ImGui::End();
        }
        if (bShowProcessingWindow)
        {
            ImGui::Begin("Processing", &bShowProcessingWindow);
            ImGui::End();
        }
        ImGui::ShowDemoWindow();
    }

  private:
    // functions

  private:
    bool bShowConfigurationWindow;
    bool bShowNodeEditorWindow;
    bool bShowProcessingWindow;

    //ImProc::TabWindow mProcessingTab{"Processing"};
    ImProc::ConfigurationTab mConfigurationTab{&bShowConfigurationWindow};
};

Walnut::Application *Walnut::CreateApplication(int argc, char **argv)
{
    Walnut::ApplicationSpecification spec;
    spec.Name = "Walnut Example";

    Walnut::Application *app = new Walnut::Application(spec);
    app->PushLayer<ExampleLayer>();
    app->SetMenubarCallback(
        [app]()
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Exit"))
                {
                    app->Close();
                }
                ImGui::EndMenu();
            }
            // if (ImGui::BeginMenu("Show"))
            //{
            //	if (ImGui::MenuItem("Demo"))
            //	{

            //	}
            //	ImGui::EndMenu();
            //}
        });
    return app;
}
