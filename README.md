# CUDAAtScaleEnterpriseProject - CUDA NPP image filtering module

## Description

This project was to provide an understanding of different image processing filter capabilities.  Emphasis was given to reusability, extensibility of the module for newer filters to be added in future.  This project make use of the NPP library as well as the Utils provided as part of the course.

Currently the module provides support for two filters:
* Sobel Edge detection filter
* Median filter 


 The project was developed in Coursera Lab environment by reusing the Common library for loading images.  ImageIO.h has been extended to load color images for the current project.  
 


## Code structure 

### ImageFilter.h
Wrapper to invoke the filtering process

### ImageProcessor.h
Core of the functionality.  This file provides an extensible scaffolding for adding new filters with minimal code changes.  This could be extended in future to chain multiple filters.

### ArgsParser.h
Responsible to parse input arguments as well as optional parameters for individual filters

### Config.h
Stores global config values


### Usage  
```
cd imageFilter
make clean all

./imageFilter --input sloth.png --filter sobel --verbose
./imageFilter --input image.png --filter median --radius 8
./imageFilter --help
```
