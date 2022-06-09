#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include <filesystem>
#include <iostream>

#include "UI/ResultsViewer.hpp"
#include "UI/BackgroundExtractor.hpp"
#include "UI/ConfigurationTab.hpp"
#include "UI/Style.hpp"
#include "implot.h"
#include <UI/BatchProcessing.hpp>

using namespace Walnut;
namespace fs = std::filesystem;

class ExampleLayer : public Walnut::Layer
{
  public:
    virtual void OnAttach() override {}

    virtual void OnUpdate() override
    {
        mBatchProcessingTab.Update();
        mBackgroundExtractorTab.Update();
        mResultsViewerTab.Update();
    }

    virtual void OnUIRender() override
    {
        ImProc::setupStyle();
        // mConfigurationTab.Render();
        // mConfigurationTab.DrawWindow();
        // mProcessingTab.Render();
        static float alignment = -1;
        // ImGui::SetNextWindowSizeConstraints({-1, 75}, {-1,100});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {-1, 75});
        // ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {-1, 75});
        ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_NoTitleBar);
        ImGui::BeginHorizontal("h1", {-1, 0});
        // if (ImGui::Button("Configuration", {150, 50}))
        //{
        //    bShowConfigurationWindow = !bShowConfigurationWindow;
        //}
        if (ImGui::Button("Background Extractor", {200, 50}))
        {
            bShowBackgroundExtractingWindow = !bShowBackgroundExtractingWindow;
        }
        if (ImGui::Button("Batch Processing", {150, 50}))
        {
            bShowBatchProcessingWindow = !bShowBatchProcessingWindow;
        }
        if (ImGui::Button("Results Viewer", {200, 50}))
        {
            bShowResultsViewerWindow = !bShowResultsViewerWindow;
        }
        ImGui::BeginDisabled(true);
        if (ImGui::Button("Node Editor", {150, 50}))
        {
            bShowNodeEditorWindow = !bShowNodeEditorWindow;
        }
        ImGui::EndDisabled();

        ImGui::EndHorizontal();
        ImGui::End(); // main window
        ImGui::PopStyleVar();

        if (bShowConfigurationWindow)
        {
            mConfigurationTab.DrawWindow();
        }

        if (bShowBatchProcessingWindow)
        {
            mBatchProcessingTab.DrawWindow();
        }
        if (bShowBackgroundExtractingWindow)
        {
            mBackgroundExtractorTab.DrawWindow();
        }
        if (bShowResultsViewerWindow)
        {
            mResultsViewerTab.DrawWindow();
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

        if (bShowDemoWindow)
            ImGui::ShowDemoWindow(&bShowDemoWindow);
        if (bShowPlotsDemo)
        {
            int bar_data[10] = {0, 2, 3, 4, 5, 8, 7, 9, 6, 10};

            ImGui::Begin("Plots");
            if (ImPlot::BeginPlot("My Plot"))
            {
                ImPlot::PlotBars("My Bar Plot", bar_data, 10);
                // ImPlot::PlotLine("My Line Plot", x_data, y_data, 1000);
                ImPlot::EndPlot();
            }
            ImGui::End();
        }
    }

  private:
    // functions
  public:
    bool bShowDemoWindow = false;
    bool bShowPlotsDemo = false;

  private:
    bool bShowConfigurationWindow;
    bool bShowBatchProcessingWindow;
    bool bShowNodeEditorWindow;
    bool bShowProcessingWindow;
    bool bShowBackgroundExtractingWindow;
    bool bShowResultsViewerWindow;

    // ImProc::TabWindow mProcessingTab{"Processing"};
    ImProc::ConfigurationTab mConfigurationTab{&bShowConfigurationWindow};
    ImProc::BatchProcessingTab mBatchProcessingTab{&bShowBatchProcessingWindow};
    ImProc::BackgroundExtractorTab mBackgroundExtractorTab{
        &bShowBackgroundExtractingWindow};
    ImProc::ResultsViewer mResultsViewerTab{&bShowResultsViewerWindow};
};

Walnut::Application *Walnut::CreateApplication(int argc, char **argv)
{
    Walnut::ApplicationSpecification spec;
    spec.Name = "Walnut Example";

    Walnut::Application *app = new Walnut::Application(spec);
    auto layer = std::make_shared<ExampleLayer>();
    auto layerptr = layer.get();
    app->PushLayer(layer);
    app->SetMenubarCallback(
        [app, layerptr]()
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Exit"))
                {
                    app->Close();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Show"))
            {
                if (ImGui::MenuItem("Demo"))
                {
                    layerptr->bShowDemoWindow = true;
                }
                if (ImGui::MenuItem("Plots Dema"))
                {
                    layerptr->bShowDemoWindow = true;
                }
                ImGui::EndMenu();
            }
        });
    return app;
}
