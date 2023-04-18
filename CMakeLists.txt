cmake_minimum_required(VERSION 3.0.0)
project( DLP_SDK CXX C)

# new added
include_directories("/usr/include/libusb-1.0")

set(DLP_SDK_VERSION_MAJOR   "2")
set(DLP_SDK_VERSION_MINOR   "0")
set(DLP_SDK_VERSION_PATCH   "0")
set(DLP_SDK_VERSION_EXTRA   "a")
set(DLP_SDK_VERSION         "${DLP_SDK_VERSION_MAJOR}.${DLP_SDK_VERSION_MINOR}")
set(DLP_SDK_VERSION_FULL    "${DLP_SDK_VERSION}.${DLP_SDK_VERSION_PATCH}${DLP_SDK_VERSION_EXTRA}")

# Set the output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)

# Check for Doxygen
option(DLP_GENERATE_DOC "Generate the doxygen html documentation..." ON)
if(DLP_GENERATE_DOC)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        # If doxygen was found create the documentation
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile @ONLY)
        add_custom_target(doc
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
            COMMENT "Generating API documentation with Doxygen" VERBATIM
        )
        
        # Copy the redirect index page
        file(COPY doc/index_redirect.html       DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/doc) 
        file(COPY DLP_SDK_Documentation.html    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})    
    else()
        message(WARNING "Doxygen NOT found. Please install Doxygen and rerun CMake to add the doxygen generation..." )
    endif(DOXYGEN_FOUND)
endif(DLP_GENERATE_DOC)

# Add OpenCV library - Image processing and management
find_package( OpenCV REQUIRED core highgui features2d calib3d flann imgproc)
list(APPEND LIBS_EXTERNAL ${OpenCV_LIBS})

find_path(  DLP_OPENCV_INSTALL_INCLUDE              opencv/cv.hpp       "${OpenCV_DIR}/install/include")        
find_path(  DLP_OPENCV_INSTALL_INCLUDE_OPENCV       cv.hpp              "${OpenCV_DIR}/install/include/opencv")     
find_path(  DLP_OPENCV_INSTALL_INCLUDE_OPENCV2      opencv.hpp          "${OpenCV_DIR}/install/include/opencv2")        

#list(APPEND INCLUDE_DIRS ${OpenCV_INCLUDE_DIRS})
list(APPEND INCLUDE_DIRS ${DLP_OPENCV_INSTALL_INCLUDE})
list(APPEND INCLUDE_DIRS ${DLP_OPENCV_INSTALL_INCLUDE_OPENCV})
list(APPEND INCLUDE_DIRS ${DLP_OPENCV_INSTALL_INCLUDE_OPENCV2})

# Add GLFW library - OpenGL support and direct video output interface
option(DLP_BUILD_GLFW "Include the GLFW library required for OpenGL graphics interface and DLP Platform video streaming..." ON)
if(DLP_BUILD_GLFW)
    # Turn off documentation, examples, and tests
    option(GLFW_BUILD_EXAMPLES  "Build the GLFW example programs"   OFF)
    option(GLFW_BUILD_TESTS     "Build the GLFW test programs"      OFF)
    option(GLFW_BUILD_DOCS      "Build the GLFW documentation"      OFF)
    option(GLFW_INSTALL         "Generate installation target"      OFF)
    
    # Build the GLFW library
    add_subdirectory(3rd_party/glfw-3.0.4)  
    
    # Add GLFW as library to link with DLP SDK
    list(APPEND INCLUDE_DIRS 3rd_party/glfw-3.0.4/include)
    list(APPEND LIBS_INTERNAL glfw)
    list(APPEND LIBS_EXTERNAL ${GLFW_LIBRARIES})
endif(DLP_BUILD_GLFW)

# Add HIDAPI library - Adds support for communication with HID USB devices
option(DLP_BUILD_HIDAPI "Include the HIDAPI library required for many DLP Platforms..." ON)
option(DLP_LINK_HIDAPI_STATIC "Link the HIDAPI library statically into the DLP_SDK library..." ON)
if(DLP_BUILD_HIDAPI)
    if (WIN32)    
        # Add the windows HIDAPI source file
        find_file(HIDAPI_SRC hid.c 3rd_party/hidapi-master/windows) 
        
        if(NOT MSVC)
            # Find the setupapi library needed for the HIDAPI library
            find_library(WIN32_SETUPAPI_LIB setupapi)
            list(APPEND HIDAPI_LIBS setupapi ${WIN32_SETUPAPI_LIB}) 
        endif(NOT MSVC)
    endif (WIN32)

    if (UNIX)
        if(NOT APPLE)
            # Add the linux HIDAPI source file
            # find_file(HIDAPI_SRC hid.c 3rd_party/hidapi-master/linux)
            find_file(HIDAPI_SRC hid.c 3rd_party/hidapi-master/libusb)# must set to libusb on Unbuntu OS!!!!

            message(${HIDAPI_SRC})

            # Find the USB and development libraries needed for USB communication in Linux
            find_library(LINUX_USB_1_0_LIB  usb-1.0)
            find_library(LINUX_UDEV_LIB     udev)       
            list(APPEND HIDAPI_LIBS usb-1.0 ${LINUX_USB_1_0_LIB})
            list(APPEND HIDAPI_LIBS udev    ${LINUX_UDEV_LIB})
        endif(NOT APPLE)
    endif (UNIX)

    if (APPLE)
        # Add the apple HIDAPI source file
        find_file(HIDAPI_SRC hid.c 3rd_party/hidapi-master/mac)
        
        # Add the required Apple frameworks for HIDAPI
        find_library(MAC_COREFOUNDATIAON_LIB    CoreFoundation)
        find_library(MAC_IO_KIT_LIB             IOkit)      
        list(APPEND HIDAPI_LIBS ${MAC_COREFOUNDATIAON_LIB})
        list(APPEND HIDAPI_LIBS ${MAC_IO_KIT_LIB}) 
    endif (APPLE)
        
    # Build the HIDAPI library
    if(DLP_LINK_HIDAPI_STATIC)  
        list(APPEND SRCS ${HIDAPI_SRC})
    else(DLP_LINK_HIDAPI_STATIC)    
        add_library( hidapi ${HIDAPI_SRC})
        target_link_libraries(  hidapi ${HIDAPI_LIBS})
        list(APPEND LIBS_INTERNAL hidapi)
    endif(DLP_LINK_HIDAPI_STATIC)   
        
    # Add HIDAPI as library to link with DLP SDK
    find_path(HIDAPI_INCLUDE_DIR    hidapi.h    "3rd_party/hidapi-master/hidapi")
    list(APPEND INCLUDE_DIRS ${HIDAPI_INCLUDE_DIR}) 
    list(APPEND LIBS_EXTERNAL ${HIDAPI_LIBS})
endif(DLP_BUILD_HIDAPI)

# Add ASIO library - Adds support for communication with sockets
#set to OFF if not use DLP 3000
option(DLP_INCLUDE_ASIO "Include the ASIO library required for the LightCrafter 3000..." OFF)

if(DLP_INCLUDE_ASIO)

    # Find the path to the header file 
    find_path( ASIO_INCLUDE_DIR asio.hpp "3rd_party/asio-1.10.4/include")
            
    if (WIN32)    
        #if(NOT MSVC)
            # Find the ws2_32 library needed for the ASIO library
            list(APPEND DEFINITIONS -D_WIN32_WINNT=0x0501)
            #                                                                                                                                       add_definitions(-D_WIN32_WINNT=0x0501)
            find_library(WIN32_WS2_32_LIB ws2_32)
            list(APPEND ASIO_LIBS ws2_32 ${WIN32_WS2_32_LIB}) 
        #endif(NOT MSVC)
    endif (WIN32)
    
    if (UNIX)
        if(NOT APPLE)
            # Add the linux HIDAPI source file
            find_file(HIDAPI_SRC hid.c 3rd_party/hidapi-master/linux)
            
            # Find the USB and development libraries needed for USB communication in Linux
            find_library(LINUX_USB_1_0_LIB    usb-1.0)
            find_library(LINUX_UDEV_LIB   udev)       
            list(APPEND HIDAPI_LIBS usb-1.0 ${LINUX_USB_1_0_LIB})
            list(APPEND HIDAPI_LIBS udev  ${LINUX_UDEV_LIB})
        endif(NOT APPLE)
    endif (UNIX)

    if (APPLE)
        # Add the apple HIDAPI source file
        find_file(HIDAPI_SRC hid.c 3rd_party/hidapi-master/mac)
        
        # Add the required Apple frameworks for HIDAPI
        find_library(MAC_COREFOUNDATIAON_LIB  CoreFoundation)
        find_library(MAC_IO_KIT_LIB               IOkit)      
        list(APPEND HIDAPI_LIBS ${MAC_COREFOUNDATIAON_LIB})
        list(APPEND HIDAPI_LIBS ${MAC_IO_KIT_LIB}) 
    endif (APPLE)
        
    # Add the include directory and required libraries
    list(APPEND INCLUDE_DIRS    ${ASIO_INCLUDE_DIR})
    list(APPEND LIBS_EXTERNAL   ${ASIO_LIBS})
endif(DLP_INCLUDE_ASIO)

# Option to include the point grey camera module
# if(NOT APPLE)
#     option(DLP_BUILD_PG_FLYCAP2_C_CAMERA_MODULE "Include the TI developed Point Grey Research camera module..." ON)
# else(NOT APPLE)
#     option(DLP_BUILD_PG_FLYCAP2_C_CAMERA_MODULE "Include the TI developed Point Grey Research camera module..." OFF)
# endif(NOT APPLE)
option(DLP_BUILD_PG_FLYCAP2_C_CAMERA_MODULE "Include the TI developed Point Grey Research camera module..." OFF)

if(DLP_BUILD_PG_FLYCAP2_C_CAMERA_MODULE)
    
    if (WIN32)    
    
        find_library(   PG_FLYCAP_LIB           FlyCapture2                 "${PG_FLYCAP_DIR}/lib")
        find_library(   PG_FLYCAP_LIB_C         FlyCapture2_C               "${PG_FLYCAP_DIR}/lib/C")
        get_filename_component(PG_FLYCAP_LIB_DIR "${PG_FLYCAP_LIB}"     DIRECTORY)
        get_filename_component(PG_FLYCAP_DIR     "${PG_FLYCAP_LIB_DIR}" DIRECTORY)

        find_path(      PG_FLYCAP_BIN_DIR       FlyCapture2.dll             "${PG_FLYCAP_DIR}/bin")
        find_path(      PG_FLYCAP_INCLUDE_DIR   C/FlyCapture2_C.h           "${PG_FLYCAP_DIR}/include")
        
    endif (WIN32)

    if (UNIX)
        if(NOT APPLE)
            find_path(      PG_FLYCAP_INCLUDE_DIR   FlyCapture2.h       "/usr/include/flycapture")      
            find_library(   PG_FLYCAP_LIB           flycapture          "/usr/lib/")
            find_library(   PG_FLYCAP_LIB_C         flycapture-c        "/usr/lib/")
                        
    #       list(APPEND LIBS flycapture     "${FLYCAPTURE2_LIB}")
    #       list(APPEND LIBS flycapture-c   "${FLYCAPTURE2_C_LIB}")         
        endif(NOT APPLE)
    endif (UNIX)

    if (APPLE)
        message( FATAL_ERROR "The Point Grey camera module is NOT available for Mac. Exiting..." )
    endif (APPLE)
    
endif(DLP_BUILD_PG_FLYCAP2_C_CAMERA_MODULE)

# Add the required sources for the DLP SDK library
list(APPEND SRCS src/common/other.cpp)
list(APPEND SRCS src/common/returncode.cpp)
list(APPEND SRCS src/common/image/image.cpp)
list(APPEND SRCS src/common/image/image_window.cpp)
list(APPEND SRCS src/common/pattern/pattern.cpp)
list(APPEND SRCS src/common/pattern/pattern_sequence.cpp)
list(APPEND SRCS src/common/capture/capture.cpp)
list(APPEND SRCS src/common/capture/capture_sequence.cpp)
list(APPEND SRCS src/common/disparity_map.cpp)
list(APPEND SRCS src/common/parameters.cpp)
list(APPEND SRCS src/common/module.cpp)
list(APPEND SRCS src/common/debug.cpp)
list(APPEND SRCS src/structured_light/structured_light.cpp)
list(APPEND SRCS src/structured_light/gray_code/gray_code.cpp)
list(APPEND SRCS src/structured_light/three_phase/three_phase.cpp)
list(APPEND SRCS src/geometry/geometry.cpp)
list(APPEND SRCS src/calibration/calibration_data.cpp)
list(APPEND SRCS src/calibration/calibration_camera.cpp)
list(APPEND SRCS src/calibration/calibration_projector.cpp)
list(APPEND SRCS src/camera/camera.cpp)
list(APPEND SRCS src/camera/opencv_cam/opencv_cam.cpp)
if(DLP_BUILD_PG_FLYCAP2_C_CAMERA_MODULE)
    list(APPEND SRCS src/camera/pg_flycap2/pg_flycap2_c.cpp)
endif(DLP_BUILD_PG_FLYCAP2_C_CAMERA_MODULE) 
list(APPEND SRCS src/dlp_platforms/dlp_platform.cpp)
# list(APPEND SRCS src/dlp_platforms/lightcrafter_3000/lcr3000.cpp)
list(APPEND SRCS src/dlp_platforms/lightcrafter_4500/lcr4500.cpp)
list(APPEND SRCS src/dlp_platforms/lightcrafter_4500/dlpc350_api.cpp)
list(APPEND SRCS src/dlp_platforms/lightcrafter_4500/dlpc350_usb.cpp)
list(APPEND SRCS src/dlp_platforms/lightcrafter_4500/dlpc350_firmware.cpp)
list(APPEND SRCS src/dlp_platforms/lightcrafter_4500/common.cpp)
# list(APPEND SRCS src/dlp_platforms/lightcrafter_6500/lcr6500.cpp)
# list(APPEND SRCS src/dlp_platforms/lightcrafter_6500/dlpc900_api.cpp)
# list(APPEND SRCS src/dlp_platforms/lightcrafter_6500/dlpc900_usb.cpp)
# list(APPEND SRCS src/dlp_platforms/lightcrafter_6500/dlpc900_image.cpp)
list(APPEND SRCS src/common/point_cloud/point_cloud.cpp)
list(APPEND SRCS src/common/point_cloud/point_cloud_window.cpp)

# Set the include directories 
list(APPEND INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include)
include_directories(${INCLUDE_DIRS} ${PG_FLYCAP_INCLUDE_DIR})

# Add the definitions
add_definitions(${DEFINITIONS})
            
# Ensure C++11 compatibility
if(NOT MSVC)
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
    CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
    if(COMPILER_SUPPORTS_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    elseif(COMPILER_SUPPORTS_CXX0X)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else()
        message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
    endif()
endif(NOT MSVC)

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
endif (WIN32)

list(APPEND LIBS ${LIBS_INTERNAL} ${LIBS_EXTERNAL})

if(DLP_BUILD_PG_FLYCAP2_C_CAMERA_MODULE)
    list(APPEND LIBS    ${PG_FLYCAP_LIB})
    list(APPEND LIBS    ${PG_FLYCAP_LIB_C}) 
endif(DLP_BUILD_PG_FLYCAP2_C_CAMERA_MODULE)

# Create the library
add_library(DLP_SDK  ${SRCS})
target_link_libraries(DLP_SDK ${LIBS})

# Build the examples
option(DLP_BUILD_EXAMPLES           "Build the DLP Structured Light SDK example programs" ON)
if(DLP_BUILD_EXAMPLES)
    add_executable( returncode_example examples/returncodes_example.cpp)
    target_link_libraries(returncode_example DLP_SDK)       # Link executable to DLP_SDK library
    target_link_libraries(returncode_example ${LIBS})       # Link executable to DLP_SDK dependencies

    add_executable( image_file_io examples/image_file_io.cpp)
    target_link_libraries(image_file_io DLP_SDK)
    target_link_libraries(image_file_io ${LIBS})

    add_executable( image_pixels examples/image_pixels.cpp)
    target_link_libraries(image_pixels DLP_SDK)
    target_link_libraries(image_pixels ${LIBS})

    add_executable( image_window examples/image_window.cpp)
    target_link_libraries(image_window DLP_SDK)
    target_link_libraries(image_window ${LIBS})

    # new added-----------------------------
    add_executable( projector_demo_sequence examples/projector_demo_sequence.cpp)
    target_link_libraries(projector_demo_sequence DLP_SDK)
    target_link_libraries(projector_demo_sequence ${LIBS})

    add_executable( projector_demo_video examples/projector_demo_video.cpp)
    target_link_libraries(projector_demo_video DLP_SDK)
    target_link_libraries(projector_demo_video ${LIBS})
    #---------------------------------------

    add_executable( camera_view_opencv examples/camera_view_opencv.cpp)
    target_link_libraries(camera_view_opencv DLP_SDK)
    target_link_libraries(camera_view_opencv ${LIBS})

    add_executable( generate_structured_light_patterns examples/generate_structured_light_patterns.cpp)
    target_link_libraries(generate_structured_light_patterns DLP_SDK)
    target_link_libraries(generate_structured_light_patterns ${LIBS})

    add_executable( generate_graycode_patterns examples/generate_graycode_patterns.cpp)
    target_link_libraries(generate_graycode_patterns DLP_SDK)
    target_link_libraries(generate_graycode_patterns ${LIBS})
    
    add_executable( point_cloud_viewer examples/point_cloud_viewer.cpp)
    target_link_libraries(point_cloud_viewer DLP_SDK)
    target_link_libraries(point_cloud_viewer ${LIBS})

    if(DLP_BUILD_PG_FLYCAP2_C_CAMERA_MODULE)
        add_executable( camera_view_pg_flycap2_c examples/camera_view_pg_flycap2_c.cpp)
        target_link_libraries(camera_view_pg_flycap2_c DLP_SDK)
        target_link_libraries(camera_view_pg_flycap2_c ${LIBS})

        add_executable( camera_config_pg_flycap2_c examples/camera_config_pg_flycap2_c.cpp)
        target_link_libraries(camera_config_pg_flycap2_c DLP_SDK)
        target_link_libraries(camera_config_pg_flycap2_c ${LIBS})
    endif(DLP_BUILD_PG_FLYCAP2_C_CAMERA_MODULE)

endif(DLP_BUILD_EXAMPLES)

# Copy the resources directory
file(COPY resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Copy the required dll's if on windows
if (WIN32)
    # Copy OpenCV dll's
    file(COPY ${OpenCV_DIR}/bin/libopencv_calib3d${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}.dll      DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    file(COPY ${OpenCV_DIR}/bin/libopencv_core${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}.dll         DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    file(COPY ${OpenCV_DIR}/bin/libopencv_features2d${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}.dll   DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    file(COPY ${OpenCV_DIR}/bin/libopencv_flann${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}.dll        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    file(COPY ${OpenCV_DIR}/bin/libopencv_highgui${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}.dll      DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    file(COPY ${OpenCV_DIR}/bin/libopencv_imgproc${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}.dll      DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    
    # If Point Grey Module enabled copy the dll's
    if(DLP_BUILD_PG_FLYCAP2_C_CAMERA_MODULE)
        file(COPY ${PG_FLYCAP_BIN_DIR}/FlyCapture2.dll      DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
        file(COPY ${PG_FLYCAP_BIN_DIR}/FlyCapture2_C.dll    DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
        file(COPY ${PG_FLYCAP_BIN_DIR}/libiomp5md.dll       DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    endif(DLP_BUILD_PG_FLYCAP2_C_CAMERA_MODULE) 
endif (WIN32)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DLP_SDK-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/DLP_SDK-config.cmake @ONLY)
# configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DLP_SDK-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/DLP_SDK-config.cmake )
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DLP_SDK-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/dlp_sdk-config.cmake )


