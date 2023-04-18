/**
 * @page how_to_dlp_sdk How to setup and compile DLP Structured Light SDK
 * @tableofcontents
 * @section how_to_dlp_sdk_introduction Introduction
 *
 * The DLP® Structured Light Software Development Kit (SDK) is a tool designed to enable faster development with
 * DLP technology platforms. The SDK includes pre-built modules that perform specialized tasks using easy
 * to understand methods. The initial goal of the SDK was to create a scaleable platform for 3D machine vision
 * applications, but is not limited to that specific application. The SDK framework is also suitable for 3D printing,
 * spectroscopy, and any other application that requires the use of DLP technology in a simple to use interface.
 *
 * @section how_to_dlp_sdk_dependencies Dependencies
 *
 * To setup the DLP Structured Light SDK, the following dependencies are required:
 * - OpenCV v2.4.6 to v2.4.10 (compiled)
 *   - Please reference \ref how_to_opencv for instructions
 * - CMake v2.8.11 or greater
 * - C/C++ Compiler with C++11
 *   - The compiler used for OpenCV must be the same compiler used for the DLP Structured Light SDK and any projects which link to it.
 * - Linux may require additional dependecies depending on the distribution
 *
 * @section how_to_dlp_sdk_installation Installation
 * There are following steps are a basic guideline to setup the DLP Structured Light SDK
 *
 * -# Download and install C/C++ compiler with C++11
 *   - GCC 4.8 has been tested on Ubuntu 14.04
 *   - MinGW 4.8 has been tested on Windows 7
 *   - Apple Clang 6.0.0 has been tested on Mac OS X 10.10
 * -# Download and install CMake (http://www.cmake.org/)
 * -# Install OpenCV (Please reference \ref how_to_opencv for instructions)
 * -# Use CMake to generate compiler instructions for the DLP Structured Light SDK
 *   - Select the DLP Structured Light SDK source directory
 *   - Select the DLP Structured Light SDK build directory
 *     - TI recommends selecting a directory \b outside of the source directory
 *   - Click "Configure" to select compiler and DLP Structured Light SDK options
 *   - Click "Generate" to create compiler instructions
 * -# Compile DLP Structured Light SDK from the build directory
 *   - For example, "make" or "mingw32-make"
 * -# Generate the documentation using doxygen
 *   - A "Doxyfile" is supplied in the doc folder of the source directory
 *   - There is also a "doc" command that can be used with the compiler.
 *     - For example, "make doc" or "mingw32-make doc"
 */
