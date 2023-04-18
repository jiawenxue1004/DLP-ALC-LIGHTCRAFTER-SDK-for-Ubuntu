/** @file   pg_flycap2_c.cpp
 *  @brief  Contains methods for pg_flycap2 class
 *  @copyright 2014 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

// DLP Structured Light SDK header files
#include <common/debug.hpp>                     // Adds dlp::Debug
#include <common/other.hpp>                     // Adds dlp::CmdLine, Time, File, String, Number namespaces
#include <common/returncode.hpp>                // Adds dlp::ReturnCode
#include <common/image/image.hpp>               // Adds dlp::Image
#include <common/parameters.hpp>                // Adds dlp::Parameter
#include <camera/camera.hpp>                    // Adds dlp::Camera
#include <common/capture/capture.hpp>           // Adds dlp::Capture and dlp::Capture::Sequence
#include <camera/pg_flycap2/pg_flycap2_c.hpp>   // Adds dlp::PG_FlyCap2_C

// C++ standard header files
#include <queue>                                // Adds std::queue
#include <atomic>                               // Adds std::atomic_bool
#include <thread>                               // Adds std::thread

// Point Grey Research FlyCapture 2 Header files
#include <C/FlyCapture2_C.h>
#include <C/FlyCapture2Defs_C.h>

/** @brief Contains all DLP SDK classes, functions, etc. */
namespace dlp{

/** @brief  Structure for image buffer
 */
struct PG_FlyCapImageBuffer{
    unsigned int            max_count;
    bool                    store_capture;
    std::queue<fc2Image>    queue;
    std::atomic_flag        lock;
    std::atomic_flag        stop;
};

/** @brief  Constructor for PG_FlyCap2_C object
 */
PG_FlyCap2_C::PG_FlyCap2_C(){
    this->debug_.SetName("PG_FLYCAP2_DEBUG(" + dlp::Number::ToString(this)+ "): ");
    this->is_connected_ = false;
    this->is_setup_     = false;
    this->is_started_   = false;
    this->flycap_callback_started_ = false;

    // Allocate memory for a new image buffer
    this->image_buffer_   = new PG_FlyCapImageBuffer;

    // Clear the image buffer
	PG_FlyCapImageBuffer *temp = (PG_FlyCapImageBuffer *)this->image_buffer_;
	temp->lock.clear();
	temp->stop.clear();

    this->height_.Set(0);
    this->width_.Set(0);
    this->frame_rate_.Set(0);
    this->shutter_.Set(0);

    this->camera_context_ = new fc2Context;
}

/** @brief  Constructor for PG_FlyCap2_C object
 */
PG_FlyCap2_C::~PG_FlyCap2_C(){
    this->debug_.Msg("Deconstructing...");
    PG_FlyCapImageBuffer* image_buffer = (PG_FlyCapImageBuffer*)this->image_buffer_;

    // Release memory from the image buffer
    this->debug_.Msg("Clearing buffer...");
    while(!image_buffer->queue.empty()){
        // Destroy the image structure contents of the oldest stored frame
        fc2DestroyImage(&image_buffer->queue.front());
        image_buffer->queue.pop();
    }

    this->debug_.Msg("Disconnecting...");
    this->Disconnect();

    this->debug_.Msg("Deallocating memory...");
    delete (PG_FlyCapImageBuffer*)this->image_buffer_;
    delete (fc2Context*)this->camera_context_;
    this->debug_.Msg("Deconstructed");
}

/** @brief      Detect number of Point grey Cameras Connected to the system
 *  @param[out] num_cameras  Pointer to return the number of cameras connected in the system
 *  @retval     PG_FLYCAP_C_NO_CAMERAS_DETECTED    No Point Grey Cameras detected in the system
 */
ReturnCode PG_FlyCap2_C::DetectNumOfCameras(unsigned int *num_cameras)
{
    ReturnCode  ret;
    fc2Context* camera_context = (fc2Context*)this->camera_context_;

    // Create an instance/context for PointGrey Camera and returns a pointer to the context.
    // Needs to be called before any calls that use camera context can be made.
    if(fc2CreateContext(camera_context) != FC2_ERROR_OK)
    {
        this->debug_.Msg("Camera Context Creation Failed");
        return ret.AddError(PG_FLYCAP_C_NO_CONTEXT_CREATED);
    }

    // Returns the number of PointGrey cameras attached to the PC
    if(fc2GetNumOfCameras((*camera_context),num_cameras) != FC2_ERROR_OK)
    {
        this->debug_.Msg("No Point Grey cameras detected");
        return ret.AddError(PG_FLYCAP_C_NO_CAMERAS_DETECTED);
    }

    return ret;
}


/**
 * @brief   Connects a camera object to the camera specified by the Camera Index
 * @param[in] id    Used to find the GUID which is a unique identifier of the camera connected to the system
 * @retval  PG_FLYCAP_C_INVALID_CAMERA_ID    Camera ID sent is invalid
 */
ReturnCode PG_FlyCap2_C::Connect(const std::string &id)
{
    ReturnCode ret;

    // Check that camera isn't already connected
    if(this->isConnected()){
        this->debug_.Msg("Camera already connected!");
        return ret.AddError(CAMERA_ALREADY_CONNECTED);
    }

    fc2CameraInfo   camera_info;
    fc2Context*     camera_context = (fc2Context*)this->camera_context_;
    unsigned int    number_cameras;

    this->DetectNumOfCameras(&number_cameras);

    // If GUID is set to NULL or ommited then the connection is made to the first camera connected to the PC (index = 0)
    fc2PGRGuid guid;


    // Convert string id to integer
    unsigned int id_int = dlp::String::ToNumber<unsigned int>(id);

    // It returns the Point Grey Guid for a camera on the PC which uniquely identifies the camera specified by index and is used
    // to identify the camera during a fc2Connect() API call
    if (fc2GetCameraFromIndex((*camera_context), id_int, &guid) != FC2_ERROR_OK)
    {
        this->debug_.Msg("Camera ID is Invalid");
        return ret.AddError(CAMERA_ID_INVALID);
    }

    // Connect the camera object to the camera specified by GUID
    this->is_connected_ = false;
    if (fc2Connect((*camera_context), &guid) != FC2_ERROR_OK)
    {
        this->debug_.Msg("Camera NOT Connected");
        return ret.AddError(CAMERA_NOT_CONNECTED);
    }

    // Set camera connected flag and save camera id
    this->is_connected_ = true;
    this->camera_id_ = id;


    // Retrieves information from the camera such as serial number, model name and
    // other camera information.
    if ((fc2GetCameraInfo((*camera_context),&camera_info) != FC2_ERROR_OK))
    {
        this->debug_.Msg("Failed to get Camera Information");
    }
    else
    {
        this->debug_.Msg(0, "**** CAMERA INFORMATION ******");
        this->debug_.Msg(0, "Serial Number      = " + dlp::Number::ToString(camera_info.serialNumber));
        this->debug_.Msg(0, "Camera Model       = " + dlp::Number::ToString(camera_info.modelName));
        this->debug_.Msg(0, "Camera Vendor      = " + dlp::Number::ToString(camera_info.vendorName));
        this->debug_.Msg(0, "Camera Sensor      = " + dlp::Number::ToString(camera_info.sensorInfo));
        this->debug_.Msg(0, "Camera Resolution  = " + dlp::Number::ToString(camera_info.sensorResolution));
    }
    return ret;
}


/** @brief   Configures the Camera
 * @param[in] settings    \ref dlp::Parameters object with settings
 * @retval    CAMERA_NOT_CONNECTED            Camera is NOT Connected
 * @retval    CAMERA_NOT_SETUP                Camera is NOT configured/setup
 */
ReturnCode PG_FlyCap2_C::Setup(const dlp::Parameters &settings)
{
    ReturnCode ret;
    fc2Error        camera_error;
    fc2Context*     camera_context = (fc2Context*)this->camera_context_;

    // Check if the camera is connected
    if (!(this->is_connected_))
    {
        this->debug_.Msg("Camera is NOT Connected");
        return ret.AddError(CAMERA_NOT_CONNECTED);
    }

    // Set the maximum buffer size
    settings.Get(&this->image_queue_max_frames_);

    // Set the camera to format 7 video mode
    fc2Format7Info fc2_format7_info;
    BOOL fc2_format7_supported;

    fc2_format7_info.mode = FC2_MODE_0;

    camera_error = fc2GetFormat7Info((*camera_context),&fc2_format7_info,&fc2_format7_supported);
    if((camera_error != FC2_ERROR_OK) || (!fc2_format7_supported))
    {
        this->debug_.Msg("Get fc2SetVideoModeAndFrameRate configuration FAILED");
        std::cout << "Camera Error = " << camera_error << std::endl;
        ret.AddError(PG_FLYCAP_C_GET_FORMAT_7_FAILED);
        ret.AddError(CAMERA_NOT_SETUP);
        return ret;
    }

    // Store the maximum and default modes
    this->mode_.Set( (unsigned int) FC2_MODE_0 );
    this->height_.Set(   fc2_format7_info.maxHeight);
    this->width_.Set(    fc2_format7_info.maxWidth);
    this->offset_x_.Set( 0 );
    this->offset_y_.Set( 0 );
    this->pixel_format_.Set(PixelFormat::MONO8);

    // Check for mode, resolution, offset, and format settings from supplied Parameters
    if(settings.Contains(this->mode_))          settings.Get(&this->mode_);
    if(settings.Contains(this->height_))        settings.Get(&this->height_);
    if(settings.Contains(this->width_))         settings.Get(&this->width_);
    if(settings.Contains(this->offset_x_))      settings.Get(&this->offset_x_);
    if(settings.Contains(this->offset_y_))      settings.Get(&this->offset_y_);
    if(settings.Contains(this->pixel_format_))  settings.Get(&this->pixel_format_);

    // Set the Format 7 Configuration with any new settings from parameters object
    fc2Format7ImageSettings format7_settings_new;
    format7_settings_new.mode    = (fc2Mode) this->mode_.Get();
    format7_settings_new.height  = this->height_.Get();
    format7_settings_new.width   = this->width_.Get();
    format7_settings_new.offsetX = this->offset_x_.Get();
    format7_settings_new.offsetY = this->offset_y_.Get();

    // Get the camera's pixel format
    switch(this->pixel_format_.Get()){
    case PixelFormat::RAW8:
        format7_settings_new.pixelFormat = FC2_PIXEL_FORMAT_RAW8;
        break;
    case PixelFormat::MONO8:
        format7_settings_new.pixelFormat = FC2_PIXEL_FORMAT_MONO8;
        break;
    case PixelFormat::RGB8:
        format7_settings_new.pixelFormat = FC2_PIXEL_FORMAT_RGB8;
        break;
    default:
        this->debug_.Msg("Invalid pixel format, camera NOT setup!");
        ret.AddError(PG_FLYCAP_C_INVALID_PIXEL_FORMAT);
        ret.AddError(CAMERA_NOT_SETUP);
        return ret;
    }

    // Check that Format7 settings are valid
    BOOL format7_settings_valid = false;
    fc2Format7PacketInfo format7_packet_info;
    camera_error = fc2ValidateFormat7Settings((*camera_context),&format7_settings_new,&format7_settings_valid,&format7_packet_info);
    if(camera_error != FC2_ERROR_OK)
    {
        this->debug_.Msg("fc2ValidateFormat7Settings FAILED, camera NOT setup!");
        ret.AddError(PG_FLYCAP_C_SET_FORMAT_7_FAILED);
        ret.AddError(CAMERA_NOT_SETUP);
        return ret;
    }

    if(!format7_settings_valid){
        // Try setting format to MONO8 max resolution

        this->mode_.Set( (unsigned int) FC2_MODE_0 );
        this->height_.Set(   fc2_format7_info.maxHeight);
        this->width_.Set(    fc2_format7_info.maxWidth);
        this->offset_x_.Set( 0 );
        this->offset_y_.Set( 0 );
        this->pixel_format_.Set(PixelFormat::MONO8);

        format7_settings_new.mode    = (fc2Mode) this->mode_.Get();
        format7_settings_new.height  = this->height_.Get();
        format7_settings_new.width   = this->width_.Get();
        format7_settings_new.offsetX = this->offset_x_.Get();
        format7_settings_new.offsetY = this->offset_y_.Get();

        camera_error = fc2ValidateFormat7Settings((*camera_context),
                                                  &format7_settings_new,
                                                  &format7_settings_valid,
                                                  &format7_packet_info);

        if((camera_error != FC2_ERROR_OK) || (!format7_settings_valid))
        {
            this->debug_.Msg("fc2ValidateFormat7Settings mono8 FAILED, camera NOT setup!");
            ret.AddError(PG_FLYCAP_C_SET_FORMAT_7_FAILED);
            ret.AddError(CAMERA_NOT_SETUP);
            return ret;
        }

        this->pixel_format_.Set(PixelFormat::MONO8);
        ret.AddWarning("Supplied settings invalid!");
        ret.AddWarning("Format settings overriden!");
    }


    // Setup the camera's format 7 settings
    // and set the maximum data packet size to 100%
    this->debug_.Msg("Setting camera resolution, offsets, and pixel format...");
    camera_error = fc2SetFormat7Configuration((*camera_context), &format7_settings_new, 100.0);
    if(camera_error != FC2_ERROR_OK)
    {
        this->debug_.Msg("Format7 configuration FAILED, camera NOT setup!");
        ret.AddError(PG_FLYCAP_C_SET_FORMAT_7_FAILED);
        ret.AddError(CAMERA_NOT_SETUP);
        return ret;
    }

    // Check for the frame rate
    fc2Property frame_rate;
    if(settings.Contains(this->frame_rate_)){

        // Retrieve the setting
        settings.Get(&this->frame_rate_);

        // Set the frame rate
        frame_rate.type           = FC2_FRAME_RATE;
        frame_rate.absControl     = true;
        frame_rate.onePush        = false;
        frame_rate.onOff          = true;
        frame_rate.autoManualMode = false;
        frame_rate.absValue       = this->frame_rate_.Get();

        this->debug_.Msg("Setting frame rate...");
        camera_error = fc2SetProperty((*camera_context), &frame_rate);
        if (camera_error != FC2_ERROR_OK)
        {
            this->debug_.Msg("Frame rate configuration FAILED");
            ret.AddError(PG_FLYCAP_C_SETUP_FAILED_FRAME_RATE);
            ret.AddError(CAMERA_NOT_SETUP);
            return ret;
        }
    }

    // Retrieve the actual framerate
    if (fc2GetProperty((*camera_context), &frame_rate) != FC2_ERROR_OK)
    {
        this->debug_.Msg("Could NOT read frame rate value!");
        ret.AddError(PG_FLYCAP_C_GET_FRAME_RATE_FAILED);
        return ret;
    }

    // Save the frame rate
    this->frame_rate_.Set(frame_rate.absValue);


    // Check for shutter time which is the time that the sensor captures light
    fc2Property shutter;
    if(settings.Contains(this->shutter_)){

        // Retrieve the setting
        settings.Get(&this->shutter_);

        // Set the camera shutter time
        shutter.type              = FC2_SHUTTER;
        shutter.absControl        = true;
        shutter.onePush           = false;
        shutter.onOff             = true;
        shutter.autoManualMode    = false;
        shutter.absValue          = this->shutter_.Get();

        camera_error = fc2SetProperty((*camera_context), &shutter);
        if (camera_error != FC2_ERROR_OK)
        {
            this->debug_.Msg("Shutter configuration FAILED");
            ret.AddError(PG_FLYCAP_C_SETUP_FAILED_SHUTTER);
            ret.AddError(CAMERA_NOT_SETUP);
            return ret;
        }
    }

    // Retrieve the shutter exposure time
    camera_error = fc2GetProperty((*camera_context), &shutter);
    if (camera_error != FC2_ERROR_OK)
    {
        this->debug_.Msg("Could NOT read shutter exposure time!");
        ret.AddError(PG_FLYCAP_C_GET_SHUTTER_EXPOSURE_FAILED);
        return ret;
    }

    // Save the shutter time
    this->shutter_.Set(shutter.absValue);



    // If using color camera, set the color related settings
    if(this->pixel_format_.Get() == PixelFormat::RGB8){

        // Check for white balance settings
        fc2Property white_balance;
        if( settings.Contains(this->white_balance_enable_) ||
            settings.Contains(this->white_balance_red_)    ||
            settings.Contains(this->white_balance_blue_)){

            // Retrieve the settings
            settings.Get(&this->white_balance_enable_);
            settings.Get(&this->white_balance_blue_);
            settings.Get(&this->white_balance_red_);

            // Set the camera white balance
            white_balance.type              = FC2_WHITE_BALANCE;
            white_balance.absControl        = false;
            white_balance.onePush           = false;
            white_balance.onOff             = this->white_balance_enable_.Get();
            white_balance.autoManualMode    = false;
            white_balance.valueA            = this->white_balance_red_.Get();
            white_balance.valueB            = this->white_balance_blue_.Get();

            this->debug_.Msg("Setting white balance...");
            camera_error = fc2SetProperty((*camera_context), &white_balance);
            if (camera_error != FC2_ERROR_OK)
            {
                this->debug_.Msg("White Balance configuration FAILED");
                ret.AddError(PG_FLYCAP_C_SETUP_FAILED_WHITE_BALANCE);
                ret.AddError(CAMERA_NOT_SETUP);
                return ret;
            }
        }

        // Retrieving white balance setting
        camera_error = fc2GetProperty((*camera_context), &white_balance);
        if (camera_error != FC2_ERROR_OK)
        {
            this->debug_.Msg("Could NOT read white balance values!");
            ret.AddError(PG_FLYCAP_C_GET_WHITE_BALANCE_FAILED);
            ret.AddError(CAMERA_NOT_SETUP);
            return ret;
        }

        // Save the white balance
        this->white_balance_enable_.Set(white_balance.onOff);
        this->white_balance_red_.Set(white_balance.valueA);
        this->white_balance_blue_.Set(white_balance.valueB);


        // Check for hue settings
        fc2Property hue;
        if( settings.Contains(this->hue_)){

            // Retrieve the value
            settings.Get(&this->hue_);

            // Set the camera hue
            hue.type            = FC2_HUE;
            hue.absControl      = true;
            hue.onePush         = false;
            hue.onOff           = true;
            hue.autoManualMode  = false;
            hue.absValue        = this->hue_.Get();

            this->debug_.Msg("Setting hue...");
            camera_error = fc2SetProperty((*camera_context), &hue);
            if (camera_error != FC2_ERROR_OK){
                this->debug_.Msg("Hue configuration FAILED");
                ret.AddError(PG_FLYCAP_C_SETUP_FAILED_HUE);
                ret.AddError(CAMERA_NOT_SETUP);
                return ret;
            }
        }

        // Check the actual value
        camera_error = fc2GetProperty((*camera_context), &hue);
        if (camera_error != FC2_ERROR_OK){
            this->debug_.Msg("Could NOT read hue value!");
            ret.AddError(PG_FLYCAP_C_GET_HUE_FAILED);
            ret.AddError(CAMERA_NOT_SETUP);
            return ret;
        }

        // Save the value
        this->hue_.Set(hue.absValue);

        // Check for saturation settings
        fc2Property saturation;
        if( settings.Contains(this->saturation_)){

            // Retrieve the value
            settings.Get(&this->saturation_);

            // Set the camera saturation
            saturation.type            = FC2_SATURATION;
            saturation.absControl      = true;
            saturation.onePush         = false;
            saturation.onOff           = true;
            saturation.autoManualMode  = false;
            saturation.absValue        = this->saturation_.Get();

            this->debug_.Msg("Setting saturation...");
            camera_error = fc2SetProperty((*camera_context), &saturation);
            if (camera_error != FC2_ERROR_OK){
                this->debug_.Msg("Saturation configuration FAILED");
                ret.AddError(PG_FLYCAP_C_SETUP_FAILED_SATURATION);
                ret.AddError(CAMERA_NOT_SETUP);
                return ret;
            }
        }

        // Check the actual value
        camera_error = fc2GetProperty((*camera_context), &saturation);
        if (camera_error != FC2_ERROR_OK){
            this->debug_.Msg("Could NOT read saturation value!");
            ret.AddError(PG_FLYCAP_C_GET_SATURATION_FAILED);
            ret.AddError(CAMERA_NOT_SETUP);
            return ret;
        }

        // Save the value
        this->saturation_.Set(saturation.absValue);

        // Check for gamma settings
        fc2Property gamma;
        if( settings.Contains(this->gamma_)){

            // Retrieve the value
            settings.Get(&this->gamma_);

            // Set the camera gamma
            gamma.type            = FC2_GAMMA;
            gamma.absControl      = true;
            gamma.onePush         = false;
            gamma.onOff           = true;
            gamma.autoManualMode  = false;
            gamma.absValue        = this->gamma_.Get();

            this->debug_.Msg("Setting gamma...");
            camera_error = fc2SetProperty((*camera_context), &gamma);
            if (camera_error != FC2_ERROR_OK){
                this->debug_.Msg("Gamma configuration FAILED");
                ret.AddError(PG_FLYCAP_C_SETUP_FAILED_GAMMA);
                ret.AddError(CAMERA_NOT_SETUP);
                return ret;
            }
        }

        // Check the actual setting
        camera_error = fc2GetProperty((*camera_context), &gamma);
        if (camera_error != FC2_ERROR_OK){
            this->debug_.Msg("Could NOT read gamma value!");
            ret.AddError(PG_FLYCAP_C_GET_GAMMA_FAILED);
            ret.AddError(CAMERA_NOT_SETUP);
            return ret;
        }

        // Save the setting
        this->gamma_.Set(gamma.absValue);
    }


    // Check for brightness settings
    fc2Property brightness;
    brightness.type = FC2_BRIGHTNESS;
    if(settings.Contains(this->brightness_)){

        // Retrieve the setting
        settings.Get(&this->brightness_);

        // Set the camera brightness
        brightness.absControl       = false;
        brightness.onePush          = false;
        brightness.onOff            = true;
        brightness.autoManualMode   = false;
        brightness.valueA           = this->brightness_.Get();

        camera_error = fc2SetProperty((*camera_context), &brightness);
        if(camera_error != FC2_ERROR_OK){
            this->debug_.Msg("Brightness configuration FAILED");
            ret.AddError(PG_FLYCAP_C_SETUP_FAILED_BRIGHTNESS);
            ret.AddError(CAMERA_NOT_SETUP);
            return ret;
        }
    }

    // Check the actual setting
    camera_error = fc2GetProperty((*camera_context), &brightness);
    if (camera_error != FC2_ERROR_OK){
        this->debug_.Msg("Could NOT read brightness value!");
        ret.AddError(PG_FLYCAP_C_GET_BRIGHTNESS_FAILED);
        ret.AddError(CAMERA_NOT_SETUP);
        return ret;
    }

    // Save the actual value
    this->brightness_.Set(brightness.valueA);

    // Check for sharpness setting
    fc2Property sharpness;
    sharpness.type = FC2_SHARPNESS;
    if(settings.Contains(this->sharpness_)){

        // Retrieve setting
        settings.Get(&this->sharpness_);

        // Set the camera sharpness
        sharpness.absControl        = false;
        sharpness.onePush           = false;
        sharpness.onOff             = true;
        sharpness.autoManualMode    = false;
        sharpness.valueA            = this->sharpness_.Get();

        camera_error = fc2SetProperty((*camera_context), &sharpness);
        if (camera_error != FC2_ERROR_OK){
            this->debug_.Msg("Sharpness configuration FAILED");
            ret.AddError(PG_FLYCAP_C_SETUP_FAILED_SHARPNESS);
            ret.AddError(CAMERA_NOT_SETUP);
            return ret;
        }
    }

    // Check actual setting
    camera_error = fc2GetProperty((*camera_context), &sharpness);
    if(camera_error != FC2_ERROR_OK)
    {
        this->debug_.Msg("Could NOT read sharpness!");
        ret.AddError(PG_FLYCAP_C_GET_SHARPNESS_FAILED);
        ret.AddError(CAMERA_NOT_SETUP);
        return ret;
    }

    // Save actual value
    this->sharpness_.Set(sharpness.valueA);

    // Check for gain setting
    fc2Property gain;
    gain.type = FC2_GAIN;
    if(settings.Contains(this->gain_)){

        // Retrieve settings
        settings.Get(&this->gain_);

        // Set the camera gain
        gain.absControl     = true;
        gain.onePush        = false;
        gain.onOff          = true;
        gain.autoManualMode = false;
        gain.absValue       = this->gain_.Get();

        camera_error = fc2SetProperty((*camera_context), &gain);
        if (camera_error != FC2_ERROR_OK)
        {
            this->debug_.Msg("Gain configuration FAILED");
            ret.AddError(PG_FLYCAP_C_SETUP_FAILED_GAIN);
            ret.AddError(CAMERA_NOT_SETUP);
            return ret;
        }
    }

    // Check actual setting
    camera_error = fc2GetProperty((*camera_context), &gain);
    if(camera_error != FC2_ERROR_OK)
    {
        this->debug_.Msg("Could NOT read gain!");
        ret.AddError(PG_FLYCAP_C_GET_GAIN_FAILED);
        ret.AddError(CAMERA_NOT_SETUP);
        return ret;
    }

    // Save actual value
    this->gain_.Set(gain.absValue);



    // Check for exposure setting
    fc2Property exposure;
    exposure.type = FC2_AUTO_EXPOSURE;
    if(settings.Contains(this->auto_exposure_) || settings.Contains(this->exposure_)){

        // Retrieve settings
        settings.Get(&this->auto_exposure_);
        settings.Get(&this->exposure_);

        // Set the camera exposure
        exposure.absControl       = true;
        exposure.onePush          = false;
        exposure.onOff            = this->auto_exposure_.Get();
        exposure.autoManualMode   = false;
        exposure.absValue         = this->exposure_.Get();

        camera_error = fc2SetProperty((*camera_context), &exposure);
        if (camera_error != FC2_ERROR_OK)
        {
            this->debug_.Msg("Gain configuration FAILED");
            ret.AddError(PG_FLYCAP_C_SETUP_FAILED_EXPOSURE);
            ret.AddError(CAMERA_NOT_SETUP);
            return ret;
        }
    }

    // Check actual setting
    camera_error = fc2GetProperty((*camera_context), &exposure);
    if(camera_error != FC2_ERROR_OK)
    {
        this->debug_.Msg("Could NOT read exposure!");
        ret.AddError(PG_FLYCAP_C_GET_EXPOSURE_FAILED);
        ret.AddError(CAMERA_NOT_SETUP);
        return ret;
    }

    // Save actual value
    this->exposure_.Set(exposure.absValue);
    this->auto_exposure_.Set(exposure.onOff);


    // First read the current trigger mode settings in case some
    // settings are missing from the parameters list
    fc2TriggerMode trigger_settings;
    camera_error = fc2GetTriggerMode((*camera_context),&trigger_settings);
    if (camera_error != FC2_ERROR_OK){
        this->debug_.Msg("Could NOT read trigger mode settings!");
        ret.AddError(PG_FLYCAP_C_GET_TRIGGER_MODE_FAILED);
        ret.AddError(CAMERA_NOT_SETUP);
        return ret;
    }

    // Save current values
    this->trigger_polarity_.Set(trigger_settings.polarity);
    this->trigger_source_.Set(trigger_settings.source);
    this->trigger_parameter_.Set(trigger_settings.parameter);
    this->trigger_enable_.Set(trigger_settings.onOff);
    this->trigger_mode_.Set(trigger_settings.mode);

    // Read any trigger settings from parameter list
    if(settings.Contains(this->trigger_enable_))    settings.Get(&this->trigger_enable_);
    if(settings.Contains(this->trigger_polarity_))  settings.Get(&this->trigger_polarity_);
    if(settings.Contains(this->trigger_source_))    settings.Get(&this->trigger_source_);
    if(settings.Contains(this->trigger_mode_))      settings.Get(&this->trigger_mode_);
    if(settings.Contains(this->trigger_parameter_)) settings.Get(&this->trigger_parameter_);
    if(settings.Contains(this->trigger_delay_))     settings.Get(&this->trigger_delay_);

    // Initialize all parameters of Trigger Settings
    trigger_settings.polarity   = this->trigger_polarity_.Get();
    trigger_settings.source     = this->trigger_source_.Get();
    trigger_settings.parameter  = this->trigger_parameter_.Get();
    trigger_settings.onOff      = this->trigger_enable_.Get();
    trigger_settings.mode       = this->trigger_mode_.Get();

    // Send the trigger mode settings
    camera_error = fc2SetTriggerMode((*camera_context),&trigger_settings);
    if (camera_error != FC2_ERROR_OK){
        this->debug_.Msg("Trigger mode configuration FAILED");
        ret.AddError(PG_FLYCAP_C_SETUP_FAILED_TRIGGER_MODE);
        ret.AddError(CAMERA_NOT_SETUP);
        return ret;
    }


    // If the trigger was turned and hardware trigger is selected,
    // poll the camera until it is ready to accept triggers
    if (trigger_settings.onOff && (trigger_settings.source == 0)){

        // Poll the camera until the device is ready to accept triggers
        // or the timeoue has occurred
        unsigned int register_value = 0;

        this->debug_.Msg("Hardware trigger mode enabled");
        this->debug_.Msg("Waiting for hardware triggers...");

        // Setup timeout
        dlp::Time::Chronograph time_out;
        time_out.Reset();

        do{
            camera_error = fc2ReadRegister((*camera_context),
                                           PG_FLYCAP_C_REGISTER_ADDRESS_SOFTWARE_TRIGGER,
                                           &register_value);
            if (camera_error != FC2_ERROR_OK){
                this->debug_.Msg(1, "Trigger ready polling FAILED");
                ret.AddError(PG_FLYCAP_C_SETUP_FAILED_TRIGGER_READY);
                ret.AddError(CAMERA_NOT_SETUP);
                return ret;
            }

            // This bit gets cleared as soon as Trigger is enabled and
            // it is finishes processing exisiting frame
        } while (((register_value >> 31) != 0) &&
                 (time_out.GetTotalTime() < 5000) );    // If time exceeds 5 seconds exit function

        // Check that a timeout did not occur
        if (time_out.GetTotalTime() >= 5000){
            this->debug_.Msg(1, "Polling for trigger ready TIME OUT");
            ret.AddError(PG_FLYCAP_C_SETUP_FAILED_TRIGGER_TIMEOUT);
            ret.AddError(CAMERA_NOT_SETUP);
            return ret;
        }
    }


    // Check if settings contained trigger delay
    fc2TriggerDelay trigger_delay;
    trigger_delay.type = FC2_TRIGGER_DELAY;

    if(settings.Contains(this->trigger_delay_)){

        // Set the trigger delay
        trigger_delay.absControl     = true;
        trigger_delay.onePush        = false;
        trigger_delay.onOff          = true;
        trigger_delay.autoManualMode = false;
        trigger_delay.absValue       = this->trigger_delay_.Get();

        camera_error = fc2SetTriggerDelay((*camera_context), &trigger_delay);
        if (camera_error != FC2_ERROR_OK){
            this->debug_.Msg("Trigger delay configuration FAILED");
            ret.AddError(PG_FLYCAP_C_SETUP_FAILED_TRIGGER_DELAY);
            ret.AddError(CAMERA_NOT_SETUP);
            return ret;
        }
    }

    // Read the trigger delay settings
    camera_error = fc2GetTriggerDelay((*camera_context), &trigger_delay);
    if (camera_error != FC2_ERROR_OK){
        this->debug_.Msg("Could NOT read trigger delay!");
        ret.AddError(PG_FLYCAP_C_GET_TRIGGER_DELAY_FAILED);
        ret.AddError(CAMERA_NOT_SETUP);
        return ret;
    }

    // Check if settings contains a specified strobe source
    if(settings.Contains(this->strobe_source_)){

        // Retrieve the strobe source
        settings.Get(&this->strobe_source_);

        // Read the strobe source current settings
        fc2StrobeControl strobe_settings;
        strobe_settings.source      = this->strobe_source_.Get();
        camera_error = fc2GetStrobe((*camera_context), &strobe_settings);
        if (camera_error != FC2_ERROR_OK){
            this->debug_.Msg("Coulnd NOT read strobe configuration!");
            ret.AddError(PG_FLYCAP_C_GET_STROBE_FAILED);
            ret.AddError(CAMERA_NOT_SETUP);
            return ret;
        }

        // Check for other strobe settings in parameter object
        if(settings.Contains(this->strobe_enable_))     settings.Get(&this->strobe_enable_);
        if(settings.Contains(this->strobe_polarity_))   settings.Get(&this->strobe_polarity_);
        if(settings.Contains(this->strobe_delay_))      settings.Get(&this->strobe_delay_);
        if(settings.Contains(this->strobe_duration_))   settings.Get(&this->strobe_duration_);

        // Update the strobe settings
        strobe_settings.onOff       = this->strobe_enable_.Get();
        strobe_settings.polarity    = this->strobe_polarity_.Get();
        strobe_settings.delay       = this->strobe_delay_.Get();
        strobe_settings.duration    = this->strobe_duration_.Get();

        // Setup the strobe
        camera_error = fc2SetStrobe((*camera_context), &strobe_settings);
        if (camera_error != FC2_ERROR_OK){
            this->debug_.Msg("Strobe configuration FAILED");
            ret.AddError(PG_FLYCAP_C_SETUP_FAILED_STROBE);
            ret.AddError(CAMERA_NOT_SETUP);
            return ret;
        }
    }

    // Mark setup flag as true
    this->is_setup_ = true;

    return ret;
}

/**
 * @brief Returns the camera configuration settings
 * @param[out] settings         Pointer to \ref dlp::Parameters object to store settings in
 * @retval  CAMERA_NOT_SETUP    Camera has NOT been set up/configured
 */
ReturnCode PG_FlyCap2_C::GetSetup(dlp::Parameters *settings)const{
    ReturnCode ret;

    // Check that pointer is NOT null
    if(!settings)
        return ret.AddError(PG_FLYCAP_C_NULL_POINTER);

    if(!this->isSetup())
        return ret.AddError(CAMERA_NOT_SETUP);

    // Clear the parameters list
    settings->Clear();

    // Save the settings

    settings->Set(this->mode_);
    settings->Set(this->height_);
    settings->Set(this->width_);
    settings->Set(this->offset_x_);
    settings->Set(this->offset_y_);
    settings->Set(this->pixel_format_);
    settings->Set(this->shutter_);
    settings->Set(this->frame_rate_);
    settings->Set(this->brightness_);
    settings->Set(this->sharpness_);
    settings->Set(this->gain_);

    if(this->pixel_format_.Get() == PixelFormat::RGB8){
        settings->Set(this->white_balance_enable_);
        settings->Set(this->white_balance_blue_);
        settings->Set(this->white_balance_red_);
        settings->Set(this->hue_);
        settings->Set(this->saturation_);
        settings->Set(this->gamma_);
    }

    settings->Set(this->trigger_enable_);
    settings->Set(this->trigger_polarity_);
    settings->Set(this->trigger_source_);
    settings->Set(this->trigger_mode_);
    settings->Set(this->trigger_parameter_);

    settings->Set(this->strobe_source_);
    settings->Set(this->strobe_enable_);
    settings->Set(this->strobe_polarity_);
    settings->Set(this->strobe_delay_);
    settings->Set(this->strobe_duration_);

    return ret;
}


/**
 * @brief Disconnects the camera object from the camera
 * @retval CAMERA_NOT_DISCONNECTED      Camera could NOT be disconnected
 */
ReturnCode PG_FlyCap2_C::Disconnect()
{
    ReturnCode  ret;
    fc2Error    camera_error;
    fc2Context* camera_context = (fc2Context*)this->camera_context_;

    if(!this->isConnected())
        return ret.AddError(CAMERA_NOT_CONNECTED);


    // Stop the capture
    if(this->flycap_callback_started_){
        this->debug_.Msg("Requesting camera to stop capture...");
        fc2StopCapture((*camera_context));
        this->flycap_callback_started_ = false;
    }

    // Disconnects the fc2Context from the camera
    this->debug_.Msg("Disconnecting from camera...");
    camera_error = fc2Disconnect((*camera_context));
    if(camera_error != FC2_ERROR_OK)
    {
        return ret.AddError(CAMERA_NOT_DISCONNECTED);
    }

    // Mark flag that the camera has been disconnected
    this->is_connected_ = false;


    this->debug_.Msg("Camera disconnected.");

    return ret;
}

/** @brief      Image capture callback function to ensure all images are grabbed from camera
 */
void OnImageGrabbed(fc2Image *image, void *pCallbackData)
{
    fc2Image image_copy;
    PG_FlyCapImageBuffer* image_buffer = (PG_FlyCapImageBuffer*)pCallbackData;

    // Check that the callback has not been requested to stop
    if(!image_buffer->stop.test_and_set()){

        // Lock the atomic flag to add the new image to the queue
        while (image_buffer->lock.test_and_set()) {}

        if(image_buffer->store_capture){
            // Create memory for the new image and copy the data from the pointer
            fc2CreateImage(&image_copy);
            fc2ConvertImage(image, &image_copy);

            // Save the new image to the queue
            image_buffer->queue.push(image_copy);

            // Check that the queue has not exceeded its maximum length
            if(image_buffer->queue.size() > image_buffer->max_count){
                // Queue has exceeded the maximum size so remove the oldest frame
                fc2DestroyImage(&image_buffer->queue.front());
                image_buffer->queue.pop();
            }
        }

        // Unlock the atomic flag
        image_buffer->lock.clear();
    }

    // Unlock the stop flag
    image_buffer->stop.clear();

    return;
}




/** @brief      Starts the image capture and clears image buffer
 *  @retval     CAMERA_NOT_CONNECTED        Camera is not connected
 *  @retval     CAMERA_NOT_STARTED          Camera cannot be started
 */
ReturnCode PG_FlyCap2_C::Start()
{
    ReturnCode ret;
    fc2Error        camera_error;
    fc2Context*     camera_context = (fc2Context*)this->camera_context_;
    PG_FlyCapImageBuffer* image_buffer = (PG_FlyCapImageBuffer*)this->image_buffer_;

    if (!(this->is_connected_  && this->is_setup_))
    {
        this->debug_.Msg("Connect and Set up the camera before starting it");
        return ret.AddError(CAMERA_NOT_CONNECTED);
    }

    // Only clear the buffer and start the callback if it hasn't
    // already been started
    if(!this->is_started_){
        this->debug_.Msg("Clearing the image buffer...");

        // Request the callback function to stop
        while (image_buffer->lock.test_and_set()) {}

        // Tell capture callback to store images
        image_buffer->store_capture = true;

        // Clear the image queue
        while(!image_buffer->queue.empty()){
            // Destroy the image structure contents of the oldest stored frame
            camera_error = fc2DestroyImage(&image_buffer->queue.front());
            image_buffer->queue.pop();
        }

        // Save the maximum queue size
        image_buffer->max_count = this->image_queue_max_frames_.Get();

        // Clear the lock
        image_buffer->lock.clear();
        image_buffer->stop.clear();

        // If the callback is not running start it
        if(!this->flycap_callback_started_){
            this->debug_.Msg("Start capturing images...");
            camera_error = fc2StartCaptureCallback( (*camera_context),OnImageGrabbed, this->image_buffer_ );

            if (camera_error != FC2_ERROR_OK)
            {
                this->flycap_callback_started_ = false;
                this->debug_.Msg("Failed to Start the camera");
                return ret.AddError(CAMERA_NOT_STARTED);
            }

            // Mark that the callback has been started
            this->flycap_callback_started_ = true;
        }


        this->debug_.Msg("Camera started.");

        // Mark flag that camera has started
        this->is_started_ = true;

    }

    return ret;
}

/** @brief  Stops image capture thread from storing images in the buffer
 *  @retval CAMERA_NOT_STOPPED      Camera cannot be stopped
 */
ReturnCode PG_FlyCap2_C::Stop()
{
    ReturnCode ret;
    PG_FlyCapImageBuffer* image_buffer = (PG_FlyCapImageBuffer*)this->image_buffer_;


    // Request the callback function to stop
    while (image_buffer->lock.test_and_set()) {}

    this->debug_.Msg("Stopping the camera...");

    image_buffer->store_capture = false;


    // Clear the lock
    image_buffer->lock.clear();

    this->debug_.Msg("Camera stopped");

    // Mark flag that camera has started
    this->is_started_ = false;

    return ret;
}


/** * @brief        Retrieves the oldest frame from the camera buffer and removes it from the buffer.
 *  @param[out]     ret_frame   Pointer to \ref dlp::Image object that holds the captured frame
 *  @retval         PG_FLYCAP_C_IMAGE_BUFFER_EMPTY   No images in buffer to grab
 */
ReturnCode PG_FlyCap2_C::GetFrameBuffered(Image* ret_frame){
    ReturnCode ret;
    PG_FlyCapImageBuffer* image_buffer = (PG_FlyCapImageBuffer*)this->image_buffer_;
    fc2Image pg_image_formatted;

    // Make sure that a new frame isn't being added to the queue
    while (image_buffer->lock.test_and_set()) {}

    // Check that there are images
    if(!image_buffer->queue.empty()){

        fc2CreateImage(&pg_image_formatted);

        // Retreive the oldest image and convert fc2Image image to dlp::Image
        if(this->pixel_format_.Get() != PG_FlyCap2_C::PixelFormat::RGB8){
            // Convert the original data from the camera (e.g. RAW8)
            // to useable data to interpret
            fc2ConvertImageTo(FC2_PIXEL_FORMAT_MONO8,
                              &image_buffer->queue.front(),   // Oldest frame from camera
                              &pg_image_formatted);

            // Convert the fc2Image to dlp::Image
            ret = ret_frame->Create(pg_image_formatted.cols,
                                    pg_image_formatted.rows,
                                    Image::Format::MONO_UCHAR,
                                    pg_image_formatted.pData);
        }
        else{
            // Convert the original data from the camera (e.g. RAW8)
            // to useable data to interpret
            fc2ConvertImageTo(FC2_PIXEL_FORMAT_RGB,
                              &image_buffer->queue.front(),   // Oldest frame from camera
                              &pg_image_formatted);

            // Convert the fc2Image to dlp::Image
            ret = ret_frame->Create(pg_image_formatted.cols,
                                    pg_image_formatted.rows,
                                    Image::Format::RGB_UCHAR,
                                    pg_image_formatted.pData);
        }

        fc2DestroyImage(&image_buffer->queue.front());
        fc2DestroyImage(&pg_image_formatted);
        image_buffer->queue.pop();
    }
    else{
        ret.AddError(PG_FLYCAP_C_IMAGE_BUFFER_EMPTY);
    }

    // Clear the lock
    image_buffer->lock.clear();

    return ret;
}



/** @brief        Retrieves the newest frame from the camera buffer. It does NOT remove the image from the buffer.
 *  @param[out]     ret_frame   Pointer to \ref dlp::Image object that holds the captured frame
 *  @retval         PG_FLYCAP_C_IMAGE_BUFFER_EMPTY   No images in buffer to grab
 */
ReturnCode PG_FlyCap2_C::GetFrame(Image *ret_frame)
{
    ReturnCode ret;
    PG_FlyCapImageBuffer* image_buffer = (PG_FlyCapImageBuffer*)this->image_buffer_;
    fc2Image pg_image_formatted;

    // Make sure that a new frame isn't being added to the queue
    while (image_buffer->lock.test_and_set()) {}

    // Check that there are images
    if(!image_buffer->queue.empty()){

        fc2CreateImage(&pg_image_formatted);

        // Retreive the oldest image and convert fc2Image image to dlp::Image
        if(this->pixel_format_.Get() != PG_FlyCap2_C::PixelFormat::RGB8){            
            // Convert the original data from the camera (e.g. RAW8)
            // to useable data to interpret
            fc2ConvertImageTo(FC2_PIXEL_FORMAT_MONO8,
                              &image_buffer->queue.back(),   // Newest frame from camera
                              &pg_image_formatted);

            // Convert the fc2Image to dlp::Image
            ret = ret_frame->Create(pg_image_formatted.cols,
                                    pg_image_formatted.rows,
                                    Image::Format::MONO_UCHAR,
                                    pg_image_formatted.pData);
        }
        else{
            // Convert the original data from the camera (e.g. RAW8)
            // to useable data to interpret
            fc2ConvertImageTo(FC2_PIXEL_FORMAT_RGB,
                              &image_buffer->queue.back(),   // Newest frame from camera
                              &pg_image_formatted);

            // Convert the fc2Image to dlp::Image
            ret = ret_frame->Create(pg_image_formatted.cols,
                                    pg_image_formatted.rows,
                                    Image::Format::RGB_UCHAR,
                                    pg_image_formatted.pData);
        }

        fc2DestroyImage(&pg_image_formatted);
    }
    else{
        ret.AddError(PG_FLYCAP_C_IMAGE_BUFFER_EMPTY);
    }

    // Clear the lock
    image_buffer->lock.clear();

    return ret;
}


/**
 * @brief           Return a capture sequence with specified number of image \ref dlp::Image captures
 * @param[in]       arg_number_captures  Number of captures to be added in the sequence
 * @param[out]      ret_capture_sequence Pointer to a \ref dlp::Capture::Sequence object that holds the capture sequence
 * @retval          CAMERA_FRAME_GRAB_FAILED    Camera frame NOT grabbed
 */
ReturnCode PG_FlyCap2_C::GetCaptureSequence(const unsigned int &arg_number_captures, Capture::Sequence* ret_capture_sequence)
{
    ReturnCode ret;
    PG_FlyCapImageBuffer* image_buffer = (PG_FlyCapImageBuffer*)this->image_buffer_;

    Capture pg_capture;
    Image pg_image;


    // Save the old buffer size
    //unsigned int old_buffer_size = image_buffer->max_count;

    // Change the buffer size to the required number of patterns
    //image_buffer->max_count = arg_number_captures;

    // Check that the buffer is large enough to capture the number of images requested
    if(image_buffer->max_count < arg_number_captures)
        return ret.AddError(CAMERA_FRAME_GRAB_FAILED);

    // Start the camera capture
    ret = this->Start();

    if(ret.hasErrors()) return ret;

    // Let the camera grab the required images
    unsigned int count = 0;
    while(count < arg_number_captures){

        // Make sure that a new frame isn't being added to the queue
        while (image_buffer->lock.test_and_set()) {}

        // Save the number of captures
        count = image_buffer->queue.size();

        if(count == arg_number_captures){
            ret = this->Stop();
            if(ret.hasErrors()) return ret;
        }

        image_buffer->lock.clear();
    }

    //image_buffer->max_count = old_buffer_size;

    pg_capture.data_type = dlp::Capture::DataType::IMAGE_DATA;

    // Grab arg_num_Captures number of frames from the camera
    for (unsigned int i = 0; i < arg_number_captures; i++)
    {
        // Grab a frame from the camera
        ret = this->GetFrame(&pg_image);

        if (ret.hasErrors())
        {
            this->debug_.Msg("Camera Capture Error");
            return ret;
        }

        pg_capture.image_data = pg_image;

        if (ret_capture_sequence->Add(pg_capture).hasErrors())
        {
            this->debug_.Msg("Camera Captures cannot be added to the Capture Sequence");
            ret.AddError(CAMERA_FRAME_GRAB_FAILED);
            return ret;
        }

        pg_capture.image_data.Clear();
    }

    return ret;
}

/** @brief Returns true if camera is connected via USB */
bool PG_FlyCap2_C::isConnected() const{
    return this->is_connected_;
}

/** @brief Returns true if camera frame captures have been started */
bool PG_FlyCap2_C::isStarted() const{
    return this->is_started_;
}

/** @brief          Returns ID of the camera connected to the system
 *  @param[out]     ret_id  Pointer to retrieve the camera ID
 *  @retval         CAMERA_NOT_CONNECTED    Camera is NOT Connected
 */
ReturnCode PG_FlyCap2_C::GetID(std::string *ret_id) const{
    ReturnCode ret;
    if (!this->isConnected())
    {
        this->debug_.Msg("No Camera is connected");
        ret.AddError(CAMERA_NOT_CONNECTED);
    }
    else{
        *ret_id = this->camera_id_;
    }

    return ret;
}

/** @brief  Returns number of active rows set on the camera sensor */
ReturnCode PG_FlyCap2_C::GetRows(unsigned int* ret_rows) const{
    ReturnCode ret;

    // Check that the camera is connected
    if (!this->isConnected())
    {
        this->debug_.Msg("No Camera is connected");
        return ret.AddError(CAMERA_NOT_CONNECTED);
    }


    // Return the rows
    (*ret_rows) = this->height_.Get();

    return ret;
}

/** @brief Returns number of active columns set on the camera sensor */
ReturnCode PG_FlyCap2_C::GetColumns(unsigned int* ret_columns) const {
    ReturnCode ret;

    // Check that the camera is connected
    if (!this->isConnected())
    {
        this->debug_.Msg("No Camera is connected");
        return ret.AddError(CAMERA_NOT_CONNECTED);
    }

    (*ret_columns) = this->width_.Get();

    return ret;
}

/** @brief Returns frame rate set on the camera sensor  */
ReturnCode PG_FlyCap2_C::GetFrameRate(float* ret_framerate) const {
    ReturnCode ret;

    // Check that the camera is connected
    if (!this->isConnected())
    {
        this->debug_.Msg("No Camera is connected");
        return ret.AddError(CAMERA_NOT_CONNECTED);
    }

    (*ret_framerate) = this->frame_rate_.Get();

    return ret;
}

/** @brief Returns exposure value set on the camera sensor */
ReturnCode PG_FlyCap2_C::GetExposure(float* ret_exposure) const {
    ReturnCode ret;

    // Check that the camera is connected
    if (!this->isConnected())
    {
        this->debug_.Msg("No Camera is connected");
        return ret.AddError(CAMERA_NOT_CONNECTED);
    }

    (*ret_exposure) = this->shutter_.Get();

    return ret;
}


namespace Number{
template <> std::string ToString<dlp::PG_FlyCap2_C::PixelFormat>( dlp::PG_FlyCap2_C::PixelFormat format ){
    switch(format){
    case dlp::PG_FlyCap2_C::PixelFormat::RAW8:      return "RAW8";
    case dlp::PG_FlyCap2_C::PixelFormat::MONO8:     return "MONO8";
    case dlp::PG_FlyCap2_C::PixelFormat::RGB8:      return "RGB8";
    case dlp::PG_FlyCap2_C::PixelFormat::INVALID:   return "INVALID";
    }
    return "INVALID";
}
}

namespace String{
template <> dlp::PG_FlyCap2_C::PixelFormat ToNumber( const std::string &text, unsigned int base ){
    // Ignore base variable
    if (text.compare("RAW8") == 0){
        return dlp::PG_FlyCap2_C::PixelFormat::RAW8;
    }
    else if (text.compare("MONO8") == 0){
        return dlp::PG_FlyCap2_C::PixelFormat::MONO8;
    }
    else if (text.compare("RGB8") == 0){
        return dlp::PG_FlyCap2_C::PixelFormat::RGB8;
    }
    else{
        return dlp::PG_FlyCap2_C::PixelFormat::INVALID;
    }
}
}

}
