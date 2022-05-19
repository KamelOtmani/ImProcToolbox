#include "ImageViewerPannel.h"
#include "Utility.h"
#include <algorithm>
#include <cmath>
#include <imgui.h>

struct DrawingInformations
{
    ImVec2 pos0, pos1;
    ImVec2 p0, p1;
};

ImageViewerPannel::ImageViewerPannel(bool showOnlyPreview )
    : bShwoOnlyPreviewImage(showOnlyPreview)
{
}

void ImageViewerPannel::UpdateViewImage(Walnut::Image *image)
{
    mImage = image;
}

void ImageViewerPannel::UpdateProcessedImage(Walnut::Image *image)
{
    mProcessedImage = image;
}

DrawingInformations GetImageDrawingInfo(Walnut::Image *image)
{
    DrawingInformations draw_info;

    if (image)
    {

        static ImVec2 offset(0.0f, 0.0f);
        static float zoom = 5.0f;
        static ImVec2 p0;
        static ImVec2 p1;

        auto size = ImGui::GetContentRegionAvail();
        ImGui::InvisibleButton("##empty", size);
        if (ImGui::IsItemActive() &&
            ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            offset.x += ImGui::GetIO().MouseDelta.x;
            offset.y += ImGui::GetIO().MouseDelta.y;
        }
        // no negative values maybe and dont excede ~x500 zoom
        zoom = std::clamp(zoom + ImGui::GetIO().MouseWheel * 0.1, 0.0, 16.0);
        const ImVec2 pos0 = ImGui::GetItemRectMin();
        const ImVec2 pos1 = ImGui::GetItemRectMax();

        auto window_center = ImVec2{pos0.x + size.x / 2, pos0.y + size.y / 2};
        // auto image_center = ImVec2{ window_center.x + offset.x
        // ,window_center.y + offset.y  }; auto image_center = window_center;

        p0 = ImVec2((window_center.x - (image->GetWidth() / 2)),
                    (window_center.y) - (image->GetHeight() / 2));
        p1 = ImVec2((p0.x + image->GetWidth()), (p0.y + image->GetHeight()));

        auto t0 = ImVec2{p0.x - window_center.x, p0.y - window_center.y};
        auto t1 = ImVec2{p1.x - window_center.x, p1.y - window_center.y};

        auto zoom_function = [](float zoom_value)
        { return std::exp(0.5f * zoom_value - 3.0f); };

        auto finalp0 =
            ImVec2{window_center.x + t0.x * zoom_function(zoom) + offset.x,
                   window_center.y + t0.y * zoom_function(zoom) + offset.y};
        auto finalp1 =
            ImVec2{window_center.x + t1.x * zoom_function(zoom) + offset.x,
                   window_center.y + t1.y * zoom_function(zoom) + offset.y};

        auto image_center = ImVec2{finalp0.x + image->GetWidth() / 2,
                                   finalp1.y + image->GetHeight() / 2};
        draw_info = DrawingInformations{pos0, pos1, finalp0, finalp1};
    }
    return draw_info;
}
void DrawDragabbleImage(Walnut::Image *image, DrawingInformations info,
                        bool bDebug = false)
{
    if (image)
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImGui::PushClipRect(info.pos0, info.pos1, true);
        auto bg_color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
        draw_list->AddImage(image->GetDescriptorSet(), info.p0, info.p1);
        if (bDebug)
        {
            // draw_list->AddCircleFilled(window_center, 5.0f, IM_COL32(220, 30,
            // 25, 255)); draw_list->AddCircleFilled(image_center, 5.0f,
            // IM_COL32(30, 30, 250, 255)); draw_list->AddLine(window_center, {
            // t0.x - window_center.x,t0.y - window_center.y }, IM_COL32(20, 50,
            // 250, 255)); draw_list->AddLine(window_center, { t1.x -
            // window_center.x,t1.y - window_center.y }, IM_COL32(20, 50, 250,
            // 255)); draw_list->AddCircleFilled(p0, 5.0f, IM_COL32(30, 150, 25,
            // 255)); draw_list->AddCircleFilled(p1, 5.0f, IM_COL32(30, 150, 25,
            // 255));
        }
        ImGui::PopClipRect();
    }
}

void ImageViewerPannel::onUIRender()
{
    if (bShwoOnlyPreviewImage)
    {
        auto draw_info = GetImageDrawingInfo(mImage);
        DrawDragabbleImage(mImage, draw_info);
    }
    else
    {
        // ImGui::Begin("Preview");
        //  need to wait two frames so we can get the window size
        static int bFrame = 0;
        if (bFrame < 2)
        {
            ImageViewerWidth = ImGui::GetContentRegionAvail().x / 2;
            bFrame++;
        }
        // pannel height
        float h = -1;
        ImGui::Text("% f", ImageViewerWidth);

        // DrawSplitter(true, 80.0f, &sz1, &sz2, 80, 80, h); // code above

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::BeginChild("Original", ImVec2(ImageViewerWidth, h), true);

        auto draw_info = GetImageDrawingInfo(mImage);
        DrawDragabbleImage(mImage, draw_info);
        ImGui::EndChild();
        ImGui::SameLine();

        ImGui::InvisibleButton("##vsplitter", ImVec2(8.0f, -1));

        if (ImGui::IsItemActive())
            ImageViewerWidth += ImGui::GetIO().MouseDelta.x;

        ImGui::SameLine();
        ImGui::PopStyleVar();
        ImGui::BeginChild("Processed", ImVec2(-1, h), true);
        draw_info = GetImageDrawingInfo(mProcessedImage);
        DrawDragabbleImage(mProcessedImage, draw_info);
        ImGui::EndChild();
        // ImGui::End();
    }
}
