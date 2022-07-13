#include "BackgroundExtractor.hpp"
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
BackgroundExtractorTab::BackgroundExtractorTab(bool *ShowWindow)
    : bShowWindow(ShowWindow)
{
}

void BackgroundExtractorTab::UpdateImageViewer()
{
    auto mCVImage = Utility::ImportImage(mPaths[mCurrentFrame]);
    mPreviewImage = Utility::CreateImageFromOpencv(mCVImage);
    mImageViewerPannel.UpdateViewImage(mPreviewImage.get());
}

void BackgroundExtractorTab::drawProcessWindow()
{
    ImGui::Begin("Process");
    ImGui::DragInt("Num Images", &mMaxImages);
    std::vector<char> buffer(100);
    if (ImGui::InputTextWithHint("Background image Name", "bg.tif", buffer.data(),
                                 buffer.size()))
    {
        mBackgroundName = {buffer.data()};
    }

    if (ImGui::Button("Extract Backgrounds"))
    {
        // First, check if we have all the background images
        int j = 0;
        for (const auto &folder : mFolders)
        {
            std::cout << "Processing " << folder.stem() << "..." << j / mFolders.size()
                      << "\n";
            mProgress = 0;
            if (mActivatedFolders[j] != 0)
            {
                auto pattern = (folder / mPattern).string();
                auto paths = utility::GetImageSequenceInfo(pattern);

                auto paths_to_calculate = std::vector < ImProc::ImageInfo> (paths.begin(),
                   paths.begin() + std::min((int)paths.size(), mMaxImages));


                // prepare the threads
                const auto coreNum = std::thread::hardware_concurrency();
                const auto cache_size = paths_to_calculate.size() / coreNum + 1;
                auto threads_paths = std::vector<std::vector<ImageInfo>>{};
                threads_paths.reserve(coreNum);
                auto threads_results = std::vector<Results>{};
                threads_results.reserve(coreNum);
                auto threads = std::vector<std::thread>{};
                threads.reserve(coreNum);
                auto bgImages = std::vector<cv::Mat>{};

                for (unsigned int i = 0; i < coreNum; i++)
                {
                    bgImages.emplace_back(cv::Mat{});
                }

                // split the workload
                for (size_t i = 0; i < paths_to_calculate.size();
                     i += cache_size)
                {
                    auto last =
                        std::min(paths_to_calculate.size(), i + cache_size);
                    threads_paths.emplace_back(paths_to_calculate.begin() + i,
                                               paths_to_calculate.begin() +
                                                   last);
                    threads_results.emplace_back(last - i);
                }

                // fire the threads
                // TODO : fix the same issue in the batch processing
                // (replacing corenum by the threads size in case we have lower images than to fill the threads)
                for (unsigned int i = 0; i < threads_paths.size(); i++)
                {
                    const auto &lPaths = threads_paths[i];
                    auto ibg = &bgImages[i];
                    auto id = (int)i;
                    threads.emplace_back(std::thread(
                        [&, ibg, id]() { WorkerThread(lPaths, ibg, id); }));
                }

                // wait for all the treads to finish
                for (auto &thread : threads)
                {
                    thread.join();
                }

                auto fbg = bgImages[0];
                for (const auto& image : bgImages)
                {
                    fbg += image;
                }

                fbg /= ((int)bgImages.size() + 1);
                //cv::normalize(fbg, fbg);
                utility::ExportGrayscaleImage(
                    utility::ConvertToUintImage(fbg,255),
                    (folder / mBackgroundName).string());
            }
            j++;
        }
    }
    ImGui::End();
}

void BackgroundExtractorTab::WorkerThread(const std::vector<ImageInfo> &paths,cv::Mat* bgImage,int id)
{    
    int j = 0;
    auto bg = utility::ConvertToFloatImage(utility::ImportGrayscaleImage(paths[0].path),
                                           1 / 255.f);
    for (const auto &[path, frame] : paths)
    {
        // bar.progress(i,paths.size());
        auto inputImage = utility::ConvertToFloatImage(
            utility::ImportGrayscaleImage(path), 1 / 255.f);

        bg +=  inputImage;
        j++;
        if (j % (paths.size() / 10 ) == 0)
        {
            std::cout << "thread " << id << " "
                      << 100 * (j / (float)paths.size())
                                                  << " %\n";
        }
    }
    *bgImage = bg / (paths.size() + 1);
    std::cout << "thread " << id << "done ! .......\n";
}

void BackgroundExtractorTab::DrawWindow()
{
    ImGui::Begin("Config", bShowWindow);

    // Paths
    std::vector<char> buffer(100);
    if (ImGui::InputText("Pattern", mPattern.data(), mPattern.size()))
    {
        //mPattern = {buffer.data()};
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
        UpdateImageViewer();
    }
    mImageViewerPannel.onUIRender();
    ImGui::End();

    // controles
    drawControles();

    // Process
    drawProcessWindow();
}
void BackgroundExtractorTab::SetupImageViwer(int i)
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
    UpdateImageViewer();
}
void BackgroundExtractorTab::drawBatches()
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
            // here we are not using Text() as it doesnt allow the IsItemClicked() 
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
void BackgroundExtractorTab::drawControles()
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
void BackgroundExtractorTab::Update()
{
    if (mPlayState != 0)
    {
        // loop back
        UpdateImageViewer();
        mCurrentFrame = (mCurrentFrame + mPlayState) % mPaths.size();
    }
}

} // namespace ImProc