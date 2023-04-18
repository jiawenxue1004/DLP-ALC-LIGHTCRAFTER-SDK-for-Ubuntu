/** @file      pg_flycap2_c.hpp
 *  @brief     Simple wrapper class for Point Grey Research USB Flea cameras
 *  @copyright 2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */


#ifndef DLP_SDK_PG_FLYCAP2_HPP
#define DLP_SDK_PG_FLYCAP2_HPP

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


#define PG_FLYCAP_C_REGISTER_ADDRESS_SOFTWARE_TRIGGER   0x62C

#define PG_FLYCAP_C_NO_CAMERAS_DETECTED         "PG_FLYCAP_C_NO_CAMERAS_DETECTED"
#define PG_FLYCAP_C_NO_CONTEXT_CREATED          "PG_FLYCAP_C_NO_CONTEXT_CREATED"
#define PG_FLYCAP_C_INVALID_CAMERA_ID           "PG_FLYCAP_C_INVALID_CAMERA_ID"
#define PG_FLYCAP_C_NULL_POINTER                "PG_FLYCAP_C_NULL_POINTER"
#define PG_FLYCAP_C_SET_FORMAT_7_FAILED         "PG_FLYCAP_C_SET_FORMAT_7_FAILED"
#define PG_FLYCAP_C_GET_FORMAT_7_FAILED         "PG_FLYCAP_C_GET_FORMAT_7_FAILED"
#define PG_FLYCAP_C_GET_FRAME_RATE_FAILED       "PG_FLYCAP_C_GET_FRAME_RATE_FAILED"
#define PG_FLYCAP_C_GET_SHUTTER_EXPOSURE_FAILED "PG_FLYCAP_C_GET_SHUTTER_EXPOSURE_FAILED"
#define PG_FLYCAP_C_GET_GAMMA_FAILED            "PG_FLYCAP_C_GET_GAMMA_FAILED"
#define PG_FLYCAP_C_GET_WHITE_BALANCE_FAILED    "PG_FLYCAP_C_GET_WHITE_BALANCE_FAILED"
#define PG_FLYCAP_C_GET_HUE_FAILED              "PG_FLYCAP_C_GET_HUE_FAILED"
#define PG_FLYCAP_C_GET_SATURATION_FAILED       "PG_FLYCAP_C_GET_SATURATION_FAILED"
#define PG_FLYCAP_C_GET_BRIGHTNESS_FAILED       "PG_FLYCAP_C_GET_BRIGHTNESS_FAILED"
#define PG_FLYCAP_C_GET_SHARPNESS_FAILED        "PG_FLYCAP_C_GET_SHARPNESS_FAILED"
#define PG_FLYCAP_C_GET_GAIN_FAILED             "PG_FLYCAP_C_GET_GAIN_FAILED"
#define PG_FLYCAP_C_GET_EXPOSURE_FAILED         "PG_FLYCAP_C_GET_EXPOSURE_FAILED"
#define PG_FLYCAP_C_GET_TRIGGER_MODE_FAILED     "PG_FLYCAP_C_GET_TRIGGER_MODE_FAILED"
#define PG_FLYCAP_C_GET_TRIGGER_DELAY_FAILED    "PG_FLYCAP_C_GET_TRIGGER_DELAY_FAILED"
#define PG_FLYCAP_C_GET_STROBE_FAILED           "PG_FLYCAP_C_GET_STROBE_FAILED"


#define PG_FLYCAP_C_BUFFER_RETRIEVE_FAILED      "PG_FLYCAP_C_BUFFER_RETRIEVE_FAILED"

#define PG_FLYCAP_C_INVALID_PIXEL_FORMAT        "PG_FLYCAP_C_INVALID_PIXEL_FORMAT"
#define PG_FLYCAP_C_INVALID_CURRENT_PIXEL_FORMAT        "PG_FLYCAP_C_INVALID_CURRENT_PIXEL_FORMAT"

#define PG_FLYCAP_C_SETUP_FAILED_BRIGHTNESS     "PG_FLYCAP_C_SETUP_FAILED_BRIGHTNESS"
#define PG_FLYCAP_C_SETUP_FAILED_SHARPNESS      "PG_FLYCAP_C_SETUP_FAILED_SHARPNESS"
#define PG_FLYCAP_C_SETUP_FAILED_WHITE_BALANCE  "PG_FLYCAP_C_SETUP_FAILED_WHITE_BALANCE"
#define PG_FLYCAP_C_SETUP_FAILED_HUE            "PG_FLYCAP_C_SETUP_FAILED_HUE"
#define PG_FLYCAP_C_SETUP_FAILED_SATURATION     "PG_FLYCAP_C_SETUP_FAILED_SATURATION"
#define PG_FLYCAP_C_SETUP_FAILED_GAMMA          "PG_FLYCAP_C_SETUP_FAILED_GAMMA"
#define PG_FLYCAP_C_SETUP_FAILED_SHUTTER        "PG_FLYCAP_C_SETUP_FAILED_SHUTTER"
#define PG_FLYCAP_C_SETUP_FAILED_GAIN           "PG_FLYCAP_C_SETUP_FAILED_GAIN"
#define PG_FLYCAP_C_SETUP_FAILED_EXPOSURE       "PG_FLYCAP_C_SETUP_FAILED_EXPOSURE"
#define PG_FLYCAP_C_SETUP_FAILED_TRIGGER_MODE   "PG_FLYCAP_C_SETUP_FAILED_TRIGGER_MODE"
#define PG_FLYCAP_C_SETUP_FAILED_TRIGGER_READY  "PG_FLYCAP_C_SETUP_FAILED_TRIGGER_READY"
#define PG_FLYCAP_C_SETUP_FAILED_TRIGGER_TIMEOUT    "PG_FLYCAP_C_SETUP_FAILED_TRIGGER_TIMEOUT"
#define PG_FLYCAP_C_SETUP_FAILED_TRIGGER_DELAY  "PG_FLYCAP_C_SETUP_FAILED_TRIGGER_DELAY"
#define PG_FLYCAP_C_SETUP_FAILED_FRAME_RATE     "PG_FLYCAP_C_SETUP_FAILED_FRAME_RATE"
#define PG_FLYCAP_C_SETUP_FAILED_GPIO           "PG_FLYCAP_C_SETUP_FAILED_GPIO"
#define PG_FLYCAP_C_SETUP_FAILED_STROBE         "PG_FLYCAP_C_SETUP_FAILED_STROBE"

#define PG_FLYCAP_C_IMAGE_BUFFER_EMPTY          "PG_FLYCAP_C_IMAGE_BUFFER_EMPTY"

namespace dlp {

/** @class PG_FlyCap2_C
 *  @brief Contains classes for Point Grey Research camera configuration and methods to connect,configure
 *          and capture frames using Point Grey Reasearch USB Cameras.
 */
class PG_FlyCap2_C : public Camera
{
public:


    /** @ingroup   group_Camera
    *  @brief      Contains classes for camera configuration
    *  @{
    */


    enum class PixelFormat{
        RAW8,      /*!< 8 bit raw data output of camera sensor         */
        MONO8,     /*!< 8 bit mono(grayscale) output of camera sensor  */
        RGB8,      /*!< R=G=B=8 bits RGB color output of camera sensor */
        INVALID
    };

    class Parameters{
    public:
        DLP_NEW_PARAMETERS_ENTRY(Mode,        "PG_FLYCAP_PARAMETERS_MODE",          unsigned int, 0);
        DLP_NEW_PARAMETERS_ENTRY(Height,      "PG_FLYCAP_PARAMETERS_HEIGHT_PXL",    unsigned int, 0);
        DLP_NEW_PARAMETERS_ENTRY(Width,       "PG_FLYCAP_PARAMETERS_WIDTH_PXL",     unsigned int, 0);
        DLP_NEW_PARAMETERS_ENTRY(OffsetX,     "PG_FLYCAP_PARAMETERS_OFFSET_X_PXL",  unsigned int, 0);
        DLP_NEW_PARAMETERS_ENTRY(OffsetY,     "PG_FLYCAP_PARAMETERS_OFFSET_Y_PXL",  unsigned int, 0);
        DLP_NEW_PARAMETERS_ENTRY(PixelFormat, "PG_FLYCAP_PARAMETERS_PIXEL_FORMAT",  dlp::PG_FlyCap2_C::PixelFormat,dlp::PG_FlyCap2_C::PixelFormat::MONO8);

        DLP_NEW_PARAMETERS_ENTRY(ShutterTime, "PG_FLYCAP_PARAMETERS_SHUTTER_EXPOSURE_MS", float, 16.666);
        DLP_NEW_PARAMETERS_ENTRY(FrameRate,   "PG_FLYCAP_PARAMETERS_FRAME_RATE_HZ",       float, 60.000);

        DLP_NEW_PARAMETERS_ENTRY(Brightness,   "PG_FLYCAP_PARAMETERS_BRIGHTNESS",   float,   0.0);
        DLP_NEW_PARAMETERS_ENTRY(AutoExposure, "PG_FLYCAP_PARAMETERS_AUTOEXPOSURE",  bool, false);
        DLP_NEW_PARAMETERS_ENTRY(Exposure,     "PG_FLYCAP_PARAMETERS_EXPOSURE",  float, 1.0);
        DLP_NEW_PARAMETERS_ENTRY(Sharpness,    "PG_FLYCAP_PARAMETERS_SHARPNESS",    float,  1000);
        DLP_NEW_PARAMETERS_ENTRY(Gain,         "PG_FLYCAP_PARAMETERS_GAIN_DB",      float,   0.0);

        DLP_NEW_PARAMETERS_ENTRY(WhiteBalanceEnable, "PG_FLYCAP_PARAMETERS_WHITE_BALANCE_ENABLE", bool, false);
        DLP_NEW_PARAMETERS_ENTRY(WhiteBalanceBlue,   "PG_FLYCAP_PARAMETERS_WHITE_BALANCE_BLUE",  float,   0.0);
        DLP_NEW_PARAMETERS_ENTRY(WhiteBalanceRed,    "PG_FLYCAP_PARAMETERS_WHITE_BALANCE_RED",   float,   0.0);

        DLP_NEW_PARAMETERS_ENTRY(Hue,         "PG_FLYCAP_PARAMETERS_HUE",         float, 0.0);
        DLP_NEW_PARAMETERS_ENTRY(Saturation,  "PG_FLYCAP_PARAMETERS_SATURATION",  float, 0.0);
        DLP_NEW_PARAMETERS_ENTRY(Gamma,       "PG_FLYCAP_PARAMETERS_GAMMA",       float, 1.0);

        DLP_NEW_PARAMETERS_ENTRY(TriggerEnable,    "PG_FLYCAP_PARAMETERS_TRIGGER_ENABLE",            bool, false);
        DLP_NEW_PARAMETERS_ENTRY(TriggerPolarity,  "PG_FLYCAP_PARAMETERS_TRIGGER_POLARITY",  unsigned int,     0);
        DLP_NEW_PARAMETERS_ENTRY(TriggerSource,    "PG_FLYCAP_PARAMETERS_TRIGGER_SOURCE",    unsigned int,     0);
        DLP_NEW_PARAMETERS_ENTRY(TriggerMode,      "PG_FLYCAP_PARAMETERS_TRIGGER_MODE",      unsigned int,     0);
        DLP_NEW_PARAMETERS_ENTRY(TriggerParameter, "PG_FLYCAP_PARAMETERS_TRIGGER_PARAMETER", unsigned int,     0);
        DLP_NEW_PARAMETERS_ENTRY(TriggerDelay,     "PG_FLYCAP_PARAMETERS_TRIGGER_DELAY",            float,   0.0);

        DLP_NEW_PARAMETERS_ENTRY(StrobeSource,     "PG_FLYCAP_PARAMETERS_STROBE_SOURCE",   unsigned int,     1);
        DLP_NEW_PARAMETERS_ENTRY(StrobeEnable,     "PG_FLYCAP_PARAMETERS_STROBE_ENABLE",           bool, false);
        DLP_NEW_PARAMETERS_ENTRY(StrobePolarity,   "PG_FLYCAP_PARAMETERS_STROBE_POLARITY", unsigned int,     0);
        DLP_NEW_PARAMETERS_ENTRY(StrobeDelay,      "PG_FLYCAP_PARAMETERS_STROBE_DELAY",           float,   0.0);
        DLP_NEW_PARAMETERS_ENTRY(StrobeDuration,   "PG_FLYCAP_PARAMETERS_STROBE_DURATION",        float,   0.0);
    };

    PG_FlyCap2_C();
    ~PG_FlyCap2_C();

    ReturnCode DetectNumOfCameras(unsigned int* num_cameras);



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
    Parameters::Mode                mode_;
    Parameters::Height              height_;
    Parameters::Width               width_;
    Parameters::OffsetX             offset_x_;
    Parameters::OffsetY             offset_y_;
    Parameters::PixelFormat         pixel_format_;

    Parameters::ShutterTime         shutter_;
    Parameters::FrameRate           frame_rate_;

    Parameters::AutoExposure        auto_exposure_;
    Parameters::Exposure            exposure_;

    Parameters::Brightness          brightness_;
    Parameters::Sharpness           sharpness_;
    Parameters::Gain                gain_;

    Parameters::WhiteBalanceEnable  white_balance_enable_;
    Parameters::WhiteBalanceBlue    white_balance_blue_;
    Parameters::WhiteBalanceRed     white_balance_red_;

    Parameters::Hue                 hue_;
    Parameters::Saturation          saturation_;
    Parameters::Gamma               gamma_;

    Parameters::TriggerEnable       trigger_enable_;
    Parameters::TriggerPolarity     trigger_polarity_;
    Parameters::TriggerSource       trigger_source_;
    Parameters::TriggerMode         trigger_mode_;
    Parameters::TriggerParameter    trigger_parameter_;
    Parameters::TriggerDelay        trigger_delay_;

    Parameters::StrobeSource        strobe_source_;
    Parameters::StrobeEnable        strobe_enable_;
    Parameters::StrobePolarity      strobe_polarity_;
    Parameters::StrobeDelay         strobe_delay_;
    Parameters::StrobeDuration      strobe_duration_;

    void* image_buffer_;
    void* camera_context_;
    std::string   camera_id_;

    Camera::Parameters::FrameBufferSize image_queue_max_frames_;
    std::atomic_bool flycap_callback_started_;

};

namespace Number{
template <> std::string ToString<dlp::PG_FlyCap2_C::PixelFormat>( dlp::PG_FlyCap2_C::PixelFormat format );
}

namespace String{
template <> dlp::PG_FlyCap2_C::PixelFormat ToNumber( const std::string &text, unsigned int base );
}

}

#endif // DLP_SDK_PG_FLYCAP2_HPP
