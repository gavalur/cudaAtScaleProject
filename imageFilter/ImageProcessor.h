#pragma once

#include "Config.h"
//#include "NPPDeviceBuffer.h"

#include <string>
#include <functional>
#include <cmath>

#include <ImagesNPP.h>
#include <npp.h>
#include <ImageIO.h>
#include <ImagesCPU.h>

class ImageProcessor
{
private:
    ProcessingConfig config_;

    // Helper methods
    // bool validateInputFile(const std::string &filename) const;
    // std::string generateOutputFilename(const std::string &inputFile,
    //                                    const std::string &suffix) const;
    bool validateInputFile(const std::string &filename) const
    {
        std::ifstream file(filename);
        return file.good();
    }
    std::string generateOutputFilename(const std::string &inputFile,
                                                    const std::string &suffix) const
    {
        if (!config_.outputFile.empty())
        {
            return config_.outputFile;
        }

        std::string result = inputFile;
        std::string::size_type dot = result.rfind('.');

        if (dot != std::string::npos)
        {
            result = result.substr(0, dot);
        }

        result += suffix + ".png";
        return result;
    }

    //void checkNppStatus(NppStatus status) const;
    void checkNppStatus(NppStatus status) const
    {
        if (status != NPP_SUCCESS)
        {
            throw std::runtime_error("NPP operation failed with status: " + std::to_string(status));
        }
    }

    // Common error handling wrapper
    template <typename Func>
    void executeWithErrorHandling(Func &&func, const std::string &operationName) const;

    // Common image processing template method
    template <typename FilterFunc>
    void processImageWithFilter(const std::string &suffix,
                                const std::string &operationName,
                                FilterFunc &&filterOperation);

public:
    ImageProcessor(const ProcessingConfig &config) : config_(config) {}

    // Filter methods

    void applySobelFilter()
    {
        processImageWithFilter("_sobel", "Sobel Filter",
                               [this](const npp::ImageNPP_8u_C3 &deviceSrc,
                                      npp::ImageNPP_8u_C3 &deviceDst,
                                      const NppiSize &filterROI,
                                      const NppiSize &srcSize) {
                                   const NppiPoint srcOffset = {0, 0};

                                   checkNppStatus(nppiFilterSobelHorizBorder_8u_C3R(
                                       deviceSrc.data(), deviceSrc.pitch(), srcSize, srcOffset,
                                       deviceDst.data(), deviceDst.pitch(), filterROI,
                                       NppiBorderType::NPP_BORDER_REPLICATE));
                               });
    }

    void applyMedianFilter()
    {
        processImageWithFilter("_median", "Median Filter",
                               [this](const npp::ImageNPP_8u_C3 &deviceSrc,
                                      npp::ImageNPP_8u_C3 &deviceDst,
                                      const NppiSize &filterROI,
                                      const NppiSize &srcSize) {
                                   const NppiPoint anchor = {0, 0};
                                   const NppiSize maskSize  = {2 * config_.filterRadius + 5,
                                                              2 * config_.filterRadius + 5};

                                   // Allocate device buffer
                                   int bufferSize;
                                   Npp8u *deviceBuffer;
                                   checkNppStatus(nppiMinMaxGetBufferHostSize_8u_C3R(srcSize, &bufferSize));
                                   cudaMalloc((void **)&deviceBuffer, bufferSize);

                                   checkNppStatus(nppiFilterMedian_8u_C3R(
                                       deviceSrc.data(), deviceSrc.pitch(),
                                       deviceDst.data(), deviceDst.pitch(),
                                       filterROI, maskSize, anchor, deviceBuffer));
                               });
    }


    // Main processing method
    void processImage()
    {
        if (!validateInputFile(config_.inputFile))
        {
            throw std::runtime_error("Cannot open input file: " + config_.inputFile);
        }

        switch (config_.filterType)
        {
        case FilterType::SOBEL_HORIZONTAL:
            applySobelFilter();
            break;
        case FilterType::MEDIAN:
            applyMedianFilter();
            break;
        default:
            throw std::runtime_error("Unknown or unsupported filter type");
        }
    }
};

// Template method implementations (must be in header)
template <typename Func>
void ImageProcessor::executeWithErrorHandling(Func &&func, const std::string &operationName) const
{
    try
    {
        func();
        if (config_.verbose)
        {
            std::cout << operationName << " completed successfully." << std::endl;
        }
    }
    catch (const npp::Exception &e)
    {
        std::cerr << "NPP Error in " << operationName << ": " << e << std::endl;
        cudaDeviceReset();
        throw;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in " << operationName << ": " << e.what() << std::endl;
        cudaDeviceReset();
        throw;
    }
    catch (...)
    {
        std::cerr << "Unknown error in " << operationName << std::endl;
        cudaDeviceReset();
        throw;
    }
}

template <typename FilterFunc>
void ImageProcessor::processImageWithFilter(const std::string &suffix,
                                            const std::string &operationName,
                                            FilterFunc &&filterOperation)
{
    const std::string outputFile = generateOutputFilename(config_.inputFile, suffix);

    executeWithErrorHandling([&]() {
        // Load source image
        npp::ImageCPU_8u_C3 hostSrc;
        npp::loadImage8uC3(config_.inputFile, hostSrc);

        // Upload to device
        npp::ImageNPP_8u_C3 deviceSrc(hostSrc);
        npp::ImageNPP_8u_C3 deviceDst(deviceSrc.width(), deviceSrc.height());

        // Set up common filter parameters
        const NppiSize filterROI = {static_cast<int>(deviceSrc.width()),
                                    static_cast<int>(deviceSrc.height())};
        const NppiSize srcSize = filterROI;

        // Apply the specific filter operation
        filterOperation(deviceSrc, deviceDst, filterROI, srcSize);

        // Copy result back to host and save
        npp::ImageCPU_8u_C3 hostDst(deviceDst.size());
        deviceDst.copyTo(hostDst.data(), hostDst.pitch());

        if (config_.verbose)
        {
            std::cout << "Saving " << operationName << " filtered image to: " << outputFile << std::endl;
        }
        saveImage8uC3(outputFile, hostDst);
    }, operationName);
}