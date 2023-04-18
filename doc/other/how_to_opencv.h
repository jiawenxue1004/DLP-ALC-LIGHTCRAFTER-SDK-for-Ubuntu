/**
 * @page how_to_opencv How to setup and compile OpenCV
 * @tableofcontents
 * @section how_to_opencv_introduction Introduction
 *
 * The DLP Structured Light SDK uses OpenCV for a variety of its features such as
 * image data management, filtering, display, chessboard feature extraction, and
 * camera calibration.
 *
 * @section how_to_opencv_dependencies Dependencies
 *
 * To setup OpenCV, the following dependencies are required:
 * - OpenCV v2.4.10 (source)
 *   - It is \b not recommended to use precompiled versions of the OpenCV libraries
 *   - OpenCV 3 is \b not supported.
 * - CMake v2.8.11 or greater
 * - C/C++ Compiler with C++11
 *   - The compiler used for OpenCV must be the same compiler used for the DLP Structured Light SDK and any projects which link to it.
 *   - While OpenCV does not require C++11, the DLP Structured Ligth SDK does and compilers used for both must match.
 * - Linux may require additional dependecies depending on the distribution
 *
 * @section how_to_opencv_installation Installation
 * There are following steps are a basic guideline to setup OpenCV for use with the DLP Structured
 * Light SDK
 * -# Download and install C/C++ compiler with C++11
 *   - GCC 4.8 has been tested on Ubuntu 14.04
 *   - MinGW 4.8 has been tested on Windows 7
 *   - Apple Clang 6.0.0 has been tested on Mac OS X 10.10
 * -# Download and install CMake (http://www.cmake.org/)
 * -# Download and extract OpenCV source files (http://www.opencv.org)
 * -# Use CMake to generate compiler instructions for OpenCV
 *   - Select the OpenCV source directory
 *   - Select the OpenCV build directory
 *     - TI recommends selecting a directory \b outside of the source directory
 *   - Click "Configure" to select compiler and OpenCV options
 *   - Click "Generate" to create compiler instructions
 * -# Compile OpenCV from the build directory
 *   - For example, "make" or "mingw32-make"
 * -# Install OpenCV
 *   - For example, "make install" or "mingw32-make install"
 *   - This step is \b critical for compiling the DLP Structured Light SDK so that the
 *     OpenCV libraries and header files are found
 *
 *
 * @section how_to_opencv_links Helpful Links
 *
 * For detailed installation instructions, the following links may be helpful:
 * - Linux Installation Instructions
 *   - http://docs.opencv.org/doc/tutorials/introduction/linux_install/linux_install.html
 *   - http://www.samontab.com/web/2014/06/installing-opencv-2-4-9-in-ubuntu-14-04-lts/
 *   - http://indranilsinharoy.com/2012/11/01/installing-opencv-on-linux/
 * - Windows Installation Instructions
 *   - http://docs.opencv.org/doc/tutorials/introduction/windows_install/windows_install.html#cpptutwindowsmakeown
 *   - https://www.youtube.com/watch?v=Ny34LpmWijo
 *   - https://www.youtube.com/watch?v=csxkdGQQgNg
 * - Mac OS X Isntallation Instructions
 *   - https://jjyap.wordpress.com/2014/05/24/installing-opencv-2-4-9-on-mac-osx-with-python-support/
 *   - http://www.jeffreythompson.org/blog/2013/08/22/update-installing-opencv-on-mac-mountain-lion/
 *
 * @warning The links above are not made available from TI, representative of TI, nor are they guaranteed by TI.
 * @note The links above may show instructions for different versions of OpenCV, but can be adjusted for the correct version.
 *
 */
