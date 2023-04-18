/** @file   OpenCV_Cam.cpp
 *  @brief  Contains methods for \ref dlp::OpenCV_Cam class
 *  @copyright 2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

// DLP Structured Light SDK header files
#include <common/debug.hpp>                     // Adds dlp::Debug
#include <common/other.hpp>                     // Adds dlp::CmdLine, Time, File, String, Number namespaces
#include <common/returncode.hpp>                // Adds dlp::ReturnCode
#include <common/image/image.hpp>               // Adds dlp::Image
#include <common/parameters.hpp>                // Adds dlp::Parameter
#include <camera/camera.hpp>                    // Adds dlp::Camera
#include <camera/opencv_cam/opencv_cam.hpp>     // Adds dlp::OpenCV_Cam

// OpenCV header files
#include <opencv2/opencv.hpp>                   // Adds OpenCV image container
#include <opencv2/highgui/highgui.hpp>          // Adds OpenCV video capture routines

// C++ standard header files
#include <thread>                               // Adds std::thread
#include <vector>                               // Adds std::vector
#include <string>                               // Adds std::string

/** @brief Contains all DLP SDK classes, functions, etc. */
namespace dlp{

/**
 * @brief  Constructs the camera object and clears the buffer
 */
OpenCV_Cam::OpenCV_Cam(){
    this->debug_.SetName("OPENCV_CAM_DEBUG(" + dlp::Number::ToString(this)+ "): ");
    this->is_connected_ = false;
    this->is_setup_     = false;
    this->is_started_   = false;
    this->capture_thread_running_ = false;

    this->image_buffer_.lock.clear();
    this->image_buffer_.continue_capture = false;
    this->image_buffer_.store_capture = false;
    this->image_buffer_.max_count = 50;
}

/**
 * @brief  Deconstructs the camera object and clears the buffer
 */
OpenCV_Cam::~OpenCV_Cam(){
    this->debug_.Msg("Deconstructing...");

    // Release memory from the image buffer
    this->debug_.Msg("Clearing buffer...");
    while(!this->image_buffer_.queue.empty()){
        // Destroy the image structure contents of the oldest stored frame
        this->image_buffer_.queue.front().release();
        this->image_buffer_.queue.pop();
    }

    this->debug_.Msg("Disconnecting...");
    this->Disconnect();
    this->debug_.Msg("Deconstructed");
}


/**
 * @brief   Connects a camera object to the camera specified by the camera Index
 * @retval  CAMERA_ALREADY_CONNECTED        Camera has already been connected
 * @retval  OPENCV_CAM_INVALID_CAMERA_ID    Camera ID sent is invalid
 */
ReturnCode OpenCV_Cam::Connect(const std::string &id){

    ReturnCode ret;

    unsigned int id_int = dlp::String::ToNumber<unsigned int>(id);

    // Check that camera isn't already connected
    if(this->isConnected()){
        this->debug_.Msg("Camera already connected!");
        return ret.AddError(CAMERA_ALREADY_CONNECTED);
    }

    // Attempt to open specified camera
    if(!this->camera_.open(id_int)){
        // Camera failed to connect
        this->debug_.Msg("Camera ID is Invalid");
        return ret.AddError(CAMERA_ID_INVALID);
    }

    // Verify that it opened
    this->is_connected_ = false;
    if(!this->camera_.isOpened()){
        // Camera failed to open
        this->debug_.Msg("Camera could not open");
        return ret.AddError(CAMERA_NOT_CONNECTED);
    }


    // Set camera connected flag and save camera id
    this->is_connected_ = true;
    this->camera_id_ = id;

    // If connected the camera has been setup by OS so mark flag
    this->is_setup_ = true;

    // OpenCV images should always be grabbed so start capture thread
    this->image_buffer_.continue_capture = true;
    this->image_buffer_.store_capture = false;

    // If the capture thread has not started start it
    if(!this->capture_thread_running_){

        this->debug_.Msg("Start capturing images...");

        // Start the window loop
        std::thread capture_thread(&OpenCV_Cam::CaptureThread, this);
        capture_thread.detach();
    }


    // Retrieve the camera settings
    this->width_.Set(       this->camera_.get(CV_CAP_PROP_FRAME_WIDTH));
    this->height_.Set(      this->camera_.get(CV_CAP_PROP_FRAME_HEIGHT));
    this->frame_rate_.Set(  this->camera_.get(CV_CAP_PROP_FPS));
    this->brightness_.Set(  this->camera_.get(CV_CAP_PROP_BRIGHTNESS));
    this->contrast_.Set(    this->camera_.get(CV_CAP_PROP_CONTRAST));
    this->saturation_.Set(  this->camera_.get(CV_CAP_PROP_SATURATION ));
    this->hue_.Set(         this->camera_.get(CV_CAP_PROP_HUE));
    this->gain_.Set(        this->camera_.get(CV_CAP_PROP_GAIN));
    this->exposure_.Set(    this->camera_.get(CV_CAP_PROP_EXPOSURE));

    this->debug_.Msg(0, "**** CAMERA INFORMATION ******");
    this->debug_.Msg(0, "Camera Resolution = " + this->width_.GetEntryValue() +"x"+this->height_.GetEntryValue());
    this->debug_.Msg(0, "Camera Frame Rate = " + this->frame_rate_.GetEntryValue());
    this->debug_.Msg(0, "Camera Brightness = " + this->brightness_.GetEntryValue());
    this->debug_.Msg(0, "Camera Contrast   = " + this->contrast_.GetEntryValue());
    this->debug_.Msg(0, "Camera Saturation = " + this->saturation_.GetEntryValue());
    this->debug_.Msg(0, "Camera Hue        = " + this->hue_.GetEntryValue());
    this->debug_.Msg(0, "Camera Gain       = " + this->gain_.GetEntryValue());
    this->debug_.Msg(0, "Camera Exposure   = " + this->exposure_.GetEntryValue());

    return ret;
}


/** @brief   Configures the camera
 * @param[in] settings    \ref dlp::Parameters object to retrieve entries
 * @retval    CAMERA_NOT_CONNECTED            Camera is NOT Connected
 */
ReturnCode OpenCV_Cam::Setup(const dlp::Parameters &settings){
    ReturnCode ret;

    // Check that the Camera is connected
    if(!this->isConnected()) return ret.AddError(CAMERA_NOT_CONNECTED);

    // Lock the capture thread
    while (this->image_buffer_.lock.test_and_set()) {}

    if(settings.Contains(this->width_)){
        settings.Get(&this->width_);
        this->debug_.Msg("Setting width...");
        if(!this->camera_.set(CV_CAP_PROP_FRAME_WIDTH,this->width_.Get())){
            ret.AddWarning(OPENCV_CAM_SET_FRAME_WIDTH_FAILED);
        }
    }

    if(settings.Contains(this->height_)){
        settings.Get(&this->height_);
        this->debug_.Msg("Setting height...");
        if(!this->camera_.set(CV_CAP_PROP_FRAME_HEIGHT,this->height_.Get())){
            ret.AddWarning(OPENCV_CAM_SET_FRAME_HEIGHT_FAILED);
        }
    }

    if(settings.Contains(this->frame_rate_)){
        settings.Get(&this->frame_rate_);
        this->debug_.Msg("Setting frame rate...");
        if(!this->camera_.set(CV_CAP_PROP_FPS,this->frame_rate_.Get())){
            ret.AddWarning(OPENCV_CAM_SET_FRAME_RATE_FAILED);
        }
    }

    if(settings.Contains(this->brightness_)){
        settings.Get(&this->brightness_);
        this->debug_.Msg("Setting brightness...");
        if(!this->camera_.set(CV_CAP_PROP_BRIGHTNESS,this->brightness_.Get())){
            ret.AddWarning(OPENCV_CAM_SET_BRIGHTNESS_FAILED);
        }
    }

    if(settings.Contains(this->contrast_)){
        settings.Get(&this->contrast_);
        this->debug_.Msg("Setting contrast...");
        if(!this->camera_.set(CV_CAP_PROP_CONTRAST,this->contrast_.Get())){
            ret.AddWarning(OPENCV_CAM_SET_CONTRAST_FAILED);
        }
    }

    if(settings.Contains(this->saturation_)){
        settings.Get(&this->saturation_);
        this->debug_.Msg("Setting saturation...");
        if(!this->camera_.set(CV_CAP_PROP_SATURATION,this->saturation_.Get())){
            ret.AddWarning(OPENCV_CAM_SET_SATURATION_FAILED);
        }
    }

    if(settings.Contains(this->hue_)){
        settings.Get(&this->hue_);
        this->debug_.Msg("Setting hue...");
        if(!this->camera_.set(CV_CAP_PROP_HUE,this->hue_.Get())){
            ret.AddWarning(OPENCV_CAM_SET_HUE_FAILED);
        }
    }

    if(settings.Contains(this->gain_)){
        settings.Get(&this->gain_);
        this->debug_.Msg("Setting gain...");
        if(!this->camera_.set(CV_CAP_PROP_GAIN,this->gain_.Get())){
            ret.AddWarning(OPENCV_CAM_SET_GAIN_FAILED);
        }
    }

    if(settings.Contains(this->exposure_)){
        settings.Get(&this->exposure_);
        this->debug_.Msg("Setting exposure...");
        if(!this->camera_.set(CV_CAP_PROP_EXPOSURE,this->exposure_.Get())){
            ret.AddWarning(OPENCV_CAM_SET_EXPOSURE_FAILED);
        }
    }

    // Retreive the maximum image buffer size
    settings.Get(&this->image_queue_max_frames_);
    this->image_buffer_.max_count = this->image_queue_max_frames_.Get();

    // Retrieve the camera settings
    this->width_.Set(       this->camera_.get(CV_CAP_PROP_FRAME_WIDTH));
    this->height_.Set(      this->camera_.get(CV_CAP_PROP_FRAME_HEIGHT));
    this->frame_rate_.Set(  this->camera_.get(CV_CAP_PROP_FPS));
    this->brightness_.Set(  this->camera_.get(CV_CAP_PROP_BRIGHTNESS));
    this->contrast_.Set(    this->camera_.get(CV_CAP_PROP_CONTRAST));
    this->saturation_.Set(  this->camera_.get(CV_CAP_PROP_SATURATION ));
    this->hue_.Set(         this->camera_.get(CV_CAP_PROP_HUE));
    this->gain_.Set(        this->camera_.get(CV_CAP_PROP_GAIN));
    this->exposure_.Set(    this->camera_.get(CV_CAP_PROP_EXPOSURE));

    // Clear the image buffer lock flag to allow capture
    this->image_buffer_.lock.clear();

    // Mark setup flag as true
    this->is_setup_ = true;

    return ret;
}

/**
 * @brief Returns the camera configuration settings
 * @param[out] settings         Pointer to \ref dlp::Parameters object to store settings in
 * @retval  CAMERA_NOT_SETUP    Camera has NOT been set up/configured
 */
ReturnCode OpenCV_Cam::GetSetup(dlp::Parameters *settings)const{
    ReturnCode ret;

    // Check that pointer is NOT null
    if(!settings)
        return ret.AddError(OPENCV_CAM_NULL_POINTER);

    if(!this->isSetup())
        return ret.AddError(CAMERA_NOT_SETUP);

    // Clear the parameters list
    settings->Clear();

    // Save the settings
    settings->Set(this->height_);
    settings->Set(this->width_);
    settings->Set(this->frame_rate_);
    settings->Set(this->brightness_);
    settings->Set(this->contrast_);
    settings->Set(this->saturation_);
    settings->Set(this->hue_);
    settings->Set(this->gain_);
    settings->Set(this->exposure_);
    settings->Set(this->image_queue_max_frames_);

    return ret;
}


/**
 * @brief Disconnects the camera object from the camera
 * @retval CAMERA_NOT_DISCONNECTED      Camera could NOT be disconnected
 */
ReturnCode OpenCV_Cam::Disconnect()
{
    ReturnCode  ret;

    // Stop the capture
    if(this->capture_thread_running_){
        this->debug_.Msg("Requesting camera to stop capture...");

        // Lock the atomic flag to add the new image to the queue
        while (this->image_buffer_.lock.test_and_set()) {}

        this->image_buffer_.continue_capture = false;
        this->image_buffer_.lock.clear();

        // Wait for the thread to finish
        while(this->capture_thread_running_){}
    }

    // Disconnects the fc2Context from the camera
    this->debug_.Msg("Disconnecting from camera...");

    this->camera_.release();

    // Mark flag that the camera has been disconnected
    this->is_connected_ = false;

    this->debug_.Msg("Camera disconnected.");

    return ret;
}

/** @brief      Capture thread to grab images from the camera and store in buffer
 */
void OpenCV_Cam::CaptureThread()
{
    // Mark that the thread has started
    this->capture_thread_running_ = true;

    // Check that the callback has not been requested to stop
    bool continue_thread = true;
    while(continue_thread){
        cv::Mat new_frame;
        bool read_success = false;

        // Always grab the frame from the camera if open
        if(this->camera_.isOpened())
            read_success = this->camera_.read(new_frame);

        // Lock the atomic flag to add the new image to the queue
        while (this->image_buffer_.lock.test_and_set()) {}

        // Check if capture should continue
        continue_thread = this->image_buffer_.continue_capture;

        if(this->image_buffer_.store_capture && continue_thread){

            // Grab the new frame
            if(read_success){

                // Save the new image to the queue
                this->image_buffer_.queue.push(new_frame.clone());

                // Check that the queue has not exceeded its maximum length
                if(this->image_buffer_.queue.size() > this->image_buffer_.max_count){
                    // Queue has exceeded the maximum size so remove the oldest frame
                    this->image_buffer_.queue.front().release();
                    this->image_buffer_.queue.pop();
                }
            }
        }

        // Unlock the atomic flag
        this->image_buffer_.lock.clear();
    }

    // Mark that the thread has finished
    this->capture_thread_running_ = false;

    return;
}




/** @brief      Starts the image capature thread and clears the image buffer
 *  @retval     CAMERA_NOT_CONNECTED        Camera is not connected
 */
ReturnCode OpenCV_Cam::Start(){
    ReturnCode ret;

    if (!(this->is_connected_  && this->is_setup_)){
        this->debug_.Msg("Connect and Set up the camera before starting it");
        return ret.AddError(CAMERA_NOT_CONNECTED);
    }

    // Only clear the buffer and start the callback if it hasn't
    // already been started
    if(!this->is_started_){
        this->debug_.Msg("Clearing the image buffer...");

        // Request the callback function to stop
        while (this->image_buffer_.lock.test_and_set()) {}

        // Tell capture callback to store images
        this->image_buffer_.store_capture    = true;
        this->image_buffer_.continue_capture = true;

        // Clear the image queue
        while(!this->image_buffer_.queue.empty()){
            // Destroy the image structure contents of the oldest stored frame
            this->image_buffer_.queue.front().release();
            this->image_buffer_.queue.pop();
        }

        // Save the maximum queue size
        this->image_buffer_.max_count = this->image_queue_max_frames_.Get();

        // Clear the lock
        this->image_buffer_.lock.clear();

        this->debug_.Msg("Camera started.");

        // Mark flag that camera has started
        this->is_started_ = true;
    }

    return ret;
}

/** @brief  Stops the capture thread from storing images in the buffer
 *  @retval CAMERA_NOT_STOPPED      Camera cannot be stopped
 */
ReturnCode OpenCV_Cam::Stop()
{
    ReturnCode ret;

    // Request the callback function to stop
    while (this->image_buffer_.lock.test_and_set()) {}

    this->debug_.Msg("Stopping the camera...");

    this->image_buffer_.store_capture = false;

    // Clear the lock
    this->image_buffer_.lock.clear();

    this->debug_.Msg("Camera stopped");

    // Mark flag that camera has started
    this->is_started_ = false;

    return ret;
}


/** * @brief        Retrieves the oldest frame from the camera buffer and removes it from the buffer.
 *  @param[out]     ret_frame   Pointer to \ref dlp::Image object that holds the captured frame
 *  @retval         OPENCV_CAM_IMAGE_BUFFER_EMPTY   No images in buffer to grab
 */
ReturnCode OpenCV_Cam::GetFrameBuffered(Image* ret_frame){
    ReturnCode ret;

    // Make sure that a new frame isn't being added to the queue
    while (this->image_buffer_.lock.test_and_set()) {}

    // Check that there are images
    if(!this->image_buffer_.queue.empty()){
        // Grab the oldest frame
        ret = ret_frame->Create(this->image_buffer_.queue.front());

        // Remove the oldest frame from buffer
        this->image_buffer_.queue.front().release();
        this->image_buffer_.queue.pop();
    }
    else{
        ret.AddError(OPENCV_CAM_IMAGE_BUFFER_EMPTY);
    }

    // Clear the lock
    this->image_buffer_.lock.clear();

    return ret;
}


/** * @brief        Retrieves the newest frame from the camera buffer. It does NOT remove the image from the buffer.
 *  @param[out]     ret_frame   Pointer to \ref dlp::Image object that holds the captured frame
 *  @retval         OPENCV_CAM_IMAGE_BUFFER_EMPTY   No images in buffer to grab
 */
ReturnCode OpenCV_Cam::GetFrame(Image *ret_frame)
{
    ReturnCode ret;

    // Make sure that a new frame isn't being added to the queue
    while (this->image_buffer_.lock.test_and_set()) {}

    // Check that there are images
    if(!this->image_buffer_.queue.empty()){
        // Grab the newest frame
        ret = ret_frame->Create(this->image_buffer_.queue.back());
    }
    else{
        ret.AddError(OPENCV_CAM_IMAGE_BUFFER_EMPTY);
    }

    // Clear the lock
    this->image_buffer_.lock.clear();

    return ret;
}


/**
 * @brief           Return a capture sequence with specified number of image \ref dlp::Image captures
 * @param[in]       arg_number_captures  Number of captures to be added in the sequence
 * @param[out]      ret_capture_sequence Pointer to a \ref dlp::Capture::Sequence object that holds the capture sequence
 * @retval          CAMERA_FRAME_GRAB_FAILED    Camera frame NOT grabbed
 */
ReturnCode OpenCV_Cam::GetCaptureSequence(const unsigned int &arg_number_captures, Capture::Sequence* ret_capture_sequence)
{
    ReturnCode ret;

    Capture cv_capture;
    Image new_image;


    // Check that the buffer is large enough to capture the number of images requested
    if(this->image_buffer_.max_count < arg_number_captures)
        return ret.AddError(CAMERA_FRAME_GRAB_FAILED);

    // Stop and then start the camera capture
    ret = this->Stop();
    if(ret.hasErrors()) return ret;

    ret = this->Start();
    if(ret.hasErrors()) return ret;


    // Let the camera grab the required images
    unsigned int count = 0;
    while(count < arg_number_captures){

        // Make sure that a new frame isn't being added to the queue
        while (this->image_buffer_.lock.test_and_set()) {}

        // Save the number of captures
        count = this->image_buffer_.queue.size();

        if(count == arg_number_captures){
            ret = this->Stop();
            if(ret.hasErrors()) return ret;
        }

        this->image_buffer_.lock.clear();
    }

    cv_capture.data_type = dlp::Capture::DataType::IMAGE_DATA;

    // Grab arg_num_Captures number of frames from the camera
    for (unsigned int i = 0; i < arg_number_captures; i++)
    {
        // Grab a frame from the camera
        ret = this->GetFrame(&new_image);

        if (ret.hasErrors())
        {
            this->debug_.Msg("Camera Capture Error");
            return ret;
        }

        cv_capture.image_data = new_image;

        if (ret_capture_sequence->Add(cv_capture).hasErrors())
        {
            this->debug_.Msg("Camera Captures cannot be added to the Capture Sequence");
            ret.AddError(CAMERA_FRAME_GRAB_FAILED);
            return ret;
        }

        cv_capture.image_data.Clear();
    }

    return ret;
}

/** @brief Returns true if camera is connected */
bool OpenCV_Cam::isConnected() const{
    return this->camera_.isOpened() && this->is_connected_;
}

/** @brief Returns true if camera frame captures have been started
 */
bool OpenCV_Cam::isStarted() const{
    return this->is_started_;
}

/** @brief          Returns ID of the camera connected to the system
 *  @param[out]     ret_id  Pointer to retrieve the camera ID
 *  @retval         CAMERA_NOT_CONNECTED    Camera is NOT Connected
 */
ReturnCode OpenCV_Cam::GetID(std::string *ret_id) const{
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

/** @brief Returns number of active rows set on the camera sensor
 */
ReturnCode OpenCV_Cam::GetRows(unsigned int* ret_rows) const{
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

/** @brief Returns number of active columns set on the camera sensor
 */
ReturnCode OpenCV_Cam::GetColumns(unsigned int* ret_columns) const {
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

/** @brief Returns frame rate set on the camera sensor
 */
ReturnCode OpenCV_Cam::GetFrameRate(float* ret_framerate) const {
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

/** @brief Returns exposure value set on the camera sensor
 */
ReturnCode OpenCV_Cam::GetExposure(float* ret_exposure) const {
    ReturnCode ret;

    // Check that the camera is connected
    if (!this->isConnected())
    {
        this->debug_.Msg("No Camera is connected");
        return ret.AddError(CAMERA_NOT_CONNECTED);
    }

    if(this->frame_rate_.Get() > 0) (*ret_exposure) = 1000/this->frame_rate_.Get();
    else (*ret_exposure) = 0;

    return ret;
}

}
