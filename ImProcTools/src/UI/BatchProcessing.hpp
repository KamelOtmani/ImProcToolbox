#pragma once
#include "ImProcProcessing/detection_algorithms.hpp"
#include "ImageViewerPannel.h"
#include <ImProcProcessing/results.hpp>
#include <filesystem>
#include <memory>
#include <vector>

// forward declarations
namespace cv
{
class Mat;
}

namespace ImProc
{
struct ImageInfo;

// BatchProcessing Tab
class BatchProcessingTab
{
  public:
    BatchProcessingTab(bool *ShowWindow);

    void DrawWindow();

    void drawBatches();

    void drawControles();

    void Update();

    void UpdateImageViewer();

    void drawProcessWindow();

    void drawResults();

    void drawParamsWindow();

    void WorkerThred(const std::vector<ImageInfo> &paths, Results *results,
                     const cv::Mat &backgroundImage);

  private:
    bool *bShowWindow = nullptr;
    ImageViewerPannel mImageViewerPannel{false};
    std::vector<std::filesystem::path> mFolders;
    std::vector<int> mActivatedFolders;
    std::vector<std::filesystem::path> mPaths;
    std::vector<std::size_t> mFoldersSizes;
    std::vector<Results> mBatchesResults;
    std::vector<std::string> mBatchesLackingBackground;
    int mCurrentSelectedBatch = 0;
    int mCurrentFrame = 0;
    std::string mPattern{"Img*.tif"};
    std::unique_ptr<Walnut::Image> mPreviewImage;
    std::unique_ptr<Walnut::Image> mProcessedImage;
    ImProc::threshold_detection::ThresholdDetectionParameters mGlobalParams
    {
        .blurKernelSize = 9,
        .threshold = 120,
        .minSize = 15,.ClosingShape = cv::MorphShapes::MORPH_RECT, 
    };
    bool bShouldCalculateSizes = true;
    int mPlayState = 0;
    std::atomic<int> mProgress{};
    float mBatchProgress;
    float mOverallProgress;
    bool bProcessImage{false};
    bool bOutputIntermediateImages{false};

    int mStart;
    int mStop;
    bool bOverrideRange;
};
} // namespace ImProc