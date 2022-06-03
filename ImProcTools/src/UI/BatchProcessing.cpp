#include "BatchProcessing.hpp"
#include "IO/FileDialogue.h"
#include "IO/ImageIO.hpp"
#include "ImProcProcessing/detection_algorithms.hpp"
#include <imgui.h>
#include <thread>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

namespace ImProc
{
namespace fs = std::filesystem;
BatchProcessingTab::BatchProcessingTab(bool *ShowWindow)
    : bShowWindow(ShowWindow)
{
}

void BatchProcessingTab::UpdateImageViewer()
{
    auto mCVImage = Utility::ImportImage(mPaths[mCurrentFrame]);
    mPreviewImage = Utility::CreateImageFromOpencv(mCVImage);
    mImageViewerPannel.UpdateViewImage(mPreviewImage.get());
    if (bProcessImage)
    {
        auto inputImage =
            utility::ImportGrayscaleImage(mPaths[mCurrentFrame].string());
        auto outputfolderPath =
            fs::path(mPaths[mCurrentFrame]).parent_path() / "output ";
        auto backgroundImage =
            utility::ConvertToFloatImage(utility::ImportGrayscaleImage(
                (fs::path(mPaths[mCurrentFrame]).parent_path() / "bg.tif")
                    .string()));
        auto processedImage = threshold_detection::ProcessImage(
            utility::RemoveBackground(inputImage, backgroundImage),
            mGlobalParams);
        mProcessedImage = Utility::CreateImageFromOpencv(
            Utility::ConvertToRGBA(processedImage));
        mImageViewerPannel.UpdateProcessedImage(mProcessedImage.get());
    }
}

void BatchProcessingTab::drawParamsWindow()
{
    ImGui::Begin("Parameters");
    ImGui::DragInt("blurKernelSize", (int *)&mGlobalParams.blurKernelSize, 2, 0,
                   9);
    ImGui::DragInt("threshold", (int *)&mGlobalParams.threshold, 1, 0, 255);
    ImGui::DragInt("minSize", (int *)&mGlobalParams.minSize, 1, 1, 20);
    // ImGui::DragInt("blurKernelSize", (int *)&mGlobalParams.blurKernelSize);
    ImGui::End();
}
void BatchProcessingTab::drawProcessWindow()
{
    ImGui::Begin("Process");
    ImGui::Checkbox("Output Intermidiate Results", &bOutputIntermediateImages);

        if (ImGui::Button("Process Selected Batches"))
    {
        // First, check if we have all the background images
        int i = 0;
        for (const auto &folder : mFolders)
        {
            if (mActivatedFolders[i] != 0)
            {
                auto backgroundImage =
                    utility::ImportGrayscaleImage((folder / "bg.tif").string());

                if (backgroundImage.empty())
                    mBatchesLackingBackground.push_back(folder.stem().string());
            }
            i++;
        }

        if (!mBatchesLackingBackground.empty())
        {
            ImGui::OpenPopup("Warning");
        }
        else
        {

            mBatchesResults.reserve(mFolders.size());
            int i = 0;
            for (const auto &folder : mFolders)
            {
                mProgress = 0;
                if (mActivatedFolders[i] != 0)
                {
                    auto pattern = (folder / mPattern).string();
                    auto path = utility::GetImageSequenceInfo(pattern);
                    auto outputfolderPath =
                        fs::path(path[0].path).parent_path() / "output ";
                    if (!fs::exists(outputfolderPath))
                        fs::create_directory(outputfolderPath);
                    auto backgroundImage = utility::ImportGrayscaleImage(
                        (folder / "bg.tif").string());
                    // convert the background image once for all the images
                    auto fBackgroundImage =
                        utility::ConvertToFloatImage(backgroundImage);
                    const auto coreNum = std::thread::hardware_concurrency();
                    const auto cache_size = path.size() / coreNum + 1;
                    auto threads_paths = std::vector<std::vector<ImageInfo>>{};
                    threads_paths.reserve(coreNum);
                    auto threads_results = std::vector<Results>{};
                    threads_results.reserve(coreNum);
                    auto threads = std::vector<std::thread>{};
                    threads.reserve(coreNum);

                    // split the workload
                    for (size_t i = 0; i < path.size(); i += cache_size)
                    {
                        auto last = std::min(path.size(), i + cache_size);
                        threads_paths.emplace_back(path.begin() + i,
                                                   path.begin() + last);
                        threads_results.emplace_back(last - i);
                    }

                    // fire the threads
                    for (unsigned int i = 0; i < coreNum; i++)
                    {
                        const auto &lPaths = threads_paths[i];
                        Results &results = threads_results[i];
                        threads.emplace_back(std::thread(
                            [&]() {
                                WorkerThred(lPaths, &results, fBackgroundImage);
                            }));
                    }

                    // wait for all the treads to finish
                    for (auto &thread : threads)
                    {
                        thread.join();
                    }
                    auto results = Results{};
                    for (auto &result : threads_results)
                    {
                        results.AppendResults(result);
                    }
                    mBatchesResults.push_back(results);
                    results.ExportToCSV(
                        fs::path(path[0].path).parent_path().string() +
                            std::string("//output"),
                        "trajectory.csv");
                    // results.ExportToCSV(outputfolderPath.string(),
                    //                    "trajectory.csv");
                }
                i++;
                mOverallProgress = i / mFolders.size();
            }
        }
    }
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Warning", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize))
    {
        // ImGui::BeginPopupContextItem();
        ImGui::Text("Warning, the following batches dont have a \"bg.tif\" "
                    "(background) file");
        for (const auto &batch : mBatchesLackingBackground)
        {
            ImGui::BulletText(batch.c_str());
        }
        auto off = ImGui::GetContentRegionAvail().x / 2;
        ImGui::SetCursorPosX(off - 60);
        if (ImGui::Button("Ok", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::ProgressBar(mBatchProgress);
    ImGui::ProgressBar(mOverallProgress);
    ImGui::End();
}

void BatchProcessingTab::DrawWindow()
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
                // auto mCVImage = Utility::ImportImage(mFolders[0]);
                // mPreviewImage = Utility::CreateImageFromOpencv(mCVImage);
                // mImageViewerPannel.UpdateViewImage(mPreviewImage.get());
            }
        }
    }
    ImGui::End();

    // Images List
    drawBatches();

    // image Viwer
    ImGui::Begin("Viewer");
    ImGui::Checkbox("Show Processed Image", &bProcessImage);
    if (ImGui::Button("Update"))
    {
        UpdateImageViewer();
    }
    mImageViewerPannel.onUIRender();
    ImGui::End();

    // controles
    drawControles();

    // Process
    drawProcessWindow();

    // results
    drawResults();

    // params
    drawParamsWindow();
}
void BatchProcessingTab::drawBatches()
{
    ImGui::Begin("Batches");
    // ImGui::Text("Batches");
    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    // table flags
    const ImGuiTableFlags flags =
        ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_NoBordersInBody;

    if (ImGui::BeginTable("BatchesTable", 3, flags))
    {
        // The first column will use the default _WidthStretch when ScrollX
        // is Off and _WidthFixed when ScrollX is On
        ImGui::TableSetupColumn("Enabled", ImGuiTableColumnFlags_WidthFixed,
                                TEXT_BASE_WIDTH * 5.0f);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn("Frames", ImGuiTableColumnFlags_WidthFixed,
                                TEXT_BASE_WIDTH * 12.0f);
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
            bool open = ImGui::TreeNodeEx(folder.stem().string().c_str(),
                                          ImGuiTreeNodeFlags_SpanFullWidth);
            if (open)
            {
                int j = 0;
                if (mFolders.size() > mCurrentSelectedBatch)
                {
                    if (fs::is_directory(mFolders[mCurrentSelectedBatch]))
                    {
                        auto pattern = (mFolders[i] / mPattern).string();
                        mPaths = utility::GetImageSequence(pattern);
                        mFoldersSizes[i] = mPaths.size();
                        for (const auto &path : mPaths)
                        {
                            ImGuiTreeNodeFlags node_flags =
                                ImGuiTreeNodeFlags_Leaf |
                                ImGuiTreeNodeFlags_NoTreePushOnOpen |
                                ImGuiTreeNodeFlags_SpanAvailWidth;
                            ;
                            const bool is_selected = (j == mCurrentFrame);
                            if (is_selected)
                                node_flags |= ImGuiTreeNodeFlags_Selected;

                            ImGui::TreeNodeEx((void *)(intptr_t)i, node_flags,
                                              path.stem().string().c_str());
                            if (ImGui::IsItemClicked() &&
                                !ImGui::IsItemToggledOpen())
                            {
                                mCurrentFrame = j;
                                auto mCVImage = Utility::ImportImage(path);
                                mPreviewImage =
                                    Utility::CreateImageFromOpencv(mCVImage);
                                mImageViewerPannel.UpdateViewImage(
                                    mPreviewImage.get());
                            }
                            // ImGui::Text(path.filename().string().c_str());
                            j++;
                        }
                    }
                }
                ImGui::TreePop();
            }

            if (ImGui::IsItemClicked())
            {
                mCurrentSelectedBatch = i;
            }
            // ImGui::Text(path.filename().string().c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%d", mFoldersSizes[i]);
            i++;
        }
    }
    bShouldCalculateSizes = false;

    ImGui::EndTable();

    ImGui::End();
}
void BatchProcessingTab::drawControles()
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
void BatchProcessingTab::Update()
{
    if (mPlayState != 0)
    {
        // loop back
        UpdateImageViewer();
        mCurrentFrame = (mCurrentFrame + mPlayState) % mPaths.size();
    }
}

void BatchProcessingTab::WorkerThred(const std::vector<ImageInfo> &paths,
                                     Results *results,
                                     const cv::Mat &backgroundImage)
{
    auto countours = std::vector<ContourInfo>{};
    countours.reserve(paths.size());

    // auto params = threshold_detection::ThresholdDetectionParameters{};
    // params.blurKernelSize = 9;
    // params.threshold = 120;
    // params.minSize = 15;
    // params.ClosingShape = cv::MorphShapes::MORPH_RECT;
    for (const auto &[path, frame] : paths)
    {
        // bar.progress(i,paths.size());
        auto inputImage = utility::ImportGrayscaleImage(path);
        auto homogeniousImage =
            utility::RemoveBackground(inputImage, backgroundImage);
        // threshold_detection::ProcessImageInPlace(homogeniousImage,
        // params); auto& result = homogeniousImage;
        auto result =
            threshold_detection::ProcessImage(homogeniousImage, mGlobalParams);
        if (bOutputIntermediateImages)
        {
            auto filename = std::filesystem::path(path).stem();
            auto output_path = fs::path(path).parent_path().string() +
                               std::string("//output//cpp_frame_") +
                               filename.string() /*+ std::to_string(frame) */
                               + ".jpg";
            cv::imwrite(output_path, result);
        }
        auto contours = utility::FindContours(result);
        if (!contours.empty())
        {
            // We export the found contours as seperate positions that have
            // the same frame index
            for (const auto &contour : contours)
            {
                ParticleInfo info{};
                info.position = utility::ComputeCenterOfMass(contour);
                info.frame = frame;
                info.area = (float)utility::ComputeArea(contour);
                // info.contour = contour;
                results->AddParticle(info);
            }
            countours.emplace_back(ContourInfo{frame, contours});
        }
        std::cout << mProgress++ << "\n";
        mBatchProgress = mProgress / paths.size();
    }
    results->AddContours(countours);
}
void BatchProcessingTab::drawResults()
{
    ImGui::Begin("Output");
    ImGui::Text("Result size %d", mBatchesResults.size());

    for (const auto &result : mBatchesResults)
    {
        ImGui::BulletText("%d Particules detectées",
                          result.GetParticles().size());
    }
    ImGui::End();
}
} // namespace ImProc