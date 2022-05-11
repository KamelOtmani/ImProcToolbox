#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include <iostream>

#include "FileDialogue.h"
#include "Walnut/Image.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

#include "ImageViewerPannel.h"
#include "Postprocess/PostProcessPannel.hpp"
#include "tiffio.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

using namespace Walnut;
namespace fs = std::filesystem;

std::string type2str(int type);

class ExampleLayer : public Walnut::Layer
{
  public:
    virtual void OnAttach() override
    {
        // mPreviewImage = std::make_unique<Image>()
        auto info = cv::getBuildInformation();
    }

    virtual void OnUpdate() override
    {
        static auto StartTime = std::chrono::system_clock::now();
        static auto EndTime = std::chrono::system_clock::now();
        if (bProcessImageSequence)
        {

            if (mCurrentFrame < mImageSequencePaths.size() - 1)
            {
                auto &path = mImageSequencePaths[mCurrentFrame];
                StartTime = std::chrono::system_clock::now();
                if (!fs::is_empty(path))
                {

                    mCVImage = ImportImage(path);
                    mPreviewImage = CreateImageFromOpencv(mCVImage);
                    mProcessedImage = CreateImageFromOpencv(mCVImage);
                    mImageViewerPannel.UpdateViewImage(mPreviewImage.get());
                    mImageViewerPannel.UpdateProcessedImage(
                        mProcessedImage.get());
                }
                if (!mCVImage.empty())
                {
                    Process_Image();
                    EndTime = std::chrono::system_clock::now();
                    FrameTime =
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            EndTime - StartTime)
                            .count();
                }
                if (mCurrentFrame < mImageSequencePaths.size() - 1)
                {
                    mCurrentFrame++;
                }
                else
                {
                    bProcessImageSequence = false;
                }
            }
        }
    }

    virtual void OnUIRender() override
    {
        // ImGui::Begin("##MainWindow");
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("MainTabs", tab_bar_flags))
        {
            if (ImGui::BeginTabItem("Main"))
            {

                ImGui::Begin("Timeline");
                if (ImGui::Button("Import Image", ImVec2{150, 75}))
                {
                    auto paths = Utility::FileDialogue::OpenFile(".TIF;.TIFF");
                    auto &path = paths[0];
                    if (!fs::is_empty(path))
                    {
                        mCVImage = ImportImage(path);
                        mPreviewImage = CreateImageFromOpencv(mCVImage);
                        mProcessedImage = CreateImageFromOpencv(mCVImage);
                        mImageViewerPannel.UpdateViewImage(mPreviewImage.get());
                        mImageViewerPannel.UpdateProcessedImage(
                            mProcessedImage.get());
                    }
                }
                // ImGui::SameLine();
                // if (ImGui::Button("Import Image Sequence"))
                //{
                //	mImageSequencePaths =
                // Utility::FileDialogue::OpenFile(".TIF;.TIFF", true);
                //	//mPreviewImage = ImportImage(path);
                // }
                if (!mImageSequencePaths.empty())
                {
                    if (ImGui::SliderInt("Frame", &mCurrentFrame, 0,
                                         mImageSequencePaths.size() - 1))
                    {
                        auto &path = mImageSequencePaths[mCurrentFrame];
                        if (!fs::is_empty(path))
                        {
                            mCVImage = ImportImage(path);
                            mPreviewImage = CreateImageFromOpencv(mCVImage);
                            mProcessedImage = CreateImageFromOpencv(mCVImage);
                            mImageViewerPannel.UpdateViewImage(
                                mPreviewImage.get());
                            mImageViewerPannel.UpdateProcessedImage(
                                mProcessedImage.get());
                        }
                    }
                }
                ImGui::End();

                mImageViewerPannel.onUIRender();

                ImGui::Begin("Processing");
                static auto m_StartTime = std::chrono::system_clock::now();
                static auto m_EndTime = std::chrono::system_clock::now();

                if (ImGui::Button("Select Background"))
                {
                    auto path =
                        Utility::FileDialogue::OpenFile(".TIF;.TIFF")[0];
                    if (!fs::is_empty(path))
                    {
                        auto cvimage = cv::imread(path.string().c_str(),
                                                  cv::IMREAD_UNCHANGED);
                        cv::cvtColor(cvimage, mBGImage,
                                     cv::ColorConversionCodes::COLOR_GRAY2RGBA);
                    }
                }
                if (ImGui::Button("Process Image"))
                {
                    if (!mCVImage.empty())
                    {

                        m_StartTime = std::chrono::system_clock::now();
                        Process_Image();

                        m_EndTime = std::chrono::system_clock::now();
                    }
                }
                auto dt =
                    (float)
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            (m_EndTime - m_StartTime))
                            .count();
                ImGui::Text("Done in : %.0fms = %.2f FPS", dt, 1000.0 / dt);
                if (ImGui::Button("Process Image Sequence"))
                {
                    bProcessImageSequence = true;
                }
                ImGui::Text("Done in : %.0fms = %.2f FPS", FrameTime,
                            1000 / FrameTime);
                ImGui::End();

                ImGui::Begin("Parameters");
                ImGui::SliderInt("threshold", &mThres, 0, 255);
                ImGui::SliderInt("Small Objects Size", &small_objects_size, 0,
                                 255);
                ImGui::End();

                ImGui::Begin("Debug");
                // ImGui::Combo("step", (int)steps.size(), keys.data());
                if (ImGui::BeginCombo("Steps", ""))
                {

                    int i = 0;
                    for (auto kv : steps)
                    {
                        if (ImGui::Selectable(kv.first.c_str(),
                                              mCureentDisplayedStep == i))
                        {
                            mCureentDisplayedStep = i;
                            Update_Image(steps[kv.first]);
                        }
                        i++;
                    }
                    ImGui::EndCombo();
                }
                ImGui::End();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::ShowDemoWindow();
    }

  private:
    std::unique_ptr<Image> CreateImageFromOpencv(const cv::Mat &image)
    {
        return std::make_unique<Image>(image.cols, image.rows,
                                       ImageFormat::RGBA, image.data);
    }

    cv::Mat ImportImage(path path)
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
                // cv::IMREAD_UNCHANGED); 			cv::cvtColor(cvimage, mCVImage,
                // cv::ColorConversionCodes::COLOR_GRAY2RGBA);
                // auto type = type2str(mCVImage.type());
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

    void Update_Image(cv::Mat &new_image)
    {
        cv::Mat rgb;
        // NOTE : this is a hack as opencv sets the alpha values of the images
        // to 0
        // TODO : profile if maybe iterating over all the values is faster ?
        cv::cvtColor(new_image, rgb, cv::ColorConversionCodes::COLOR_RGBA2RGB);
        cv::cvtColor(rgb, new_image, cv::ColorConversionCodes::COLOR_RGB2RGBA);

        mProcessedImage->SetData(new_image.data);
        // mImageViewerPannel.UpdateViewImage(new_image.data);
    }

    void Process_Image()
    {
        cv::Mat input = mCVImage;
        // keep a copy of the original
        steps["original"] = mCVImage;
        // calculate the homogenious image if we have a bg
        if (!mBGImage.empty())
        {
            // TODO : this operation is saturaing , convert to float !
            // cv::absdiff(mBGImage, mCVImage, mCVImage);
            cv::Mat fimage, fbg;
            input.convertTo(fimage, CV_32F, 1);
            mBGImage.convertTo(fbg, CV_32F, 1);
            // cv::subtract(fimage, fbg, fimage);
            // cv::divide(fimage, fbg, fimage);
            fimage = fimage / fbg;
            // cv::normalize(fimage, fimage, 0, 1, cv::NORM_MINMAX);
            fimage = fimage * 255;
            fimage.convertTo(fimage, CV_8U);
            steps["homo"] = fimage;
            input.convertTo(fimage, CV_32F, 1);
            cv::subtract(fimage, fbg, fimage);
            cv::normalize(fimage, fimage, 0, 1, cv::NORM_MINMAX);
            fimage = fimage * 255;
            fimage.convertTo(input, CV_8U);
            steps["homo_sub"] = fimage;
            // cv::Mat sub_mat = cv::Mat::ones(mCVImage.size(), mCVImage.type())
            // * 255; mCVImage = 255 - mCVImage; cv::invert(mCVImage, mCVImage);
            // mCVImage
        }
        // cv::divide(mCVImage, mBGImage, mCVImage);
        // Filtrage
        cv::Mat blurred;
        int ksize = 9;
        cv::medianBlur(input, blurred, ksize);
        // cv::imshow("preview", blurred);
        // cv::GaussianBlur(mCVImage, blurred, cv::Size(3,3),100);
        // cv::imshow("preview", blurred);
        steps["blurred"] = blurred;
        // Thresholding
        cv::Mat threholded;
        cv::threshold(blurred, threholded, mThres, 255, cv::THRESH_BINARY_INV);
        // cv::adaptiveThreshold()
        steps["threshold"] = threholded;
        // erosion
        cv::Mat erosion, dilation;
        auto erosion_size = cv::Size{small_objects_size, small_objects_size};
        cv::erode(threholded, erosion,
                  cv::getStructuringElement(cv::MORPH_RECT, erosion_size));
        steps["erosion"] = erosion;
        cv::dilate(erosion, dilation,
                   cv::getStructuringElement(cv::MORPH_RECT, erosion_size));
        steps["dilation"] = dilation;
        cv::dilate(dilation, dilation,
                   cv::getStructuringElement(cv::MORPH_RECT, erosion_size));
        steps["dilation2"] = dilation;
        cv::erode(dilation, dilation,
                  cv::getStructuringElement(cv::MORPH_RECT, erosion_size));
        steps["erosion2"] = dilation;
        Update_Image(dilation);
    }

  private:
    ImageViewerPannel mImageViewerPannel;
    // PostprocessTab mPostProcessTab;

    bool bProcessImageSequence = false;
    std::unique_ptr<Image> mPreviewImage;
    std::unique_ptr<Image> mProcessedImage;
    cv::Mat mCVImage;
    cv::Mat mBGImage;
    std::unordered_map<std::string, cv::Mat> steps;
    int mCureentDisplayedStep = 0;
    std::vector<path> mImageSequencePaths;
    int mCurrentFrame = 0;
    // float mLastMouseX, mLastMouseY;
    float uv0[2] = {0, 0};
    float uv1[2] = {1, 1};
    int mThres = 55;
    float FrameTime;
    int small_objects_size = 8;
};

std::string type2str(int type)
{
    std::string r;

    uchar depth = type & CV_MAT_DEPTH_MASK;
    uchar chans = 1 + (type >> CV_CN_SHIFT);

    switch (depth)
    {
    case CV_8U:
        r = "8U";
        break;
    case CV_8S:
        r = "8S";
        break;
    case CV_16U:
        r = "16U";
        break;
    case CV_16S:
        r = "16S";
        break;
    case CV_32S:
        r = "32S";
        break;
    case CV_32F:
        r = "32F";
        break;
    case CV_64F:
        r = "64F";
        break;
    default:
        r = "User";
        break;
    }

    r += "C";
    r += (chans + '0');

    return r;
}

Walnut::Application *Walnut::CreateApplication(int argc, char **argv)
{
    Walnut::ApplicationSpecification spec;
    spec.Name = "Walnut Example";

    Walnut::Application *app = new Walnut::Application(spec);
    app->PushLayer<ExampleLayer>();
    app->SetMenubarCallback(
        [app]()
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Exit"))
                {
                    app->Close();
                }
                ImGui::EndMenu();
            }
            // if (ImGui::BeginMenu("Show"))
            //{
            //	if (ImGui::MenuItem("Demo"))
            //	{

            //	}
            //	ImGui::EndMenu();
            //}
        });
    return app;
}
