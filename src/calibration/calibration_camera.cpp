/** @file       calibration_camera.cpp
 *  @brief      Contains OpenCV calibration routines for a camera
 *  @copyright  2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

// DLP Structured Light SDK header files
#include <common/debug.hpp>                     // Adds dlp::Debug
#include <common/other.hpp>                     // Adds dlp::CmdLine, Time, File, String, Number namespaces
#include <common/returncode.hpp>                // Adds dlp::ReturnCode
#include <common/image/image.hpp>               // Adds dlp::Image
#include <common/parameters.hpp>                // Adds dlp::Parameter
#include <camera/camera.hpp>                    // Adds dlp::Camera
#include <calibration/calibration.hpp>          // Adds dlp::Calibration::Camera and dlp::Calibration::Data

// OpenCV header files
#include <opencv2/opencv.hpp>                   // Adds OpenCV image container
#include <opencv2/highgui/highgui.hpp>          // Adds OpenCV camera calibration routines

// C++ standard header files
#include <vector>                               // Adds std:vector
#include <string>                               // Adds std::string

/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{


/** @brief  Constructs empty Calibration::Camera object */
Calibration::Camera::Camera(){
    this->debug_.SetName("CALIBRATION_CAMERA_DEBUG(" + dlp::Number::ToString(this)+ "): ");
    this->debug_.Msg("Constructing object...");
    this->ClearAll();
    this->debug_.Msg("Object constructed");
}


/** @brief  Destroys Calibration::Camera object and releases all allocated memory */
Calibration::Camera::~Camera(){
    this->debug_.Msg("Deconstructing object...");
    this->ClearAll();
    this->debug_.Msg("Object deconstructed");
}


/** @brief Resets all settings and sets all calibration data to zero
 *
 *  @code
 *  dlp::Calibration::Camera camera_calibration;
 *  camera_calibration.ClearAll();
 *  @endcode
 */
void Calibration::Camera::ClearAll(){
    this->debug_.Msg("Clearing all calibration settings and data...");

    this->model_rows_.Set(0);
    this->model_columns_.Set(0);
    this->image_rows_.Set(0);
    this->image_columns_.Set(0);
    this->board_number_required_.Set(0);
    this->board_color_foreground_.Set(dlp::PixelRGB(255,255,255));
    this->board_color_background_.Set(dlp::PixelRGB(0,0,0));
    this->board_columns_.Set(0);
    this->board_column_distance_.Set(0);
    this->board_column_distance_in_pixels_.Set(0);
    this->board_rows_.Set(0);
    this->board_row_distance_.Set(0);
    this->board_row_distance_in_pixels_.Set(0);
    this->zero_tangent_distortion_.Set(false);
    this->is_setup_     = false;
    this->camera_set_   = false;
    this->calibration_board_feature_points_xyz_.clear();

    // Clear all points
    this->ClearCalibrationImagePoints();

    // Clear the calibration data
    this->ClearCalibrationData();

    this->debug_.Msg("All calibration settings and data cleared");
}

/** @brief Sets all calibration data to zero but retains other settings
 *
 *  @code
 *  dlp::Calibration::Camera camera_calibration;
 *  camera_calibration.ClearCalibrationData();
 *  @endcode
 */
void Calibration::Camera::ClearCalibrationData(){
    this->debug_.Msg("Clearing calibration data...");

    // Reset boolean values
    this->calibration_data_.Clear();

    this->debug_.Msg("Calibration data cleared");
}

/** @brief Removes all added calibration board image points but retains other settings.
 *
 *  @code
 *  dlp::Calibration::Camera camera_calibration;
 *  camera_calibration.ClearCalibrationImagePoints();
 *  @endcode
 */
void Calibration::Camera::ClearCalibrationImagePoints(){
    this->debug_.Msg("Clearing calibration image points...");

    // Clear the calibration board counter
    this->board_number_successes_ = 0;

    // Clear the point vectors
    this->object_points_xyz_.clear();
    this->image_points_xy_.clear();

    // Clear the homography data
    for(unsigned int iBoard = 0; iBoard < this->calibration_data_.homography_.size(); iBoard++){
        this->calibration_data_.homography_.at(iBoard).release();
    }
    this->calibration_data_.homography_.clear();

    this->debug_.Msg("Calibration image points cleared");
}


/** @brief Returns true if calibration has been successfully completed
 *
 * @code
 * dlp::Calibration::Camera camera_calibraiton;
 *
 * if(camera_calibration.isCalibrationComplete(){
 *      // Calibration is complete
 * }
 * else{
 *      // Calibration is NOT completed
 * }
 * @endcode
 *
 */
bool Calibration::Camera::isCalibrationComplete() const{
    return this->calibration_data_.isComplete();
}

/** @brief      Retrieves calibration settings
 *  @param[out] settings    Pointer to \ref dlp::Parameters object to store settings in
 *  @warning    This method clears the \ref dlp::Parameters object before adding any settings
 *  @retval     CALIBRATION_NOT_SETUP               Calibration has not been setup
 *  @retval     CALIBRATION_NULL_POINTER_SETTINGS   Input argument is NULL
 *
 *  @code
 *  dlp::ReturnCode ret;
 *  dlp::Parameters calibration_parameters;
 *  dlp::Calibration::Camera camera_calibration;
 *
 *  ret = camera_calibration.GetSetup(&calibration_parameters);
 *  @endcode
 */
ReturnCode Calibration::Camera::GetSetup(dlp::Parameters *settings) const{
    ReturnCode ret;
    this->debug_.Msg("Saving camera calibration setup to dlp::Parameters object...");

    // Check that object is setup
    if(!this->isSetup()){
        this->debug_.Msg("Calibration has NOT been setup!");
        ret.AddError(CALIBRATION_NOT_SETUP);
    }

    // Check that the input argument is not null
    if(!settings){
        this->debug_.Msg("Input argument NULL!");
        ret.AddError(CALIBRATION_NULL_POINTER_SETTINGS);
    }

    // Check for errors
    if(ret.hasErrors()) return ret;

    // Clear the parameter object
    settings->Clear();

    // Add the camera calibraiton object's settings
    settings->Set(this->model_rows_);
    settings->Set(this->model_columns_);
    settings->Set(this->image_rows_);
    settings->Set(this->image_columns_);
    settings->Set(this->board_number_required_);
    settings->Set(this->board_color_foreground_);
    settings->Set(this->board_color_background_);
    settings->Set(this->board_columns_);
    settings->Set(this->board_column_distance_);
    settings->Set(this->board_column_distance_in_pixels_);
    settings->Set(this->board_rows_);
    settings->Set(this->board_row_distance_);
    settings->Set(this->board_row_distance_in_pixels_);
    settings->Set(this->zero_tangent_distortion_);
    settings->Set(this->fix_sixth_order_distortion_);

    this->debug_.Msg("Camera calibration setup saved to dlp::Parameters object");

    return ret;
}

/** @brief      Retrieves the number of successfull calibration board images added
 *              and how many are required
 *  @param[out] successfull     Pointer for method to return the number of
 *                              calibration boards successfully added with \ref dlp::Calibration::Camera::AddCalibrationBoard()
 *  @param[out] total_required  Pointer for method to return the required number of
 *                              calibration boards set during \ref dlp::Calibration::Camera::Setup()
 *  @retval     CALIBRATION_NOT_SETUP                   Calibration has not been setup
 *  @retval     CALIBRATION_NULL_POINTER_SUCCESSFUL     Input argument is NULL
 *  @retval     CALIBRATION_NULL_POINTER_TOTAL_REQUIRED Input argument is NULL
 *
 *  @code
 *  unsigned int boards_added;      // Number of boards which have been added successfully with
 *  unsigned int boards_required;   // Total number of boards required
 *  dlp::ReturnCode ret;
 *  dlp::Calibration::Camera camera_calibration;
 *
 *  // Setup camera calibration.
 *  // Add calibration board images.
 *
 *  ret = camera_calibration.GetCalibrationProgress(&boards_added, &boards_required);
 *  @endcode
 *
 */
ReturnCode Calibration::Camera::GetCalibrationProgress(unsigned int *successfull, unsigned int *total_required) const{
    ReturnCode ret;

    this->debug_.Msg("GetCalibrationProgress()");

    // Check that object is setup
    if(!this->isSetup()){
        this->debug_.Msg("Calibration has NOT been setup!");
        ret.AddError(CALIBRATION_NOT_SETUP);
    }

    // Check that pointers are not null
    if(!successfull){
        this->debug_.Msg("Input argument NULL!");
        ret.AddError(CALIBRATION_NULL_POINTER_SUCCESSFUL);
    }

    if(!total_required){
        this->debug_.Msg("Input argument NULL!");
        ret.AddError(CALIBRATION_NULL_POINTER_TOTAL_REQUIRED);
    }

    // Check for errors
    if(ret.hasErrors()) return ret;


    // Return the values
    (*total_required) = this->board_number_required_.Get();
    (*successfull)     = this->board_number_successes_;

    this->debug_.Msg("Number of calibration images added    = " +
                     dlp::Number::ToString(*successfull));
    this->debug_.Msg("Number of required calibration images = " +
                     dlp::Number::ToString(*total_required));

    return ret;
}


/** @brief      Retrieves calibration data
 *  @param[out] data    Pointer for method to return dlp::Calibration::Data object
 *  @retval     CALIBRATION_NOT_SETUP           Calibration has not been setup
 *  @retval     CALIBRATION_NOT_COMPLETE        Calibration has not been completed
 *  @retval     CALIBRATION_NULL_POINTER_DATA   Input argument is NULL
 *
 *  @code
 *  dlp::ReturnCode ret;
 *  dlp::Calibration::Data   calibration_data;
 *  dlp::Calibration::Camera camera_calibration;
 *
 *  // Setup camera calibration.
 *  // Add calibration board images.
 *  // Calibrate
 *
 *  ret = camera_calibration.GetCalibrationData(&calibration_data);
 *  @endcode
 *
 */
ReturnCode Calibration::Camera::GetCalibrationData( dlp::Calibration::Data *data ) const{
    ReturnCode ret;

    this->debug_.Msg("Retrieving calibration data...");

    // Check that object is setup
    if(!this->isSetup()){
        this->debug_.Msg("Calibration has NOT been setup!");
        ret.AddError(CALIBRATION_NOT_SETUP);
    }

    // Check that calibration is complete
    if(!this->isCalibrationComplete()){
        this->debug_.Msg("Calibration has NOT been completed!");
        ret.AddError(CALIBRATION_NOT_COMPLETE);
    }

    // Check that pointers are not null
    if(!data){
        this->debug_.Msg("Input argument NULL!");
        ret.AddError(CALIBRATION_NULL_POINTER_DATA);
    }

    // Check for errors
    if(ret.hasErrors()) return ret;

    // Return the calibration data
    (*data) = this->calibration_data_;

    this->debug_.Msg("Calibration data retrieved");

    return ret;
}

/** @brief      Copies calibration data into the \ref dlp::Calibration::Data
 *  @param[int] data                        dlp::Calibration::Data object to copy data from
 *  @retval     CALIBRATION_NOT_COMPLETE    Supplied calibration data is NOT complete
 *
 *  @code
 *  dlp::ReturnCode ret;
 *  dlp::Calibration::Data   calibration_data;
 *  dlp::Calibration::Camera camera_calibration;
 *
 *  // Setup camera calibration.
 *  // Load calibration data from file
 *
 *  ret = camera_calibration.SetCalibrationData(calibration_data);
 *  @endcode
 *  \note       Useful for updating calibration data if previously completed
 *
 */
ReturnCode Calibration::Camera::SetCalibrationData( dlp::Calibration::Data &data ){
    ReturnCode ret;

    this->debug_.Msg("Setting calibration data...");

    // Check that calibration is complete
    if(!data.isComplete()){
        this->debug_.Msg("Calibration has NOT been completed!");
        return ret.AddError(CALIBRATION_NOT_COMPLETE);
    }

    // Copy the calibration data
    this->calibration_data_ = data;

    this->debug_.Msg("Calibration data set");

    return ret;
}

/** @brief      Retrieves camera resolution to set the calibration board image resolution
 *  \note       If this method is called before \ref Setup() the image rows and columns do NOT
 *              need to be stored in the \ref dlp::Parameters
 *  @param[int] camera              dlp::Camera object to get resolution from
 *  @retval     CAMERA_NOT_SETUP    Camera has not been setup and resolution cannot be
 *                                  retrieved
 *
 *  @code
 *  dlp::ReturnCode ret;
 *  dlp::OpenCV_Cam camera
 *  dlp::Calibration::Camera camera_calibration;
 *  dlp::Parameters          camera_calibration_settings;
 *
 *  // Load calibration settings from file
 *
 *  // Connect and setup camera
 *
 *  // Set the camera first
 *  ret = camera_calibration.SetCamera(camera);
 *
 *  // Setup the calibration module
 *  ret = camera_calibration.Setup(camera_calibration_settings);
 *
 * @endcode
 *
 */
ReturnCode Calibration::Camera::SetCamera(const dlp::Camera &camera){
    ReturnCode ret;

    this->debug_.Msg("Retrieving camera resolution...");

    // Check that camera is setup
    if(!camera.isSetup()){
        this->debug_.Msg("Camera NOT setup!");
        return ret.AddError(CAMERA_NOT_SETUP);
    }

    // Grab the resolution from camera to set the model and image settings
    unsigned int rows;
    unsigned int columns;

    camera.GetRows(&rows);
    camera.GetColumns(&columns);

    // Save these values to the model and image settings
    this->image_rows_.Set(rows);
    this->image_columns_.Set(columns);
    this->camera_set_ = true;

    this->debug_.Msg("Camera resolution  = " +
                     this->image_columns_.GetEntryValue() +
                     " by " +
                     this->image_rows_.GetEntryValue());

    return ret;
}


/** @brief      Sets all required parameters for camera calibration
 *  @param[in]  settings    \ref dlp::Parameters object to retrieve entries
 *  @retval     PARAMETERS_EMPTY                                                    Supplied \ref dlp::Parameters object is empty
 *  @retval     CALIBRATION_PARAMETERS_MODEL_SIZE_MISSING                             The model resolution settings are missing (\ref dlp::Calibration::Camera::model_rows_ and/or \ref dlp::Calibration::Camera::model_columns_)
 *  @retval     CALIBRATION_PARAMETERS_IMAGE_SIZE_MISSING                             The image resolution settings are missing (\ref dlp::Calibration::Camera::image_rows_ and/or \ref dlp::Calibration::Camera::image_columns_)
 *  @retval     CALIBRATION_PARAMETERS_NUMBER_BOARDS_MISSING                          The number of calibration board images is missing (\ref dlp::Calibration::Camera::board_number_required_)
 *  @retval     CALIBRATION_PARAMETERS_BOARD_FEATURE_SIZE_MISSING                     The board feature resolution settings are missing (\ref dlp::Calibration::Camera::board_rows_ and/or \ref dlp::Calibration::Camera::board_columns_))
 *  @retval     CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_MISSING                 The board feature distance settings are missing (\ref dlp::Calibration::Camera::board_row_distance_ and/or \ref dlp::Calibration::Camera::board_column_distance_))
 *  @retval     CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_IN_PIXELS_MISSING       The board feature distance in pixel settings are missing (\ref dlp::Calibration::Camera::board_row_distance_in_pixels_ and/or \ref dlp::Calibration::Camera::board_column_distance_in_pixels_))
 *  @retval     CALIBRATION_PARAMETERS_TANGENT_DISTORTION_MISSING                     The tangent distortion setting is missing (\ref dlp::Calibration:Camera::zero_tangent_distortion_)
 *  @retval     CALIBRATION_PARAMETERS_SIXTH_ORDER_DISTORTION_MISSING                 The sixth order distortion setting is missing (\ref dlp::Calibration::Camera::fix_sixth_order_distortion_)
 *
 * The following code demonstrates how to use the setup function with hard coded values to calibrate a 640x480 resolution camera:
 *
 * @code
 *
 *  dlp::ReturnCode ret;
 *  dlp::Parameters calibration_parameters;
 *  dlp::Calibration::Camera camera_calibration;
 *
 *  calibration_parameters.Set(dlp::Calibration::Parameters::ModelColumns(640));     // Not required if SetCamera() called previously
 *  calibration_parameters.Set(dlp::Calibration::Parameters::ModelRows(480));        // Not required if SetCamera() called previously
 *  calibration_parameters.Set(dlp::Calibration::Parameters::ImageColumns(640));     // Not required if SetCamera() called previously
 *  calibration_parameters.Set(dlp::Calibration::Parameters::ImageRows(480));        // Not required if SetCamera() called previously
 *  calibration_parameters.Set(dlp::Calibration::Parameters::BoardCount(20));
 *  calibration_parameters.Set(dlp::Calibration::Parameters::BoardForeground(dlp::PixelRGB(255,255,255)));
 *  calibration_parameters.Set(dlp::Calibration::Parameters::BoardBackground(dlp::PixelRGB(150,150,150))); // If calibrating a projector as well, it is recommended to use grey sqaures rather than black ones
 *  calibration_parameters.Set(dlp::Calibration::Parameters::BoardFeatureColumns(16));
 *  calibration_parameters.Set(dlp::Calibration::Parameters::BoardFeatureColumnDistance(16.67));
 *  calibration_parameters.Set(dlp::Calibration::Parameters::BoardFeatureColumnDistancePixels(100));
 *  calibration_parameters.Set(dlp::Calibration::Parameters::BoardFeatureColumnOffsetPixels(700));
 *  calibration_parameters.Set(dlp::Calibration::Parameters::BoardFeatureRows(7));
 *  calibration_parameters.Set(dlp::Calibration::Parameters::BoardFeatureRowDistance(16.67));
 *  calibration_parameters.Set(dlp::Calibration::Parameters::BoardFeatureRowDistancePixels(100));
 *  calibration_parameters.Set(dlp::Calibration::Parameters::BoardFeatureRowOffsetPixels(700));
 *  calibration_parameters.Set(dlp::Calibration::Parameters::SetTangentDistZero(false));
 *  calibration_parameters.Set(dlp::Calibration::Parameters::FixSixthOrderDist(false));
 *
 *  ret = camera_calibration.Setup(calibration_parameters);
 *
 * @endcode
 *
 * Please reference \ref dlp::Calibration::Camera::GenerateCalibrationBoard() to view the calibration board generated with this settings
 *
 * The above code could also be accomplished with the following parameters file and code.
 *
 * Equivalent parameters file saved as "calibration_camera_settings.txt":
 * @code
 * CALIBRATION_PARAMETERS_MODEL_COLUMNS = 640
 * CALIBRATION_PARAMETERS_MODEL_ROWS = 480
 * CALIBRATION_PARAMETERS_IMAGE_COLUMNS = 640
 * CALIBRATION_PARAMETERS_IMAGE_ROWS = 480
 * CALIBRATION_PARAMETERS_BOARD_COUNT   =   20
 * CALIBRATION_PARAMETERS_BOARD_FOREGROUND  =   255, 255, 255
 * CALIBRATION_PARAMETERS_BOARD_BACKGROUND  =   150, 150, 150
 * CALIBRATION_PARAMETERS_BOARD_FEATURE_ROWS    =   7
 * CALIBRATION_PARAMETERS_BOARD_FEATURE_ROW_DISTANCE    =   16.67
 * CALIBRATION_PARAMETERS_BOARD_FEATURE_ROW_DISTANCE_PIXELS =   100
 * CALIBRATION_PARAMETERS_BOARD_FEATURE_ROW_OFFSET_PIXELS   =   700
 * CALIBRATION_PARAMETERS_BOARD_FEATURE_COLUMNS =   10
 * CALIBRATION_PARAMETERS_BOARD_FEATURE_COLUMN_DISTANCE =   16.67
 * CALIBRATION_PARAMETERS_BOARD_FEATURE_COLUMN_DISTANCE_PIXELS  =   100
 * CALIBRATION_PARAMETERS_BOARD_FEATURE_COLUMN_OFFSET_PIXELS    =   700
 * CALIBRATION_PARAMETERS_SET_TANGENT_DIST_TO_ZERO  =   0
 * CALIBRATION_PARAMETERS_FIX_SIXTH_ORDER_DIST  =   0
 * @endcode
 *
 * The following code would then be used to load the settings:
 * @code
 *
 * dlp::ReturnCode ret;
 * dlp::Parameters calibration_parameters;
 * dlp::Calibration::Camera camera_calibration;
 *
 * ret = calibration_parameters.Load("calibration_camera_settings.txt");
 * ret = camera_calibration.Setup(calibration_parameters);
 *
 * @endcode
 *
 */
ReturnCode Calibration::Camera::Setup(const dlp::Parameters &settings){
    ReturnCode ret;

    this->debug_.Msg("Setting up camera calibration object...");

    // Check that settings is not empty
    if(settings.isEmpty()){
        this->debug_.Msg("Setting list is EMPTY!");
        return ret.AddError(PARAMETERS_EMPTY);
    }

    // Reset flags
    this->is_setup_ = false;
    this->board_number_successes_ = 0;

    // If the camera has not been set get the model and image resolutions
    if(!this->camera_set_){
        this->debug_.Msg("Camera was not set, retrieving model and image resolution...");

        if(settings.Get(&this->model_rows_).hasErrors()){
            this->debug_.Msg("Calibration model rows MISSING!");
            ret.AddError(CALIBRATION_PARAMETERS_MODEL_SIZE_MISSING);
        }

        if(settings.Get(&this->model_columns_).hasErrors()){
            this->debug_.Msg("Calibration model columns MISSING!");
            ret.AddError(CALIBRATION_PARAMETERS_MODEL_SIZE_MISSING);
        }

        if(settings.Get(&this->image_rows_).hasErrors()){
            this->debug_.Msg("Calibration image rows MISSING!");
            ret.AddError(CALIBRATION_PARAMETERS_IMAGE_SIZE_MISSING);
        }

        if(settings.Get(&this->image_columns_).hasErrors()){
            this->debug_.Msg("Calibration image columns MISSING!");
            ret.AddError(CALIBRATION_PARAMETERS_IMAGE_SIZE_MISSING);
        }
    }
    else{
        // Camera was set so save image resolution to model resolution
        // since they are the same for a camera
        this->debug_.Msg("Camera was resolution already set");
        this->model_rows_.Set(this->image_rows_.Get());
        this->model_columns_.Set(this->image_columns_.Get());
    }

    this->debug_.Msg("Model resolution = " +
                     this->model_columns_.GetEntryValue() +
                     " by " +
                     this->model_rows_.GetEntryValue());

    this->debug_.Msg("Image resolution = " +
                     this->image_columns_.GetEntryValue() +
                     " by " +
                     this->image_rows_.GetEntryValue());

    // Save model and image resolution to calibration data
    this->debug_.Msg("Saving model and image resolution to calibration data...");
    this->calibration_data_.image_rows_     = this->image_rows_.Get();
    this->calibration_data_.image_columns_  = this->image_columns_.Get();
    this->calibration_data_.model_rows_     = this->model_rows_.Get();
    this->calibration_data_.model_columns_  = this->model_columns_.Get();


    // Get calibration board settings
    this->debug_.Msg("Retrieving calibration board settings...");

    // Get number of calibration boards required
    if(settings.Get(&this->board_number_required_).hasErrors()){
        this->debug_.Msg("Number of required calibration boards MISSING!");
        ret.AddError(CALIBRATION_PARAMETERS_NUMBER_BOARDS_MISSING);
    }
    else{
        this->debug_.Msg("Number of required calibration boards = " + this->board_number_required_.GetEntryValue());
    }

    // Get the calibration board color information (not required)
    settings.Get(&this->board_color_foreground_);
    settings.Get(&this->board_color_background_);
    this->debug_.Msg("Calibration foreground color = " + this->board_color_foreground_.GetEntryValue());
    this->debug_.Msg("Calibration background color = " + this->board_color_background_.GetEntryValue());

    // Get calibration board feature information
    if(settings.Get(&this->board_columns_).hasErrors())
        ret.AddError(CALIBRATION_PARAMETERS_BOARD_FEATURE_SIZE_MISSING);
    this->debug_.Msg("Calibration board column features = " + this->board_columns_.GetEntryValue());

    if(settings.Get(&this->board_column_distance_).hasErrors())
        ret.AddError(CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_MISSING);
    this->debug_.Msg("Calibration board column feature distance = " + this->board_column_distance_.GetEntryValue());

    if(settings.Get(&this->board_column_distance_in_pixels_).hasErrors())
        ret.AddError(CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_IN_PIXELS_MISSING);
    this->debug_.Msg("Calibration board column feature pixels per unit = " + this->board_column_distance_in_pixels_.GetEntryValue());

    if(settings.Get(&this->board_rows_).hasErrors())
        ret.AddError(CALIBRATION_PARAMETERS_BOARD_FEATURE_SIZE_MISSING);
    this->debug_.Msg("Calibration board column features = " + this->board_rows_.GetEntryValue());

    if(settings.Get(&this->board_row_distance_).hasErrors())
        ret.AddError(CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_MISSING);
    this->debug_.Msg("Calibration board column feature distance = " + this->board_column_distance_.GetEntryValue());

    if(settings.Get(&this->board_row_distance_in_pixels_).hasErrors())
        ret.AddError(CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_IN_PIXELS_MISSING);
    this->debug_.Msg("Calibration board column feature pixels per unit = " + this->board_column_distance_in_pixels_.GetEntryValue());

    settings.Get(&this->board_column_offset_pixels_);
    settings.Get(&this->board_row_offset_pixels_);

    // Create calibration_board_feature_points_xyz_ in real x, y, z space
    // Assume that calibration board is both planar and at z = 0
    this->debug_.Msg("Generating camera calibration board points in real space (x,y,z)...");

    // Clear pattern points in case it has already been setup
    this->calibration_board_feature_points_xyz_.clear();

    // Assume that the first feature is located a 0,0 because the chessboard
    // is in real space and has no direct relationship to the camera sensor
    for(     unsigned int iRow = 0; iRow < this->board_rows_.Get();    iRow++ ){
        for( unsigned int iCol = 0; iCol < this->board_columns_.Get();  iCol++ ){
            float x_pos = iCol * this->board_column_distance_.Get();
            float y_pos = iRow * this->board_row_distance_.Get();

            cv::Point3f feature_point = cv::Point3f(x_pos,y_pos,0.0f);
            this->calibration_board_feature_points_xyz_.push_back(feature_point);
        }
    }

    // Check for tangent distortion setting
    if(settings.Get(&this->zero_tangent_distortion_).hasErrors())
        return ret.AddError(CALIBRATION_PARAMETERS_TANGENT_DISTORTION_MISSING);
    this->debug_.Msg("Calibration tangent distortion = " + this->zero_tangent_distortion_.GetEntryValue());

    // Check for sixth order distortion setting
    if(settings.Get(&this->fix_sixth_order_distortion_).hasErrors())
        return ret.AddError(CALIBRATION_PARAMETERS_SIXTH_ORDER_DISTORTION_MISSING);
    this->debug_.Msg("Calibration sixth order distortion = " + this->fix_sixth_order_distortion_.GetEntryValue());

    // Check for errors
    if(ret.hasErrors()) return ret;

    // Set flag that camera calibration is setup
    this->debug_.Msg("Camera calibration setup completed");
    this->is_setup_ = true;

    return ret;
}

/** @brief      Generates the calibration chessboard board
 *
 *  Uses the parameters from the \ref dlp::Calibration::Camera::Setup() to generate
 *  a \ref dlp::Image object which contains a calibration board.
 *
 *  @param[out] calibration_pattern     dlp::Image object pointer to store calibration image in
 *  @retval     CALIBRATION_NOT_SETUP   Calibration has not been setup
 *  @retval     CALIBRATION_NULL_POINTER_CALIBRATION_IMAGE  Input argument is NULL
 */
ReturnCode Calibration::Camera::GenerateCalibrationBoard( dlp::Image *calibration_pattern ) const{
    ReturnCode ret;

    this->debug_.Msg("Generating calibration board...");


    // Check that pointer is not empty
    if(!calibration_pattern)
        return ret.AddError(CALIBRATION_NULL_POINTER_CALIBRATION_IMAGE);

    // Check that the calibration object has been setup
    if(!this->isSetup())
        return ret.AddError(CALIBRATION_NOT_SETUP);

    // Calculate the size of the image in pixels
    unsigned int row_size_pixels    = this->board_row_distance_in_pixels_.Get();
    unsigned int column_size_pixels = this->board_column_distance_in_pixels_.Get();

    unsigned int rows_in_pattern    = this->board_rows_.Get()    + 1; // Add one since this board_rows_ is the number of feature point rows in calibration board
    unsigned int columns_in_pattern = this->board_columns_.Get() + 1; // Add one since this board_columns_ is the number of feature point rows in calibration board

    unsigned int total_chessboard_pixel_rows      = (row_size_pixels    * rows_in_pattern) ;
    unsigned int total_chessboard_pixel_columns   = (column_size_pixels * columns_in_pattern);

    // Add the pixel border to the total pixel count
    unsigned int total_rows      = total_chessboard_pixel_rows    + (2 * this->board_row_offset_pixels_.Get());
    unsigned int total_columns   = total_chessboard_pixel_columns + (2 * this->board_column_offset_pixels_.Get());

    // Clear the image
    calibration_pattern->Clear();

    // Create the image
    calibration_pattern->Create(total_columns,total_rows,dlp::Image::Format::RGB_UCHAR);

    // Fill it with foreground color
    calibration_pattern->FillImage(this->board_color_foreground_.Get());

    unsigned int black_chess_square_col  = 0;
    unsigned int black_chess_square_row  = 0;
    bool         black_chess_square_draw = true;

    unsigned int last_chessboard_pixel_row    = total_chessboard_pixel_rows    + this->board_row_offset_pixels_.Get();
    unsigned int last_chessboard_pixel_column = total_chessboard_pixel_columns + this->board_column_offset_pixels_.Get();

    // Create the pattern (only draw within the border)
    // Start just after the border and end before the border starts again
    for(     unsigned int yRow = this->board_row_offset_pixels_.Get(); yRow < last_chessboard_pixel_row;    yRow++){

        // Draw a line in the image
        for( unsigned int xCol = this->board_column_offset_pixels_.Get(); xCol < last_chessboard_pixel_column; xCol++){

            // Draw black pixel if within correct square
            if(black_chess_square_draw){
                calibration_pattern->SetPixel(xCol,yRow,this->board_color_background_.Get());
            }

            // Increment the black_chess_sqaure_col counter
            black_chess_square_col++;

            // If full square been drawn reset counters
            if(black_chess_square_col == column_size_pixels){
                black_chess_square_draw = !black_chess_square_draw;
                black_chess_square_col  = 0;
            }
        }

        // If the number of chessboard columns is odd,
        // invert the black_chess_square_col_draw
        // (i.e. first column is black and last column is white)
        if(columns_in_pattern % 2 == 1){
            black_chess_square_draw = !black_chess_square_draw;
        }

        // Increment the black_chess_sqaure_row counter
        black_chess_square_row++;

        // If full square been drawn reset counters
        if(black_chess_square_row == row_size_pixels){
            black_chess_square_draw = !black_chess_square_draw;
            black_chess_square_row  = 0;
        }
    }

    this->debug_.Msg("Calibration board generated");

    return ret;
}




/** @brief Analyzes the supplied calibration board image for feature points
 *  @param[in]  calib_image     dlp::Image object to analyze for calibration board feature points
 *  @param[out] success         Method returns true if the calibration board feature points were found
 *  @retval     CALIBRATION_NOT_SETUP                       Calibration has not been setup
 *  @retval     CALIBRATION_NULL_POINTER_SUCCESS            Pointer argument is NULL
 *  @retval     CALIBRATION_IMAGE_EMPTY                     The supplied dlp::Image object is empty
 *  @retval     CALIBRATION_IMAGE_RESOLUTION_INVALID        Supplied image resolution does not match the resolution saved during \ref Setup()
 *  @retval     CALIBRATION_IMAGE_CONVERT_TO_MONO_FAILED    Converting to supplied image to monochrome failed
 *  @retval     CALIBRATION_BOARD_NOT_DETECTED              Method did NOT find the calibration board feature points
 */
ReturnCode Calibration::Camera::AddCalibrationBoard(const dlp::Image &calib_image, bool *success){
    ReturnCode ret;

    this->debug_.Msg("Adding calibration board...");

    // Check that the calibration object has been setup
    if(!this->isSetup())
        return ret.AddError(CALIBRATION_NOT_SETUP);

    // Check that the pointer is not NULL
    if(!success) return ret.AddError(CALIBRATION_NULL_POINTER_SUCCESS);

    // Check if the image is empty
    if(calib_image.isEmpty())
        return ret.AddError(CALIBRATION_IMAGE_EMPTY);

    // Check the image resolution
    this->debug_.Msg("Checking calibration image resolution...");
    unsigned int rows = 0;
    unsigned int cols = 0;
    calib_image.GetRows(&rows);
    calib_image.GetColumns(&cols);

    if((rows != this->image_rows_.Get()) ||
       (cols != this->image_columns_.Get())){
        return ret.AddError(CALIBRATION_IMAGE_RESOLUTION_INVALID);
    }

    // Copy the image
    this->debug_.Msg("Copying calibration image...");
    dlp::Image temp_calibration_image;
    temp_calibration_image.Create(calib_image);

    // If the image is RGB convert it to greyscale
    this->debug_.Msg("Converting calibration image to monochrome...");

    if(temp_calibration_image.ConvertToMonochrome().hasErrors())
        return ret.AddError(CALIBRATION_IMAGE_CONVERT_TO_MONO_FAILED);

    // Convert dlp::Image to cv::Mat
    cv::Mat calibration_image_cv;
    temp_calibration_image.GetOpenCVData(&calibration_image_cv);

    // Look for the chessboard (checkerboard corners
    this->debug_.Msg("Looking for chessboard corners in calibration image...");
    std::vector<cv::Point2f> board_feature_locations_xy;
    cv::Size board_feature_size(this->board_columns_.Get(),this->board_rows_.Get());
    if (cv::findChessboardCorners(calibration_image_cv,
                                  board_feature_size,
                                  board_feature_locations_xy,
                                  CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS )){

        this->debug_.Msg("Chessboard corners found");
        this->debug_.Msg("Refining corner locations...");

        // Board was found. Refine the corner positions
        // NOTE : Many of these arguments would be good parameter settings for calibration
        cv::cornerSubPix(calibration_image_cv,
                         board_feature_locations_xy,
                         cv::Size(11, 11),
                         cv::Size(-1, -1),
                         cv::TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));

        // Return success as true and increment counter
        (*success) = true;
        this->board_number_successes_++;

        // Add this calibration image's corners to the list
        this->image_points_xy_.push_back(board_feature_locations_xy);
        this->object_points_xyz_.push_back(this->calibration_board_feature_points_xyz_);

        this->debug_.Msg("Calibration image added");
    }
    else{
        this->debug_.Msg("No chessboard corners found in calibration image");
        (*success) = false;
        ret.AddError(CALIBRATION_BOARD_NOT_DETECTED);
    }

    // Release the cv image
    calibration_image_cv.release();

    return ret;
}

/** @brief  Removes to most recently added calibration board feature points
 *  \note   If a Capture::Sequence or image file list was last added only the
 *          last image from the sequence or list will be removed.
 *  @retval     CALIBRATION_NO_BOARDS_ADDED     No previous feature points exist
 */
ReturnCode Calibration::Camera::RemoveLastCalibrationBoard(){
    ReturnCode ret;

    this->debug_.Msg("Removing the most recently added calibration board feature points...");

    if(this->board_number_successes_ > 0){
        // Decrement the number of successfull calibration boards added
        this->board_number_successes_--;

        // Remove the most recently added image points and object points
        this->object_points_xyz_.pop_back();
        this->image_points_xy_.pop_back();
    }
    else{
        ret.AddError(CALIBRATION_NO_BOARDS_ADDED);
    }

    return ret;
}



/** @brief Calibrates the camera using OpenCV
 *  \note Reference http://docs.opencv.org/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html
 *        for more information about camera calibration using OpenCV.
 *  \note Experimental results show that reprojection errors under 1.0 are acceptable
 *  @param[out] reprojection_error          Returns the reprojection error from the calibration routine. This number should be as close to zero as possible.
 *  @retval     CALIBRATION_NOT_SETUP                           Calibration has not been setup
 *  @retval     CALIBRATION_NOT_COMPLETE                        Calibration has not been completed
 *  @retval     CALIBRATION_NULL_POINTER_REPROJECTION_ERROR     Input argument is NULL
 */
ReturnCode Calibration::Camera::Calibrate(double *reprojection_error){
    return this->Calibrate(reprojection_error,true,true,true);
}

/** @brief Calibrates the camera using OpenCV
 *  \note Reference http://docs.opencv.org/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html
 *        for more information about camera calibration using OpenCV.
 *  \note Experimental results show that reprojection errors under 1.0 are acceptable
 *  @param[out] reprojection_error      Returns the reprojection error from the calibration routine. This number should be as close to zero as possible.
 *  @param[in]  update_intrinsic        If true this method updates the stored intrinsic parameters of the camera
 *  @param[in]  update_distortion       If true this method updates the stored distortion coefficients of the camera
 *  @param[in]  update_extrinsic        If true this method updates the stored extrinsic parameters of the camera
 *  @retval     CALIBRATION_NOT_SETUP                           Calibration has not been setup
 *  @retval     CALIBRATION_NOT_COMPLETE                        Calibration has not been completed
 *  @retval     CALIBRATION_NULL_POINTER_REPROJECTION_ERROR     Input argument is NULL
 */
ReturnCode Calibration::Camera::Calibrate(double *reprojection_error,
                                         const bool &update_intrinsic,
                                         const bool &update_distortion,
                                         const bool &update_extrinsic){
    ReturnCode ret;

    this->debug_.Msg("Calibrating camera...");

    // Check that calibration has been setup
    if(!this->isSetup())
        return ret.AddError(CALIBRATION_NOT_SETUP);

    // Check that enough boards have been added
    if(this->board_number_successes_ < this->board_number_required_.Get())
        return ret.AddError(CALIBRATION_NOT_COMPLETE);

    if(!reprojection_error) return ret.AddError(CALIBRATION_NULL_POINTER_REPROJECTION_ERROR);

    // Create calibration flags
    int cv_calibration_flags = 0;

    if(this->zero_tangent_distortion_.Get()){
        this->debug_.Msg("Set tangent distortion to zero");
        cv_calibration_flags += CV_CALIB_ZERO_TANGENT_DIST;
    }
    if(this->fix_sixth_order_distortion_.Get()){
        this->debug_.Msg("Fix sixth order radial distortion coeffecient");
        cv_calibration_flags += CV_CALIB_FIX_K3;
    }

    // Determine which calibration data should be updated
    cv::Mat intrinsic;
    cv::Mat distortion;
    cv::Mat extrinsic;
    cv::Mat homography;

    if(update_intrinsic){
        this->debug_.Msg("Update stored intrinsic calibration data");
        intrinsic = this->calibration_data_.intrinsic_;     // creates link to calibration objects data
    }
    else{
        intrinsic.create(3,3,CV_64FC1); //CV_INTRINSIC_SETUP
        intrinsic.setTo(cv::Scalar(0));
    }

    if(update_distortion){
        this->debug_.Msg("Update stored lens distortion calibration data");
        distortion = this->calibration_data_.distortion_;   // creates link to calibration objects data
    }
    else{
        distortion.create(5,1,CV_64FC1);
        distortion.setTo(cv::Scalar(0));
    }

    if(update_extrinsic){
        this->debug_.Msg("Update stored extrinsic calibration data");
        extrinsic  = this->calibration_data_.extrinsic_;    // creates link to calibration objects data
    }
    else{
        extrinsic.create(  2, 3, CV_64FC1);
        extrinsic.setTo(cv::Scalar(0));
    }

    homography.create( 3, 3, CV_64FC1);
    homography.setTo(cv::Scalar(0));


    // Calibrate the camera
    double               reproj_error;
    std::vector<cv::Mat> rotation_vector;
    std::vector<cv::Mat> translation_vector;
    cv::Size calibration_model_size = cv::Size(this->calibration_data_.model_columns_,
                                               this->calibration_data_.model_rows_);

    // Perform calibration
    reproj_error = cv::calibrateCamera(this->object_points_xyz_,    // chessboard corners coordinate x, y, z = 0 in mm
                                       this->image_points_xy_,      // chessboard corners coordinate x, y in camera pixels
                                       calibration_model_size,
                                       intrinsic,                   // in pixels
                                       distortion,                  // in pixels
                                       rotation_vector,             // unitless direction angle of camera to calibration board
                                       translation_vector,          // translation vector x,y,z for each calibration board in mm
                                       cv_calibration_flags);


    // Set reprojectiorn_error return value
    this->debug_.Msg("Calibration reprojection error = " + dlp::Number::ToString(reproj_error));
    this->calibration_data_.reprojection_error_ = reproj_error;
    (*reprojection_error) = reproj_error;

    // Copy the rotation and translation vector to the extrinsic calibration data object.
    // Only copy the vectors from the first pattern board.
    cv::transpose(rotation_vector.at(0),    extrinsic.row(dlp::Calibration::Data::EXTRINSIC_ROW_ROTATION));
    cv::transpose(translation_vector.at(0), extrinsic.row(dlp::Calibration::Data::EXTRINSIC_ROW_TRANSLATION));

    std::stringstream msg;
    msg << "Camera calibration complete" <<
           "\n\nIntrinsic (pixels)  = \n"  << intrinsic  <<
           "\n\nDistortion(pixels)  = \n"  << distortion <<
           "\n\nExtrinsic (real)    = \n"  << extrinsic  << std::endl;
    this->debug_.Msg(msg);

    // Calculate the homography for each calibration image
    this->debug_.Msg("Calculating calibration board homographies...");
    for(unsigned int iBoard=0; iBoard<this->image_points_xy_.size();iBoard++){

        // Undistort the image points using the new calibration information
        cv::Mat image_points_xy_distorted = cv::Mat(this->image_points_xy_.at(iBoard));
        cv::Mat image_points_xy_undistorted( this->board_columns_.Get() * this->board_rows_.Get(), 1, CV_32FC2);

        cv::undistortPoints(image_points_xy_distorted,       // chessboard corners coordinate x, y in camera pixels
                            image_points_xy_undistorted,     // returned normalized undistorted chessboard corner coordinates in x, y (unitless)
                            this->calibration_data_.intrinsic_,     // pixels  // always use the calibration objects data even is update was not selected for undistortion
                            this->calibration_data_.distortion_);   // pixels  // always use the calibration objects data even is update was not selected for undistortion

        // Convert the 3d calibration board points to a 2d point (ignore z)
        std::vector<cv::Point2f> calibration_board_feature_points_xy;
        for(unsigned int j = 0; j < this->calibration_board_feature_points_xyz_.size();j++){
            cv::Point2f calibration_board_feature_point_xy;

            // Remove z component
            calibration_board_feature_point_xy.x = this->calibration_board_feature_points_xyz_.at(j).x;
            calibration_board_feature_point_xy.y = this->calibration_board_feature_points_xyz_.at(j).y;

            // Save 2 dimensional point
            calibration_board_feature_points_xy.push_back(calibration_board_feature_point_xy);
        }

        // Clear the homography matrix
        homography.create(DLP_CV_HOMOGRAPHY_SETUP);
        homography.setTo(cv::Scalar(0));

        // Find the homography to convert the undistorted image's calibration board feature point
        // locations in pixels to the actual feature point's locaiton in space
        homography = cv::findHomography(image_points_xy_undistorted,                 // pixels
                                        calibration_board_feature_points_xy);   // x,y position (z=0 for our calibration image so it is not needed here)

        // Update the extrinsic data
        if(update_extrinsic){
            this->calibration_data_.homography_.push_back(homography.clone());
        }
        homography.release();
    }

    this->debug_.Msg("Calibration board homographies calculated");

    // Update calibration data complete
    this->calibration_data_.calibration_complete_  = true;
    this->calibration_data_.calibration_of_camera_ = true;

    return ret;
}

}
