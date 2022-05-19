#include "ConfigurationTab.hpp"
#include <imgui.h>
#include "IO/FileDialogue.h"
#include "IO/ImageIO.hpp"

namespace ImProc
{
namespace fs = std::filesystem;
ConfigurationTab::ConfigurationTab(bool *ShowWindow) : bShowWindow(ShowWindow) {
}

void ConfigurationTab::DrawWindow()
{
    ImGui::Begin("Config", bShowWindow);

    // Paths
    ImGui::BeginChild("Paths", {-1,100});
    if (ImGui::Button("Import Image", ImVec2{150, 50}))
    {
        mPaths = Utility::FileDialogue::OpenFile(".TIF;.TIFF",true);
        
    if (!mPaths.empty())
        {
            if (!fs::is_empty(mPaths[0]))
            {
                auto mCVImage = Utility::ImportImage(mPaths[0]);
                mPreviewImage = Utility::CreateImageFromOpencv(mCVImage);
                mImageViewerPannel.UpdateViewImage(mPreviewImage.get());
            }
        }
    }
    if (!mPaths.empty())
    {
        ImGui::SameLine();
        if (ImGui::SliderInt("Frame", &mCurrentFrame, 0, mPaths.size() - 1))
        {
            auto &path = mPaths[mCurrentFrame];
            if (!fs::is_empty(path))
            {
                auto mCVImage = Utility::ImportImage(path);
                mPreviewImage = Utility::CreateImageFromOpencv(mCVImage);
                mImageViewerPannel.UpdateViewImage(mPreviewImage.get());
            }
        }
    }
    ImGui::EndChild();

    // Image List
    ImGui::BeginChild("List", {300, -1});
    int i = 0; 
    for (const auto &path : mPaths)
    {
        ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_Leaf |
                                        ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet|
            ImGuiTreeNodeFlags_SpanAvailWidth;
        ;
        const bool is_selected = (i == mCurrentFrame);
        if (is_selected)
            node_flags |= ImGuiTreeNodeFlags_Selected;

        ImGui::TreeNodeEx((void *)(intptr_t)i, node_flags,
                          path.filename().string().c_str());
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        {
            mCurrentFrame = i;
            auto mCVImage = Utility::ImportImage(path);
            mPreviewImage = Utility::CreateImageFromOpencv(mCVImage);
            mImageViewerPannel.UpdateViewImage(mPreviewImage.get());
        }
        //ImGui::Text(path.filename().string().c_str());
        i++;
    }
    ImGui::EndChild();
    ImGui::SameLine();

    // image Viwer
    ImGui::BeginChild("Viewer", {-1,-1});
    mImageViewerPannel.onUIRender();
    ImGui::EndChild();



    ImGui::End();
}
}