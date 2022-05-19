#pragma once
#include "ImageViewerPannel.h"
#include <vector>
#include <filesystem>
#include <memory>

namespace ImProc
{

class ConfigurationTab 
{
  public:
    ConfigurationTab(bool* ShowWindow);

    void DrawWindow();
  private:
    bool* bShowWindow = nullptr;
    ImageViewerPannel mImageViewerPannel{true};
    std::vector<std::filesystem::path> mPaths;
    int mCurrentFrame = 0;
    std::unique_ptr<Walnut::Image> mPreviewImage;
};
}