/** @file   camera.cpp
 *  @brief  Contains methods for dlp::Camera class
 *  @copyright 2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

// DLP Structured Light SDK header files
#include <common/debug.hpp>                     // Adds dlp::Debug
#include <common/other.hpp>                     // Adds dlp::CmdLine, Time, File, String, Number namespaces
#include <common/returncode.hpp>                // Adds dlp::ReturnCode
#include <common/image/image.hpp>               // Adds dlp::Image
#include <common/parameters.hpp>                // Adds dlp::Parameter
#include <camera/camera.hpp>                    // Adds dlp::Camera
#include <common/capture/capture.hpp>           // Adds dlp::Capture and dlp::Capture::Sequence
#include <common/module.hpp>                    // Adds dlp::Module

// C++ standard header files
#include <atomic>                               // Adds std::atomic_bool
#include <string>                               // Adds std::string
#include <thread>                               // Adds std::thread
#include <functional>                           // Adds std::ref

namespace dlp{

/** @brief      Opens \ref dlp::Image::Window of the most recent frame in a camera buffer
 *  @param[in] title            String title of the live view window
 *  @param[in] camera           \ref dlp::Camera object to retrieve frames from
 *  @param[in] continue_view    Live view window will close if continuw_view is false
 *  @param[in] delay_ms         Time between displayed frames (does NOT need to match camera frame rate)
 *  @param[in] width            Width in pixels of the live view window
 *  @param[in] height           Height in pixels of the live view window
 *  @bug This function may not operate correctly on Linux or Mac because of multi-threading issues with OpenCV's namedWindow and imshow
 */
void CameraLiveView(const std::string title, dlp::Camera &camera, std::atomic_bool &continue_view, const unsigned int &delay_ms = 16, unsigned int width = 0, unsigned int height = 0){
    dlp::Image::Window* camera_view = new dlp::Image::Window();
    dlp::Image  captured_image;
    dlp::ReturnCode ret;

    // Open the camera view window
    camera_view->Open(title);

    // Wait for the escape key to be pressed or flag
    unsigned int return_key = 0;
    while((return_key != 27) && continue_view){ // 27 is ASCII code for ESC key

        // Clear the previous image
        captured_image.Clear();

        // Grab the most recent frame
        ret = camera.GetFrame(&captured_image);

        // Display the image if no errors occurred
        if(!ret.hasErrors()){
            if((width == 0)||(height ==0))   camera_view->Update(captured_image);
            else    camera_view->Update(captured_image,width,height);
        }

        // Delay 1ms and get any keys that are pressed from window
        camera_view->WaitForKey(delay_ms,&return_key);
    }

    // Close the camera view
    camera_view->Close();

    // Set flag to false so outside world knows the window has closed
    continue_view = false;

    delete camera_view;
}

/** @brief      Opens \ref dlp::Image::Window and displays all images in the camera buffer
 *  @param[in] title            String title of the live view window
 *  @param[in] camera           \ref dlp::Camera object to retrieve frames from
 *  @param[in] continue_view    Live view window will close if continuw_view is false
 *  @param[in] delay_ms         Time between displayed frames (does NOT need to match camera frame rate)
 *  @param[in] width            Width in pixels of the live view window
 *  @param[in] height           Height in pixels of the live view window
 *  @bug This function may not operate correctly on Linux or Mac because of multi-threading issues with OpenCV's namedWindow and imshow
 */
void CameraBufferedView(const std::string title, dlp::Camera &camera, std::atomic_bool &continue_view, const unsigned int &delay_ms = 16, unsigned int width = 0, unsigned int height = 0){
    dlp::Image::Window* camera_view = new dlp::Image::Window();
    dlp::Image  captured_image;
    dlp::ReturnCode ret;

    // Open the camera view window
    camera_view->Open(title);

    // Wait for the escape key to be pressed or flag
    unsigned int return_key = 0;
    while((return_key != 27) && continue_view){ // 27 is ASCII code for ESC key

        // Clear the previous image
        captured_image.Clear();

        // Grab the most recent frame
        ret = camera.GetFrameBuffered(&captured_image);

        // Display the image if no errors occurred
        if(!ret.hasErrors()){
            if((width == 0)||(height ==0))   camera_view->Update(captured_image);
            else    camera_view->Update(captured_image,width,height);
        }

        // Delay 1ms and get any keys that are pressed from window
        camera_view->WaitForKey(delay_ms,&return_key);
    }

    // Close the camera view
    camera_view->Close();

    // Set flag to false so outside world knows the window has closed
    continue_view = false;

    delete camera_view;
}

/** @brief      Opens \ref dlp::Image::Window of the most recent frame in a camera buffer
 *  @param[in] title            String title of the live view window
 *  @param[in] camera           \ref dlp::Camera object to retrieve frames from
 *  @param[in] continue_view    Live view window will close if continuw_view is false
 *  @param[in] delay_ms         Time between displayed frames (does NOT need to match camera frame rate)
 */
void Camera::StartLiveView(dlp::Camera &camera, std::string title, std::atomic_bool &continue_view, const unsigned int &delay_ms){
    unsigned int width = 0;
    unsigned int height = 0;

    camera.GetColumns(&width);
    camera.GetRows(&height);

    std::thread camera_view_thread(CameraLiveView,
                                   title,
                                   std::ref(camera),
                                   std::ref(continue_view),
                                   std::ref(delay_ms),
                                   width,
                                   height);
    camera_view_thread.detach();
}


/** @brief      Opens \ref dlp::Image::Window and displays all images in the camera buffer
 *  @param[in] title            String title of the live view window
 *  @param[in] camera           \ref dlp::Camera object to retrieve frames from
 *  @param[in] continue_view    Live view window will close if continuw_view is false
 *  @param[in] delay_ms         Time between displayed frames (does NOT need to match camera frame rate)
 */
void Camera::StartBufferedView(dlp::Camera &camera, std::string title, std::atomic_bool &continue_view, const unsigned int &delay_ms){
    unsigned int width = 0;
    unsigned int height = 0;

    camera.GetColumns(&width);
    camera.GetRows(&height);

    std::thread camera_view_thread(CameraBufferedView,
                                   title,
                                   std::ref(camera),
                                   std::ref(continue_view),
                                   std::ref(delay_ms),
                                   width,
                                   height);
    camera_view_thread.detach();
}



/** @brief Loads \ref dlp::Parameters file and calls \ref dlp::Camera::Connect and \ref dlp::Camera::Setup for supplied \ref dlp::Camera object
 */
ReturnCode Camera::ConnectSetup(dlp::Camera &camera, std::string id, std::string parameters_file, bool output_cmdline){
    dlp::ReturnCode ret;
    dlp::Parameters settings;

    // Load the parameters from a file
    ret = settings.Load(parameters_file);
    if(ret.hasErrors()){
        if(output_cmdline){
            dlp::CmdLine::Print("Loading parameters file FAILED!");
            dlp::CmdLine::Print(ret.ToString());
        }
        return ret;
    }

    return ConnectSetup(camera, id, settings, output_cmdline);
}

/** @brief Calls \ref dlp::Camera::Connect and \ref dlp::Camera::Setup for supplied \ref dlp::Camera object
 */
ReturnCode Camera::ConnectSetup(dlp::Camera &camera, std::string id, const dlp::Parameters &settings, bool output_cmdline){
    dlp::ReturnCode ret;

    // Connect the projector
    dlp::CmdLine::Print("Connecting to camera...");
    ret = camera.Connect(id);
    if(ret.hasErrors()){
        if(output_cmdline){
            dlp::CmdLine::Print("Connecting to camera FAILED!");
            dlp::CmdLine::Print(ret.ToString());
        }
        return ret;
    }

    // Setup the projector
    dlp::CmdLine::Print("Configuring camera...");
    ret = camera.Setup(settings);
    if(ret.hasErrors()){
        if(output_cmdline){
            dlp::CmdLine::Print("Camera setup FAILED!");
            dlp::CmdLine::Print(ret.ToString());
        }
    }

    return ret;
}

}
