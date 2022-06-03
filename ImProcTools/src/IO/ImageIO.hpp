#pragma once

#include <opencv2/core.hpp>
#include <filesystem>

namespace ImProc::Utility
{
	cv::Mat ImportImage(const std::filesystem::path& path);

	std::unique_ptr<Walnut::Image>
        CreateImageFromOpencv(const cv::Mat &image);

	cv::Mat ConvertToRGBA(const cv::Mat &image);
        }
