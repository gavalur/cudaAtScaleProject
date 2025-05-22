#pragma once

#include "ArgsParser.h"
#include <iostream>
#include <helper_string.h>
#include <helper_cuda.h>
#include "ImageProcessor.h"

#include <cuda_runtime.h>
#include <npp.h>
#include <helper_cuda.h>

class ImageFilter
{
private:
    ArgsParser parser_;

public:
    int run(int argc, char *argv[])
    {
        try
        {
            std::cout << argv[0] << " Starting Advanced Image Processor...\n"
                      << std::endl;

            // Check for help flag
            if (checkCmdLineFlag(argc, const_cast<const char **>(argv), "help"))
            {
                parser_.printUsage(argv[0]);
                return EXIT_SUCCESS;
            }

            // Parse command line arguments
            ProcessingConfig config = parser_.parseArguments(argc, argv);

            if (config.verbose)
            {
                std::cout << "Processing " << config.inputFile << " with filter type "
                          << static_cast<int>(config.filterType) << std::endl;
            }

            // Create processor and run
            ImageProcessor processor(config);
            processor.processImage();

            std::cout << "Image processing completed successfully!" << std::endl;
            return EXIT_SUCCESS;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        catch (...)
        {
            std::cerr << "Unknown error occurred" << std::endl;
            return EXIT_FAILURE;
        }
    }
};