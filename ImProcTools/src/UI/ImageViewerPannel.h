#pragma once
#include "Walnut/Image.h"
#include <memory>

class ImageViewerPannel
{
  public:
    ImageViewerPannel(bool showOnlyPreview = false);

    void UpdateViewImage(Walnut::Image *image);
    void UpdateProcessedImage(Walnut::Image *image);

    void onUIRender();

  private:
    Walnut::Image *mImage{};
    Walnut::Image *mProcessedImage{};
    bool bShwoOnlyPreviewImage = false;
    float ImageViewerWidth = -1;
};
