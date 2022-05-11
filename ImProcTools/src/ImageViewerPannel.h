#pragma once
#include "Walnut/Image.h"
#include <memory>


class ImageViewerPannel
{
public:
	ImageViewerPannel();

	void UpdateViewImage(Walnut::Image* image);
	void UpdateProcessedImage(Walnut::Image* image);

	void onUIRender();

private:
	Walnut::Image* mImage;
	Walnut::Image* mProcessedImage;

	float ImageViewerWidth = -1;
};
