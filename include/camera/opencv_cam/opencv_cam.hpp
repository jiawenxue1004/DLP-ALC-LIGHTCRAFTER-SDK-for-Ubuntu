/** @file      opencv_cam.hpp
 *  @brief     Simple wrapper class for OpenCV VideoCapture module with cameras
 *  @copyright 2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#ifndef DLP_SDK_OPENCV_CAM_HPP
#define DLP_SDK_OPENCV_CAM_HPP

// DLP Structured Light SDK header files
#include <common/debug.hpp>                     // Adds dlp::Debug
#include <common/other.hpp>                     // Adds dlp::CmdLine, Time, File, String, Number namespaces
#include <common/returncode.hpp>                // Adds dlp::ReturnCode
#include <common/image/image.hpp>               // Adds dlp::Image
#include <common/parameters.hpp>                // Adds dlp::Parameter
#include <camera/camera.hpp>                    // Adds dlp::Camera
#include <common/capture/capture.hpp>           // Adds dlp::Capture and dlp::Capture::Sequence

// C++ standard header files
#include <queue>                                // Adds std::queue
#include <atomic>                               // Adds std::atomic_bool

#define OPENCV_CAM_NO_CAMERAS_DETECTED         "OPENCV_CAM_NO_CAMERAS_DETECTED"
#define OPENCV_CAM_NO_CONTEXT_CREATED          "OPENCV_CAM_NO_CONTEXT_CREATED"
#define OPENCV_CAM_INVALID_CAMERA_ID           "OPENCV_CAM_INVALID_CAMERA_ID"
#define OPENCV_CAM_NULL_POINTER                "OPENCV_CAM_NULL_POINTER"

#define OPENCV_CAM_BUFFER_RETRIEVE_FAILED           "OPENCV_CAM_BUFFER_RETRIEVE_FAILED"

#define OPENCV_CAM_INVALID_PIXEL_FORMAT             "OPENCV_CAM_INVALID_PIXEL_FORMAT"
#define OPENCV_CAM_INVALID_CURRENT_PIXEL_FORMAT     "OPENCV_CAM_INVALID_CURRENT_PIXEL_FORMAT"

#define OPENCV_CAM_SET_FRAME_WIDTH_FAILED           "OPENCV_CAM_SET_FRAME_WIDTH_FAILED"
#define OPENCV_CAM_SET_FRAME_HEIGHT_FAILED          "OPENCV_CAM_SET_FRAME_HEIGHT_FAILED"
#define OPENCV_CAM_SET_FRAME_RATE_FAILED            "OPENCV_CAM_SET_FRAME_RATE_FAILED"
#define OPENCV_CAM_SET_BRIGHTNESS_FAILED            "OPENCV_CAM_SET_BRIGHTNESS_FAILED"
#define OPENCV_CAM_SET_CONTRAST_FAILED              "OPENCV_CAM_SET_CONTRAST_FAILED"
#define OPENCV_CAM_SET_SATURATION_FAILED            "OPENCV_CAM_SET_SATURATION_FAILED"
#define OPENCV_CAM_SET_HUE_FAILED                   "OPENCV_CAM_SET_HUE_FAILED"
#define OPENCV_CAM_SET_GAIN_FAILED                  "OPENCV_CAM_SET_GAIN_FAILED"
#define OPENCV_CAM_SET_EXPOSURE_FAILED              "OPENCV_CAM_SET_EXPOSURE_FAILED"

#define OPENCV_CAM_IMAGE_BUFFER_EMPTY          "OPENCV_CAM_IMAGE_BUFFER_EMPTY"

namespace dlp {

/** @class OpenCV_Cam
 *  @brief Generic camera class which uses the OpenCV VideoCapture class
 *         to interface with a variety of cameras.
 */
class OpenCV_Cam : public Camera
{
public:


    /** @ingroup   group_Camera
    *  @brief      Contains classes for camera configuration
    *  @{
    */


    struct OpenCVImageBuffer{
        unsigned int            max_count;          /**< Stores the maximum size of the image bugger */
        std::atomic_bool        store_capture;      /**< Boolean flag to determine if recent images should be stored in the buffer  */
        std::atomic_bool        continue_capture;   /**< Boolean flag for capture thread to continue capture or to close the thread */
        std::queue<cv::Mat>     queue;              /**< Image buffer queue */
        std::atomic_flag        lock;               /**< Atomic flag so that different threads do not attempt to access data simultaneously */
    };


    class Parameters{
    public:
        DLP_NEW_PARAMETERS_ENTRY(Height,      "OPENCV_CAM_PARAMETERS_HEIGHT",        double,  0);
        DLP_NEW_PARAMETERS_ENTRY(Width,       "OPENCV_CAM_PARAMETERS_WIDTH",         double,  0);
        DLP_NEW_PARAMETERS_ENTRY(FrameRate,   "OPENCV_CAM_PARAMETERS_FRAME_RATE",    double, 30);
        DLP_NEW_PARAMETERS_ENTRY(Brightness,  "OPENCV_CAM_PARAMETERS_BRIGHTNESS",    double,  0);
        DLP_NEW_PARAMETERS_ENTRY(Contrast,    "OPENCV_CAM_PARAMETERS_CONTRAST",      double,  0);
        DLP_NEW_PARAMETERS_ENTRY(Saturation,  "OPENCV_CAM_PARAMETERS_SATURATION",    double,  0);
        DLP_NEW_PARAMETERS_ENTRY(Hue,         "OPENCV_CAM_PARAMETERS_HUE",           double,  0);
        DLP_NEW_PARAMETERS_ENTRY(Gain,        "OPENCV_CAM_PARAMETERS_GAIN",          double,  0);
        DLP_NEW_PARAMETERS_ENTRY(Exposure,    "OPENCV_CAM_PARAMETERS_EXPOSURE",      double,  0);
    };

    OpenCV_Cam();
    ~OpenCV_Cam();


    // Define pure virtual functions
    ReturnCode Connect(const std::string &id = "0");
    ReturnCode Disconnect();
    ReturnCode Setup(const dlp::Parameters &settings);
    ReturnCode GetSetup(dlp::Parameters *settings)const;
    ReturnCode Start();
    ReturnCode Stop();
    ReturnCode GetFrame(Image* ret_frame);
    ReturnCode GetFrameBuffered(Image* ret_frame);
    ReturnCode GetCaptureSequence(const unsigned int &arg_number_captures,
                                  Capture::Sequence* ret_capture_sequence);

    bool isConnected() const;
    bool isStarted() const;

    ReturnCode GetID(std::string* ret_id) const;
    ReturnCode GetRows(unsigned int* ret_rows) const;
    ReturnCode GetColumns(unsigned int* ret_columns) const;

    ReturnCode GetFrameRate(float* ret_framerate) const;
    ReturnCode GetExposure(float* ret_exposure) const;

private:
    // Members to document whether camera is connected or started
    bool is_connected_;
    bool is_started_;

    unsigned int packet_size;
    float        packet_percent;

    // Parameter settings
    Parameters::Height      height_;
    Parameters::Width       width_;
    Parameters::FrameRate   frame_rate_;
    Parameters::Brightness  brightness_;
    Parameters::Contrast    contrast_;
    Parameters::Saturation  saturation_;
    Parameters::Hue         hue_;
    Parameters::Gain        gain_;
    Parameters::Exposure    exposure_;

    Camera::Parameters::FrameBufferSize image_queue_max_frames_;

    cv::VideoCapture camera_;
    std::string   camera_id_;

    OpenCVImageBuffer image_buffer_;

    std::atomic_bool capture_thread_running_;

    void CaptureThread();
};

}

#endif // DLP_SDK_OPENCV_CAM_HPP
