/*! @file   camera.hpp
 *  @brief  Contains definitions for the DLP SDK camera base class
 *  @copyright 2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#ifndef DLP_SDK_CAMERA_HPP
#define DLP_SDK_CAMERA_HPP

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


#define CAMERA_ALREADY_CONNECTED    "CAMERA_ALREADY_CONNECTED"
#define CAMERA_ID_INVALID           "CAMERA_ID_INVALID"
#define CAMERA_NOT_CONNECTED        "CAMERA_NOT_CONNECTED"
#define CAMERA_NOT_SETUP            "CAMERA_NOT_SETUP"
#define CAMERA_NOT_STARTED          "CAMERA_NOT_STARTED"
#define CAMERA_NOT_STOPPED          "CAMERA_NOT_STOPPED"
#define CAMERA_NOT_DISCONNECTED     "CAMERA_NOT_DISCONNECTED"
#define CAMERA_RESOLUTION_INVALID   "CAMERA_RESOLUTION_INVALID"
#define CAMERA_TRIGGER_INVALID      "CAMERA_TRIGGER_INVALID"
#define CAMERA_FRAME_GRAB_FAILED    "CAMERA_FRAME_GRAB_FAILED"
#define CAMERA_FRAME_RATE_INVALID   "CAMERA_FRAME_RATE_INVALID"
#define CAMERA_EXPOSURE_INVALID     "CAMERA_EXPOSURE_INVALID"

#define CAMERA_ALREADY_STARTED      "CAMERA_ALREADY_STARTED"


// Add more #defines for camera errors

/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{
/** @defgroup   group_Camera Camera
 *  @brief      Base class definition for interfacing with cameras
 *
 *  @{
 */

/** @class      Camera
 *  @brief
 */
class Camera: public dlp::Module{
public:

    class Parameters{
    public:
        DLP_NEW_PARAMETERS_ENTRY(FrameBufferSize, "CAMERA_PARAMETERS_FRAME_BUFFER_SIZE",    unsigned int,   60);
        DLP_NEW_PARAMETERS_ENTRY(Shutter_MS,      "CAMERA_PARAMETERS_SHUTTER_EXPOSURE_MS",  float,          16.666);
        DLP_NEW_PARAMETERS_ENTRY(FrameRate_HZ,    "CAMERA_PARAMETERS_FRAME_RATE_HZ",        float,          60.000);
    };


    // Define by subclass
    virtual ReturnCode Connect(const std::string &id = "0") = 0;
    virtual ReturnCode Disconnect() = 0;

    virtual ReturnCode Start() = 0;
    virtual ReturnCode Stop() = 0;

    virtual ReturnCode GetFrame(Image* ret_frame) = 0;
    virtual ReturnCode GetFrameBuffered(Image* ret_frame) = 0;
    virtual ReturnCode GetCaptureSequence(const unsigned int &arg_number_captures,
                                          Capture::Sequence* ret_capture_sequence) = 0;

    virtual bool isConnected() const = 0;
    virtual bool isStarted() const = 0;

    virtual ReturnCode GetID(      std::string* ret_id)      const = 0;
    virtual ReturnCode GetRows(    unsigned int* ret_rows)    const = 0;
    virtual ReturnCode GetColumns( unsigned int* ret_columns) const = 0;

    virtual ReturnCode GetFrameRate( float* ret_framerate) const = 0;
    virtual ReturnCode GetExposure(  float* ret_exposure)  const = 0;

    static ReturnCode ConnectSetup(dlp::Camera &camera, std::string id, std::string parameters_file,     bool output_cmdline = false);
    static ReturnCode ConnectSetup(dlp::Camera &camera, std::string id, const dlp::Parameters &settings, bool output_cmdline = false);


    static void StartLiveView(dlp::Camera &camera, std::string title, std::atomic_bool &continue_view, const unsigned int &delay_ms = 16);
    static void StartBufferedView(  dlp::Camera &camera, std::string title, std::atomic_bool &continue_view, const unsigned int &delay_ms = 16);
};

/** @}*/

}
#endif //DLP_SDK_CAMERA_HPP

