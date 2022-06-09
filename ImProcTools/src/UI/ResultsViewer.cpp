#include "ResultsViewer.hpp"
#include "IO/FileDialogue.h"
#include "IO/ImageIO.hpp"
#include "ImProcProcessing/detection_algorithms.hpp"
#include <imgui.h>
#include <implot.h>
#include <thread>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <ranges>

namespace ImProc
{
namespace fs = std::filesystem;
ResultsViewer::ResultsViewer(bool *ShowWindow) : bShowWindow(ShowWindow) {}

void ResultsViewer::UpdateImageViewer()
{
    auto mCVImage = Utility::ImportImage(mPaths[mCurrentFrame]);
    mPreviewImage = Utility::CreateImageFromOpencv(mCVImage);
    mImageViewerPannel.UpdateViewImage(mPreviewImage.get());
}

std::vector<CSVRow> ResultsViewer::getResults(const fs::path &path)
{

    // Create a vector of <string, int vector> pairs to store the result
    std::vector<CSVRow> result;

    // Create an input filestream
    std::ifstream myFile(path);

    // Make sure the file is open
    if (!myFile.is_open())
        assert(false); // file dont exist

    // Helper vars
    std::string line, colname;
    float val{};

    // Read the column names
    if (myFile.good())
    {
        // Extract the first line in the file
        std::getline(myFile, line);
        // ignore the comments
        while (line.starts_with("#"))
            std::getline(myFile, line);

        // Create a stringstream from line
        std::stringstream ss(line);

        // Extract each column name
        while (std::getline(ss, colname, ','))
        {

            // Initialize and add <colname, int vector> pairs to result
            result.push_back({colname, std::vector<float>{}});
        }
    }
    // Read data, line by line while (std::getline(myFile, line))
    while (std::getline(myFile, line))
    {
        // Create a stringstream of the current line
        std::stringstream ss(line);

        // Keep track of the current column index
        int colIdx = 0;

        // Extract each float
        std::stringstream copy(line);
        std::string str{};
        if (copy >> str)
        {
            if (str.find("nan") != std::string::npos)
            {
            }
        }
        while (ss >> val)
        {

            // Add the current integer to the 'colIdx' column's values vector
            result.at(colIdx).second.push_back(val);

            // If the next token is a comma, ignore it and move on
            if (ss.peek() == ',')
                ss.ignore();

            // Increment the column index
            colIdx++;
        }
    }

    // Close file
    myFile.close();
    // return ImProc::Results();
    return result;
}

struct ResultsData
{
    std::vector<int> Frames;
    std::vector<float> Area;
    std::vector<float> X;
    std::vector<float> Y;
};

ResultsData getData(const std::vector<CSVRow> &in)
{
    const auto &frames = in[0].second;
    return {std::vector<int>(frames.begin(), frames.end()), in[1].second,
            in[2].second, in[3].second};
}

void ResultsViewer::DrawWindow()
{
    ImGui::Begin("Config", bShowWindow);

    // Paths
    std::vector<char> buffer(100);
    if (ImGui::InputTextWithHint("Pattern", "Img*.tif", buffer.data(),
                                 buffer.size()))
    {
        mPattern = {buffer.data()};
    }
    if (ImGui::Button("Set Batches Folder", ImVec2{150, 50}))
    {
        // mPaths = Utility::FileDialogue::BrowseFolder({});
        mFolders = Utility::FileDialogue::BrowseFolder();
        mActivatedFolders = std::vector<int>(mFolders.size(), 1);
        mFoldersSizes = std::vector<size_t>(mFolders.size(), 0);
        int i = 0;
        for (const auto &folder : mFolders)
        {
            auto pattern = (folder / mPattern).string();
            auto path = utility::GetImageSequence(pattern);
            mFoldersSizes[i] = path.size();
            i++;
        }
        if (!mFolders.empty())
        {
            bShouldCalculateSizes = true;
            if (!fs::is_empty(mFolders[0]))
            {
                SetupImageViwer(0);
            }
        }
    }
    ImGui::End();

    // Images List
    drawBatches();

    // image Viwer
    ImGui::Begin("Viewer");
    if (ImGui::Button("Update"))
    {
        // UpdateImageViewer();
    }
    // mImageViewerPannel.onUIRender();
    ImPlot::BeginPlot("Main Window", {-1, -1});
    // ploting the image
    if (mPreviewImage)
    {

        auto [frames, areas, xs, ys] = getData(mResults);
        ImPlot::PlotImage("Input Image", mPreviewImage->GetDescriptorSet(),
                          {0, 0},
                          {(double)mPreviewImage->GetWidth(),
                           (double)mPreviewImage->GetHeight()});
        if (mCurrentFrame < mParticles.size())
        {
            int i = 0;
            for (auto &[x, y] : mParticles[mCurrentFrame])
            {
                ImVec2 cntr = ImPlot::PlotToPixels(ImPlotPoint(x, y),
                                                   IMPLOT_AUTO, IMPLOT_AUTO);
                ImPlot::GetPlotDrawList()->AddCircleFilled(
                    cntr, 2, IM_COL32(255, 100, 50, 255), 10);
                i++;
            }
        }
        if (mCurrentFrame < mContours.size())
        {
            int i = 0;
            for (auto &contour : mContours[mCurrentFrame])
            {
                std::vector<ImVec2> ImVeccontour{};
                for (auto& point : contour)
                {
                    // transform to pixels
                    ImVeccontour.push_back(
                        ImPlot::PlotToPixels(ImPlotPoint({point.x, point.y}),
                                             IMPLOT_AUTO, IMPLOT_AUTO));
                }
                ImPlot::GetPlotDrawList()->AddPolyline(
                    ImVeccontour.data(), contour.size(), IM_COL32(128, 0, 255, 255),
                    ImDrawFlags_Closed, 1.5);
                i++;
            }
        }
    }

    ImPlot::EndPlot();
    ImGui::End();

    // controles
    drawControles();
}
void ResultsViewer::SetupImageViwer(int i)
{

    mCurrentSelectedBatch = i;
    mPaths = utility::GetImageSequence(mFolders[i].string());
    mCurrentFrame = 0;
    // sets the bg image for the image viwer if it exist
    if (fs::exists(mFolders[mCurrentSelectedBatch] / mBackgroundName))
    {
        auto bg = utility::ImportGrayscaleImage(
            (mFolders[mCurrentSelectedBatch] / mBackgroundName).string(), true);
        mBackgroundImage = Utility::CreateImageFromOpencv(bg);
        mImageViewerPannel.UpdateProcessedImage(mBackgroundImage.get());
    }
    else
    {
        mBackgroundImage.reset();
        mImageViewerPannel.UpdateProcessedImage(nullptr);
    }
    // update the image , need to do this first because we use the image in the
    // next function
    UpdateImageViewer();
    // check if we have results
    auto resultPath = mFolders[mCurrentSelectedBatch] / "output/trajectory.csv";
    auto contoursPath =
        mFolders[mCurrentSelectedBatch] / "output/contours_trajectory.csv";
    // Beware this code is trash :)
    // a csv Parser or an Python interpreter would be awesome
    if (fs::exists(resultPath))
    {
        mResults = getResults(resultPath);
        auto contoursResults = getResults(contoursPath);
        auto [frames, areas, xs, ys] = getData(mResults);
        mParticles = {};
        {
            int i = 0;
            int lastFrame = -1;
            for (const auto &frame : frames)
            {
                // is this a new particle in the smae frame ?
                if (frame == lastFrame)
                {
                    mParticles.back().push_back(
                        Point{xs[i], mPreviewImage->GetHeight() - ys[i]});
                }
                // new frame
                else
                {
                    mParticles.push_back(
                        {{xs[i], mPreviewImage->GetHeight() - ys[i]}});
                }
                i++;
                lastFrame = frame;
            }
        }
        {
            mContours = {};
            int i = 0;
            int lastFrame = -1;
            int lastId = -1;
            auto [frames, id, xs, ys] = getData(contoursResults);
            for (const auto &frame : frames)
            {
                auto contour = Contour{};
                // is this a new particle in the smae frame ?
                if (frame == lastFrame)
                {
                    auto &particle = mContours.back();
                    // a new point to the same contour ?
                    if (int(id[i]) == lastId)
                    {
                        particle.back().push_back(
                            Point{xs[i], mPreviewImage->GetHeight() - ys[i]});
                    }
                    else // new contour
                    {
                        particle.push_back(
                            {Point{xs[i], mPreviewImage->GetHeight() - ys[i]}});
                    }
                    lastId = int(id[i]);
                }
                // new frame
                else
                {
                    lastId = int(id[i]);
                    mContours.push_back(
                        {{Point{xs[i], mPreviewImage->GetHeight() - ys[i]}}});
                }
                i++;
                lastFrame = frame;
            }
        }
    }
}
void ResultsViewer::drawBatches()
{
    ImGui::Begin("Batches");
    // ImGui::Text("Batches");
    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    // table flags
    const ImGuiTableFlags flags =
        ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_NoBordersInBody;

    if (ImGui::BeginTable("BatchesTable", 4, flags))
    {
        // The first column will use the default _WidthStretch when ScrollX
        // is Off and _WidthFixed when ScrollX is On
        ImGui::TableSetupColumn("Enabled", ImGuiTableColumnFlags_WidthFixed,
                                TEXT_BASE_WIDTH * 5.0f);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn("Frames", ImGuiTableColumnFlags_WidthFixed,
                                TEXT_BASE_WIDTH * 7.0f);
        ImGui::TableSetupColumn("has Background ?",
                                ImGuiTableColumnFlags_WidthFixed,
                                TEXT_BASE_WIDTH * 5.0f);
        ImGui::TableHeadersRow();

        int i = 0;
        for (const auto &folder : mFolders)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            auto cdt = mActivatedFolders.at(i) != 0;
            ImGui::PushID(4651 + i);
            if (ImGui::Checkbox("##test", &cdt))
                mActivatedFolders[i] = cdt;
            ImGui::PopID();
            ImGui::TableNextColumn();
            // here we are not using Text() as it doesnt allow the
            // IsItemClicked()
            auto flags =
                ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Bullet;
            if (i == mCurrentSelectedBatch)
                flags |= ImGuiTreeNodeFlags_Selected;
            if (ImGui::TreeNodeEx(folder.stem().string().c_str(), flags))
                ImGui::TreePop();
            if (ImGui::IsItemClicked())
            {
                SetupImageViwer(i);
                UpdateImageViewer();
            }
            // ImGui::Text(path.filename().string().c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%d", mFoldersSizes[i]);
            ImGui::TableNextColumn();
            bool bHasBg =
                fs::exists(mFolders[mCurrentSelectedBatch] / mBackgroundName);
            ImGui::Checkbox("bg", &bHasBg);
            i++;
        }
    }
    bShouldCalculateSizes = false;

    ImGui::EndTable();

    ImGui::End();
}
void ResultsViewer::drawControles()
{
    ImGui::Begin("Controles");
    if (ImGui::Button("<|<|", {50, 50}))
    {
        mPlayState = -2;
    }
    ImGui::SameLine();
    if (ImGui::Button("<|", {50, 50}))
    {
        mCurrentFrame--;
        UpdateImageViewer();
    }
    ImGui::SameLine();
    if (ImGui::Button("|>||", {50, 50}))
    {
        if (mPlayState != 0)
            mPlayState = 0;
        else if (mPlayState == 0)
            mPlayState = 1;
    }
    ImGui::SameLine();
    if (ImGui::Button("|>", {50, 50}))
    {
        mCurrentFrame++;
        UpdateImageViewer();
    }
    ImGui::SameLine();
    if (ImGui::Button("|>|>", {50, 50}))
    {
        mPlayState = 2;
    }

    if (!mFolders.empty())
    {
        if (ImGui::SliderInt("Frame", &mCurrentFrame, 0, mPaths.size() - 1))
        {
            UpdateImageViewer();
        }
    }
    ImGui::End();
}
void ResultsViewer::Update()
{
    if (mPlayState != 0)
    {
        // loop back
        mCurrentFrame = (mCurrentFrame + mPlayState) % mPaths.size();
        UpdateImageViewer();
    }
}

} // namespace ImProc