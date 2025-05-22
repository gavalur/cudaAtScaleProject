/* Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NV_UTIL_NPP_IMAGE_IO_H
#define NV_UTIL_NPP_IMAGE_IO_H

#include "ImagesCPU.h"
#include "ImagesNPP.h"

#include "FreeImage.h"
#include "Exceptions.h"

#include <string>
#include "string.h"


// Error handler for FreeImage library.
//  In case this handler is invoked, it throws an NPP exception.
void
FreeImageErrorHandler(FREE_IMAGE_FORMAT oFif, const char *zMessage)
{
    throw npp::Exception(zMessage);
}

namespace npp
{
    // Load a gray-scale image from disk.
    void
    loadImage(const std::string &rFileName, ImageCPU_8u_C1 &rImage)
    {
        // set your own FreeImage error handler
        FreeImage_SetOutputMessage(FreeImageErrorHandler);

        FREE_IMAGE_FORMAT eFormat = FreeImage_GetFileType(rFileName.c_str());

        // no signature? try to guess the file format from the file extension
        if (eFormat == FIF_UNKNOWN)
        {
            eFormat = FreeImage_GetFIFFromFilename(rFileName.c_str());
        }

        NPP_ASSERT(eFormat != FIF_UNKNOWN);
        // check that the plugin has reading capabilities ...
        FIBITMAP *pBitmap;

        if (FreeImage_FIFSupportsReading(eFormat))
        {
            pBitmap = FreeImage_Load(eFormat, rFileName.c_str());
        }

        NPP_ASSERT(pBitmap != 0);
        // make sure this is an 8-bit single channel image
        //NPP_ASSERT(FreeImage_GetColorType(pBitmap) == FIC_MINISBLACK);
        //NPP_ASSERT(FreeImage_GetBPP(pBitmap) == 8);

        // create an ImageCPU to receive the loaded image data
        ImageCPU_8u_C1 oImage(FreeImage_GetWidth(pBitmap), FreeImage_GetHeight(pBitmap));

        // Copy the FreeImage data into the new ImageCPU
        unsigned int nSrcPitch = FreeImage_GetPitch(pBitmap);
        const Npp8u *pSrcLine = FreeImage_GetBits(pBitmap) + nSrcPitch * (FreeImage_GetHeight(pBitmap) -1);
        Npp8u *pDstLine = oImage.data();
        unsigned int nDstPitch = oImage.pitch();

        for (size_t iLine = 0; iLine < oImage.height(); ++iLine)
        {
            memcpy(pDstLine, pSrcLine, oImage.width() * sizeof(Npp8u));
            pSrcLine -= nSrcPitch;
            pDstLine += nDstPitch;
        }

        // swap the user given image with our result image, effecively
        // moving our newly loaded image data into the user provided shell
        oImage.swap(rImage);
    }

    // Save an gray-scale image to disk.
    void
    saveImage(const std::string &rFileName, const ImageCPU_8u_C1 &rImage)
    {
        // create the result image storage using FreeImage so we can easily
        // save
        FIBITMAP *pResultBitmap = FreeImage_Allocate(rImage.width(), rImage.height(), 8 /* bits per pixel */);
        NPP_ASSERT_NOT_NULL(pResultBitmap);
        unsigned int nDstPitch   = FreeImage_GetPitch(pResultBitmap);
        Npp8u *pDstLine = FreeImage_GetBits(pResultBitmap) + nDstPitch * (rImage.height()-1);
        const Npp8u *pSrcLine = rImage.data();
        unsigned int nSrcPitch = rImage.pitch();

        for (size_t iLine = 0; iLine < rImage.height(); ++iLine)
        {
            memcpy(pDstLine, pSrcLine, rImage.width() * sizeof(Npp8u));
            pSrcLine += nSrcPitch;
            pDstLine -= nDstPitch;
        }

        // now save the result image
        bool bSuccess;
        bSuccess = FreeImage_Save(FIF_PNG, pResultBitmap, rFileName.c_str(), 0) == TRUE;
        NPP_ASSERT_MSG(bSuccess, "Failed to save result image.");
    }

    // Load a gray-scale image from disk.
    void
    loadImage(const std::string &rFileName, ImageNPP_8u_C1 &rImage)
    {
        ImageCPU_8u_C1 oImage;
        loadImage(rFileName, oImage);
        ImageNPP_8u_C1 oResult(oImage);
        rImage.swap(oResult);
    }

    // Save an gray-scale image to disk.
    void
    saveImage(const std::string &rFileName, const ImageNPP_8u_C1 &rImage)
    {
        ImageCPU_8u_C1 oHostImage(rImage.size());
        // copy the device result data
        rImage.copyTo(oHostImage.data(), oHostImage.pitch());
        saveImage(rFileName, oHostImage);
    }

    // Load a gray-scale image from disk.
    void
    loadImage8uC3_old(const std::string &rFileName, ImageCPU_8u_C3 &rImage)
    {
        // set your own FreeImage error handler
        FreeImage_SetOutputMessage(FreeImageErrorHandler);

        FREE_IMAGE_FORMAT eFormat = FreeImage_GetFileType(rFileName.c_str());

        // no signature? try to guess the file format from the file extension
        if (eFormat == FIF_UNKNOWN)
        {
            eFormat = FreeImage_GetFIFFromFilename(rFileName.c_str());
        }

        NPP_ASSERT(eFormat != FIF_UNKNOWN);
        // check that the plugin has reading capabilities ...
        FIBITMAP *pBitmap;

        if (FreeImage_FIFSupportsReading(eFormat))
        {
            pBitmap = FreeImage_Load(eFormat, rFileName.c_str());
        }

        NPP_ASSERT(pBitmap != 0);
        // make sure this is an 8-bit single channel image
        NPP_ASSERT(FreeImage_GetColorType(pBitmap) != FIC_MINISBLACK);
        NPP_ASSERT(FreeImage_GetBPP(pBitmap) == 8);

        // create an ImageCPU to receive the loaded image data
        ImageCPU_8u_C3 oImage(FreeImage_GetWidth(pBitmap), FreeImage_GetHeight(pBitmap));

        // Copy the FreeImage data into the new ImageCPU
        unsigned int nSrcPitch = FreeImage_GetPitch(pBitmap);
        const Npp8u *pSrcLine = FreeImage_GetBits(pBitmap) + nSrcPitch * (FreeImage_GetHeight(pBitmap) - 1);
        Npp8u *pDstLine = oImage.data();
        unsigned int nDstPitch = oImage.pitch();

        for (size_t iLine = 0; iLine < oImage.height(); ++iLine)
        {
            memcpy(pDstLine, pSrcLine, oImage.width() * sizeof(Npp8u));
            pSrcLine -= nSrcPitch;
            pDstLine += nDstPitch;
        }

        // swap the user given image with our result image, effecively
        // moving our newly loaded image data into the user provided shell
        oImage.swap(rImage);
    }

    // Load a 3-channel color image from disk.
    void loadImage8uC3(const std::string &rFileName, ImageCPU_8u_C3 &rImage)
    {
        // Set FreeImage error handler
        FreeImage_SetOutputMessage(FreeImageErrorHandler);

        // Check if file exists
        std::ifstream fileCheck(rFileName.c_str());
        if (!fileCheck.good())
        {
            std::cerr << "Error: File does not exist: " << rFileName << std::endl;
        }
        fileCheck.close();

        // Get file format
        FREE_IMAGE_FORMAT eFormat = FreeImage_GetFileType(rFileName.c_str());

        // No signature? Try to guess the file format from the file extension
        if (eFormat == FIF_UNKNOWN)
        {
            eFormat = FreeImage_GetFIFFromFilename(rFileName.c_str());
            if (eFormat == FIF_UNKNOWN)
            {
                std::cerr << "Error: Unknown file format for " << rFileName << std::endl;
            }
        }

        // Check if the format supports reading
        if (!FreeImage_FIFSupportsReading(eFormat))
        {
            std::cerr << "Error: FreeImage doesn't support reading this format" << std::endl;
        }

        // Load the image
        FIBITMAP *pBitmap = FreeImage_Load(eFormat, rFileName.c_str());
        if (!pBitmap)
        {
            std::cerr << "Error: Failed to load image " << rFileName << std::endl;
        }

        // Convert to 24-bit (3 channel) if it's not already
        FIBITMAP *pTemp = nullptr;
        if (FreeImage_GetBPP(pBitmap) != 24)
        {
            pTemp = FreeImage_ConvertTo24Bits(pBitmap);
            FreeImage_Unload(pBitmap);
            if (!pTemp)
            {
                std::cerr << "Error: Failed to convert image to 24-bit format" << std::endl;
            }
            pBitmap = pTemp;
        }

        // Create an ImageCPU to receive the loaded image data
        unsigned int width = FreeImage_GetWidth(pBitmap);
        unsigned int height = FreeImage_GetHeight(pBitmap);
        ImageCPU_8u_C3 oImage(width, height);

        // Copy the FreeImage data into the new ImageCPU
        unsigned int nSrcPitch = FreeImage_GetPitch(pBitmap);
        const Npp8u *pSrcLine = FreeImage_GetBits(pBitmap) + nSrcPitch * (height - 1);
        Npp8u *pDstLine = oImage.data();
        unsigned int nDstPitch = oImage.pitch();

        // FreeImage stores data in BGR format, assuming ImageCPU_8u_C3 expects RGB
        // We need to copy 3 bytes per pixel
        for (unsigned int y = 0; y < height; ++y)
        {
            // For better performance, use a direct memcpy if no color conversion is needed
            // Otherwise use a pixel-by-pixel copy with color channel reordering
            memcpy(pDstLine, pSrcLine, width * 3 * sizeof(Npp8u));

            // Move to the next line (FreeImage stores bottom-up, so we go backwards)
            pSrcLine -= nSrcPitch;
            pDstLine += nDstPitch;
        }

        // Clean up
        FreeImage_Unload(pBitmap);

        // Swap the user given image with our result image
        oImage.swap(rImage);

    }

    // Save an gray-scale image to disk.
    void
    saveImage8uC3_old(const std::string &rFileName, const ImageCPU_8u_C3 &rImage)
    {
        // create the result image storage using FreeImage so we can easily
        // save
        FIBITMAP *pResultBitmap = FreeImage_Allocate(rImage.width(), rImage.height(), 8 /* bits per pixel */);
        NPP_ASSERT_NOT_NULL(pResultBitmap);
        unsigned int nDstPitch = FreeImage_GetPitch(pResultBitmap);
        Npp8u *pDstLine = FreeImage_GetBits(pResultBitmap) + nDstPitch * (rImage.height() - 1);
        const Npp8u *pSrcLine = rImage.data();
        unsigned int nSrcPitch = rImage.pitch();

        for (size_t iLine = 0; iLine < rImage.height(); ++iLine)
        {
            memcpy(pDstLine, pSrcLine, rImage.width() * sizeof(Npp8u));
            pSrcLine += nSrcPitch;
            pDstLine -= nDstPitch;
        }

        // now save the result image
        bool bSuccess;
        bSuccess = FreeImage_Save(FIF_PNG, pResultBitmap, rFileName.c_str(), 0) == TRUE;
        NPP_ASSERT_MSG(bSuccess, "Failed to save result image.");
    }

    /**
 * Save a 3-channel color image to disk.
 * 
 * @param rFileName - The file path where the image will be saved
 * @param rImage - The 3-channel image data to save
 * @param format - Optional: The image format to save as (default: auto-detect from filename)
 * @return true if the image was saved successfully, false otherwise
 */
    bool saveImage8uC3(const std::string &rFileName, const ImageCPU_8u_C3 &rImage, FREE_IMAGE_FORMAT format = FIF_UNKNOWN)
    {
        // Validate input
        if (rFileName.empty() || rImage.width() == 0 || rImage.height() == 0 || rImage.data() == nullptr)
        {
            std::cerr << "Error: Invalid input parameters for saving image" << std::endl;
            return false;
        }

        // Auto-detect format from filename if not specified
        if (format == FIF_UNKNOWN)
        {
            format = FreeImage_GetFIFFromFilename(rFileName.c_str());
            if (format == FIF_UNKNOWN)
            {
                // Default to PNG if format cannot be determined
                format = FIF_PNG;
                std::cerr << "Warning: Could not determine image format from filename. Using PNG." << std::endl;
            }
        }

        // Check if the format supports writing
        if (!FreeImage_FIFSupportsWriting(format))
        {
            std::cerr << "Error: FreeImage doesn't support writing to this format" << std::endl;
            return false;
        }

        // Create a 24-bit (3 channels * 8 bits) FreeImage bitmap
        FIBITMAP *pResultBitmap = FreeImage_Allocate(rImage.width(), rImage.height(), 24 /* bits per pixel */);
        if (!pResultBitmap)
        {
            std::cerr << "Error: Failed to allocate memory for the output image" << std::endl;
            return false;
        }

        // Set up pointers for copying data
        unsigned int nDstPitch = FreeImage_GetPitch(pResultBitmap);
        Npp8u *pDstLine = FreeImage_GetBits(pResultBitmap) + nDstPitch * (rImage.height() - 1);
        const Npp8u *pSrcLine = rImage.data();
        unsigned int nSrcPitch = rImage.pitch();

        // Copy data from source image to FreeImage bitmap
        // For a 3-channel image, we need to copy 3 bytes per pixel
        for (unsigned int iLine = 0; iLine < rImage.height(); ++iLine)
        {
            // Copy the entire line (3 bytes per pixel)
            memcpy(pDstLine, pSrcLine, rImage.width() * 3 * sizeof(Npp8u));

            // Move to the next line (FreeImage is bottom-up, our image is top-down)
            pSrcLine += nSrcPitch;
            pDstLine -= nDstPitch;
        }

        // Save the image
        int flags = 0;
        if (format == FIF_JPEG)
        {
            // For JPEG, use a quality setting of 90%
            flags = 90;
        }
        else if (format == FIF_PNG)
        {
            // For PNG, use maximum compression
            flags = PNG_Z_BEST_COMPRESSION;
        }

        bool bSuccess = (FreeImage_Save(format, pResultBitmap, rFileName.c_str(), flags) == TRUE);

        // Clean up
        FreeImage_Unload(pResultBitmap);

        if (!bSuccess)
        {
            std::cerr << "Error: Failed to save image to " << rFileName << std::endl;
        }

        return bSuccess;
    }
    
    // Load a gray-scale image from disk.
    void
    loadImage8uC3(const std::string &rFileName, ImageNPP_8u_C3 &rImage)
    {
        ImageCPU_8u_C3 oImage;
        loadImage8uC3(rFileName, oImage);
        ImageNPP_8u_C3 oResult(oImage);
        rImage.swap(oResult);
    }

    // Save an gray-scale image to disk.
    void
    saveImage8uC3(const std::string &rFileName, const ImageNPP_8u_C3 &rImage)
    {
        ImageCPU_8u_C3 oHostImage(rImage.size());
        // copy the device result data
        rImage.copyTo(oHostImage.data(), oHostImage.pitch());
        saveImage8uC3(rFileName, oHostImage);
    }

    void
    loadColorImage(const std::string &rFileName, ImageCPU_8u_C3 &rImage)
    {
        // set your own FreeImage error handler
        FreeImage_SetOutputMessage(FreeImageErrorHandler);

        FREE_IMAGE_FORMAT eFormat = FreeImage_GetFileType(rFileName.c_str());

        // no signature? try to guess the file format from the file extension
        if (eFormat == FIF_UNKNOWN)
        {
            eFormat = FreeImage_GetFIFFromFilename(rFileName.c_str());
        }

        NPP_ASSERT(eFormat != FIF_UNKNOWN);
        // check that the plugin has reading capabilities ...
        FIBITMAP *pBitmap;

        if (FreeImage_FIFSupportsReading(eFormat))
        {
            pBitmap = FreeImage_Load(eFormat, rFileName.c_str());
        }

        NPP_ASSERT(pBitmap != 0);

        // Convert the loaded image to 24-bit RGB if it's not already
        FIBITMAP *pConvertedBitmap = nullptr;
        if (FreeImage_GetBPP(pBitmap) != 24)
        {
            pConvertedBitmap = FreeImage_ConvertTo24Bits(pBitmap);
            FreeImage_Unload(pBitmap);
            pBitmap = pConvertedBitmap;
        }

        // Make sure we have an RGB color image
        NPP_ASSERT(FreeImage_GetColorType(pBitmap) == FIC_RGB);
        NPP_ASSERT(FreeImage_GetBPP(pBitmap) == 24); // 24-bit = 3 channels, 8 bits per channel

        // Create an ImageCPU to receive the loaded image data
        ImageCPU_8u_C3 oImage(FreeImage_GetWidth(pBitmap), FreeImage_GetHeight(pBitmap));

        // Copy the FreeImage data into the new ImageCPU
        unsigned int nSrcPitch = FreeImage_GetPitch(pBitmap);
        const Npp8u *pSrcLine = FreeImage_GetBits(pBitmap) + nSrcPitch * (FreeImage_GetHeight(pBitmap) - 1);
        Npp8u *pDstLine = oImage.data();
        unsigned int nDstPitch = oImage.pitch();

        for (size_t iLine = 0; iLine < oImage.height(); ++iLine)
        {
            memcpy(pDstLine, pSrcLine, oImage.width() * 3 * sizeof(Npp8u)); // 3 bytes per pixel for RGB
            pSrcLine -= nSrcPitch;
            pDstLine += nDstPitch;
        }

        // swap the user given image with our result image, effectively
        // moving our newly loaded image data into the user provided shell
        oImage.swap(rImage);

        // Clean up if we created a converted bitmap
        if (pConvertedBitmap)
        {
            FreeImage_Unload(pBitmap);
        }
    }

    // Load a gray-scale image from disk.
    void
    loadColorImage(const std::string &rFileName, ImageNPP_8u_C3 &rImage)
    {
        ImageCPU_8u_C3 oImage;
        loadColorImage(rFileName, oImage);
        ImageNPP_8u_C3 oResult(oImage);
        rImage.swap(oResult);
    }


    void
    saveImage32(const std::string &rFileName, const ImageCPU_32f_C3 &rImage)
    {
        // First convert 32f to 8u for saving with FreeImage
        // Create a temporary 8u image for conversion
        ImageCPU_32f_C3  convertedImage(rImage.width(), rImage.height());
        Npp32f *pDstData = convertedImage.data();
        const Npp32f *pSrcData = rImage.data();

        // Convert floating point values to 8-bit
        // Assuming the floating point values are in range [0.0, 1.0]
        for (size_t i = 0; i < rImage.height() * rImage.width(); ++i)
        {
            // Clamp to [0.0, 1.0] and scale to [0, 255]
            float pixelValue = pSrcData[i];
            pixelValue = (pixelValue < 0.0f) ? 0.0f : (pixelValue > 1.0f) ? 1.0f : pixelValue;
            pDstData[i] = static_cast<Npp32f>(pixelValue * 255.0f);
        }

        // Create the result image storage using FreeImage
        FIBITMAP *pResultBitmap = FreeImage_Allocate(rImage.width(), rImage.height(), 32 );
        NPP_ASSERT_NOT_NULL(pResultBitmap);
        unsigned int nDstPitch = FreeImage_GetPitch(pResultBitmap);
        Npp8u *pDstLine = FreeImage_GetBits(pResultBitmap) + nDstPitch * (rImage.height() - 1);
        const Npp32f *pSrcLine = convertedImage.data();
        unsigned int nSrcPitch = convertedImage.pitch();

        for (size_t iLine = 0; iLine < rImage.height(); ++iLine)
        {
            memcpy(pDstLine, pSrcLine, rImage.width() * sizeof(Npp8u));
            pSrcLine += nSrcPitch;
            pDstLine -= nDstPitch;
        }

        // Save the result image
        bool bSuccess;
        bSuccess = FreeImage_Save(FIF_PNG, pResultBitmap, rFileName.c_str(), 0) == TRUE;
        NPP_ASSERT_MSG(bSuccess, "Failed to save result image.");

        // Clean up
        FreeImage_Unload(pResultBitmap);
    }

    // Save an color image to disk.
    void
    saveImage32(const std::string &rFileName, const ImageNPP_32f_C3 &rImage)
    {
        ImageCPU_32f_C3 oHostImage(rImage.size());
        // copy the device result data
        rImage.copyTo(oHostImage.data(), oHostImage.pitch());
        saveImage32(rFileName, oHostImage);
    }
    
}


#endif // NV_UTIL_NPP_IMAGE_IO_H
