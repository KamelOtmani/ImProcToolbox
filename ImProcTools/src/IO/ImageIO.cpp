
#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <Walnut/Image.h>
#include <memory>
#include "ImageIO.hpp"

namespace ImProc::Utility
{

namespace fs = std::filesystem;
cv::Mat ImportImage(const fs::path& path)
{
    cv::Mat image;
    // if path.extension() == ".tif" | ".tiff"
    if (!fs::is_empty(path))
    {
        // if (false)
        if (path.extension() == ".tif" || path.extension() == ".tiff")
        {
            // TIFF* tif = TIFFOpen(path.string().c_str(), "r");
            // if (tif) {
            //	uint32_t width, height;
            //	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
            //	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
            //	size_t npixels = static_cast<size_t>(width) * height;
            //	// allocate the data for the image
            //	uint32_t* raster = (uint32_t*)_TIFFmalloc(npixels *
            // sizeof(uint32_t)); 	if (raster != nullptr) {
            //		// Read the image as RGBA (libtiff handles the
            // conversions) 		auto result = TIFFReadRGBAImage(tif,
            // width, height, raster, 0); 		if (result)
            //		{
            //			// Create the vulkan image
            //			// Read the file
            //			image = std::make_unique<Image>(width,
            // height, ImageFormat::RGBA, raster); 			auto
            // cvimage = cv::imread(path.string().c_str(),
            // cv::IMREAD_UNCHANGED); 			cv::cvtColor(cvimage,
            // mCVImage, cv::ColorConversionCodes::COLOR_GRAY2RGBA); auto type =
            // type2str(mCVImage.type());
            //		}
            //		_TIFFfree(raster);
            //	}
            //	TIFFClose(tif);
            // }

            // Read the file
            auto cvimage =
                cv::imread(path.string().c_str(), cv::IMREAD_UNCHANGED);
            // Convert to RGBA
            cv::cvtColor(cvimage, image,
                         cv::ColorConversionCodes::COLOR_GRAY2RGBA);
        }
        else
        {
            // Read the file
            image = cv::imread(path.string().c_str(), cv::IMREAD_UNCHANGED);
        }
    }
    return image;
}

std::unique_ptr<Walnut::Image> CreateImageFromOpencv(const cv::Mat &image)
{
    return std::make_unique<Walnut::Image>(
        image.cols, image.rows,
                                   Walnut::ImageFormat::RGBA,
                                   image.data);
}

cv::Mat ConvertToRGBA(const cv::Mat &image)
{
    cv::Mat output;
    cv::cvtColor(image, output, cv::ColorConversionCodes::COLOR_GRAY2RGBA);
    return output;
}
} // namespace ImProc::Utility
