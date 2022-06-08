#pragma once
#include "ImProcProcessing/detection_algorithms.hpp"
#include "ImageViewerPannel.h"
#include <ImProcProcessing/results.hpp>
#include <filesystem>
#include <memory>
#include <vector>

namespace cv
{
class Mat;
}

namespace ImProc
{
struct ImageInfo;

class BackgroundExtractorTab
{
  public:
    BackgroundExtractorTab(bool *ShowWindow);

    void DrawWindow();

    void SetupImageViwer(int i);

    void drawBatches();

    void drawControles();

    void Update();

    void UpdateImageViewer();

    void drawProcessWindow();

  private:
    bool *bShowWindow = nullptr;
    ImageViewerPannel mImageViewerPannel{false};
    std::vector<std::filesystem::path> mFolders;
    std::vector<int> mActivatedFolders;
    std::vector<std::filesystem::path> mPaths;
    std::vector<std::size_t> mFoldersSizes;
    std::vector<Results> mBatchesResults;
    int mCurrentSelectedBatch = 0;
    int mCurrentFrame = 0;
    std::string mPattern{"Img*.tif"};
    std::string mBackgroundName{"bg.tif"};
    std::unique_ptr<Walnut::Image> mPreviewImage;
    std::unique_ptr<Walnut::Image> mBackgroundImage;
    bool bShouldCalculateSizes = true;
    int mPlayState = 0;
    std::atomic<int> mProgress{};
};
} // namespace ImProc