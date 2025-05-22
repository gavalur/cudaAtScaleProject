#pragma once 

#include "Config.h"
#include <map>
#include <helper_string.h>
#include <iostream>
#include <stdexcept>

class ArgsParser
{
private:
    std::map<std::string, FilterType> filterMap_;

public:

    ArgsParser()
    {
        filterMap_ = {
            {"sobel", FilterType::SOBEL_HORIZONTAL},
            {"median", FilterType::MEDIAN}};
    }

    ProcessingConfig parseArguments(int argc, char *argv[])
    {
        ProcessingConfig config;

        // Set default input file
        char *inputImagePath = nullptr;
        if (checkCmdLineFlag(argc, const_cast<const char **>(argv), "input"))
        {
            getCmdLineArgumentString(argc, const_cast<const char **>(argv), "input", &inputImagePath);
            config.inputFile = inputImagePath;
        }
        else
        {
            inputImagePath = sdkFindFilePath("sloth.png", argv[0]);
            if (inputImagePath)
            {
                config.inputFile = inputImagePath;
            }
            else
            {
                throw std::runtime_error("No input file specified and default not found");
            }
        }

        // Set output file if specified
        char *outputImagePath = nullptr;
        if (checkCmdLineFlag(argc, const_cast<const char **>(argv), "output"))
        {
            getCmdLineArgumentString(argc, const_cast<const char **>(argv), "output", &outputImagePath);
            config.outputFile = outputImagePath;
        }

        // Set filter type
        char *filterTypeStr = nullptr;
        if (checkCmdLineFlag(argc, const_cast<const char **>(argv), "filter"))
        {
            getCmdLineArgumentString(argc, const_cast<const char **>(argv), "filter", &filterTypeStr);
            std::string filterName = filterTypeStr;

            auto it = filterMap_.find(filterName);
            if (it != filterMap_.end())
            {
                config.filterType = it->second;
            }
            else
            {
                throw std::runtime_error("Unknown filter type: " + filterName);
            }
        }
        else
        {
            config.filterType = FilterType::SOBEL_HORIZONTAL; // Default
        }

        // Parse optional parameters

        if (checkCmdLineFlag(argc, const_cast<const char **>(argv), "radius"))
        {
            config.filterRadius = getCmdLineArgumentInt(argc, const_cast<const char **>(argv), "radius");
        }

        config.verbose = checkCmdLineFlag(argc, const_cast<const char **>(argv), "verbose");

        return config;
    }

    void printUsage(const char *programName) const
    {
        std::cout << "Usage: " << programName << " [options]\n"
                  << "Options:\n"
                  << "  --input <file>     Input image file path\n"
                  << "  --output <file>    Output image file path (optional)\n"
                  << "  --filter <type>    Filter type: sobel, median\n"
                  << "  --radius <value>   Filter radius for median filter (default: 6)\n"
                  << "  --verbose          Enable verbose output\n"
                  << "  --help             Show this help message\n";
    }
};