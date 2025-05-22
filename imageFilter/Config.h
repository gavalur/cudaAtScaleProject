#pragma once

#include <string>

enum class FilterType
{
    SOBEL_HORIZONTAL,
    MEDIAN,
    GAUSSIAN_SMOOTH,
    BILATERAL,
    UNKNOWN
};

struct ProcessingConfig
{
    std::string inputFile;
    std::string outputFile;
    FilterType filterType = FilterType::SOBEL_HORIZONTAL;
    float sigma = 5.0f;
    int filterRadius = 6;
    float sigmaSpatial = 10.0f;
    float sigmaRange = 20.0f;
    bool verbose = false;
};