/** @file   calibration_projector.cpp
 *  @brief  Contains calibration routines for a projector, by assuming it is an inverse camera.
 *  @copyright 2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

// DLP Structured Light SDK header files
#include <common/debug.hpp>                     // Adds dlp::Debug
#include <common/returncode.hpp>                // Adds dlp::ReturnCode
#include <common/image/image.hpp>               // Adds dlp::Image
#include <common/parameters.hpp>                // Adds dlp::Parameter
#include <dlp_platforms/dlp_platform.hpp>       // Adds dlp::DLP_Platform
#include <calibration/calibration.hpp>          // Adds dlp::Calibration::Projector and dlp::Calibration::Data

// OpenCV header files
#include <opencv2/opencv.hpp>                   // Adds OpenCV image container
#include <opencv2/highgui/highgui.hpp>          // Adds OpenCV camera calibration routines

// C++ standard header files
#include <vector>                               // Adds std:vector
#include <string>                               // Adds std::string

/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{


/** @brief  Constructs empty object */
Calibration::Projector::Projector(){
    this->debug_.SetName("CALIBRATION_PROJECTOR_DEBUG(" + dlp::Number::ToString(this)+ "): ");
    this->debug_.Msg("Constructing object...");
    this->ClearAll();
    this->debug_.Msg("Object constructed");
}

/** @brief  Destroys object and releases all memory allocated */
Calibration::Projector::~Projector(){
    this->debug_.Msg("Deconstructing object...");
    this->ClearAll();
    this->debug_.Msg("Object deconstructed");
}

/** @brief     Resets all settings and sets all calibration data to zero
 *  \note      Memory is not deallocated.
 */
void Calibration::Projector::ClearAll(){
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
    this->fix_sixth_order_distortion_.Set(false);
    this->is_setup_      = false;
    this->camera_set_    = false;
    this->projecter_set_ = false;
    this->projector_mirror_type_ = DLP_Platform::Mirror::ORTHOGONAL;
    this->calibration_board_feature_points_xyz_.clear();

    // Clear all points
    this->ClearCalibrationImagePoints();

    // Clear the calibration data
    this->ClearCalibrationData();

    this->debug_.Msg("All calibration settings and data cleared");
}

/** @brief     Sets all calibration data to zero
 *  \note      Memory is NOT deallocated.
 */
void Calibration::Projector::ClearCalibrationData(){
    this->debug_.Msg("Clearing calibration data...");

    this->calibration_data_.Clear();
    this->camera_calibration_data_.Clear();

    this->debug_.Msg("Calibration data cleared");
}



/** @brief      Retrieves \ref dlp::DLP_Platform resolution
 *  \note       If called before \ref Setup(), \ref dlp::Calibration::Camera::model_columns_
 *              and \ref dlp::Calibration::Camera::model_rows_ does NOT need to be
 *              included in the dlp::Parameters object
 *  @param[in]  platform                \ref dlp::DLP_Platform object to retrieve reoslution from
 *  @retval     DLP_PLATFORM_NOT_SETUP  \ref dlp::DLP_Platform has not been setup
 */
ReturnCode Calibration::Projector::SetDlpPlatform( const dlp::DLP_Platform &platform ){
    ReturnCode ret;

    this->debug_.Msg("Retrieving DLP Platform resolution...");

    // Check that DLP_Platform is setup
    if(!platform.isPlatformSetup())
        return ret.AddError(DLP_PLATFORM_NOT_SETUP);


    // Grab the resolution from DMD to set the model and image settings
    unsigned int rows;
    unsigned int columns;

    platform.GetRows(&rows);
    platform.GetColumns(&columns);

    // Save these values to the model and image settings
    this->model_rows_.Set(rows);
    this->model_columns_.Set(columns);
    this->projecter_set_ = true;

    this->debug_.Msg("Projector resolution  = " +
                     this->model_columns_.GetEntryValue() +
                     " by " +
                     this->model_rows_.GetEntryValue());

    // Get the DMD mirror orientation
    platform.GetMirrorType(&this->projector_mirror_type_ );

    if(this->projector_mirror_type_ == DLP_Platform::Mirror::DIAMOND){
        this->debug_.Msg("Projector mirror orientation = DIAMOND" );
    }
    else if(this->projector_mirror_type_ == DLP_Platform::Mirror::ORTHOGONAL){
        this->debug_.Msg("Projector mirror orientation = ORTHOGONAL" );
    }

    // Get the effective mirror size
    float mirror_size;
    platform.GetEffectiveMirrorSize(&mirror_size);
    this->pixel_size_.Set(mirror_size);


    return ret;
}

/** @brief  Adds camera calibration data so that the projected calibration board feature
 *          points can be converted to real space coordinates
 *  \note   Camera calibration must be added before calling \ref Calibrate()
 *  @retval CALIBRATION_NOT_SETUP           The projector calibration has NOT been setup
 *  @retval CALIBRATION_NOT_FROM_CAMERA     The supplied calibration data is NOT from a camera
 *  @retval CALIBRATION_NOT_COMPLETE        The supplied calibration data is NOT complete
 */
ReturnCode Calibration::Projector::SetCameraCalibration(const dlp::Calibration::Data &camera_calibration){
    ReturnCode ret;

    this->debug_.Msg("Retrieving camera calibration data...");

    // Check that calibration has been setup
    if(!this->isSetup()) ret.AddError(CALIBRATION_NOT_SETUP);

    // Check that calibration data argument is from a camera
    if(!camera_calibration.isCamera()) ret.AddError(CALIBRATION_NOT_FROM_CAMERA);

    // Check that camera calibration has been completed
    if(!camera_calibration.isComplete()) ret.AddError(CALIBRATION_NOT_COMPLETE);

    // Check for errors
    if(ret.hasErrors()) return ret;

    // Copy the camera calibration data
    this->camera_calibration_data_ = camera_calibration;

    this->debug_.Msg("Camera calibration data retrieved");

    return ret;
}


/** @brief      Sets all required parameters for projector calibration
 *  \note       The calibration board feature
 *              \ref Calibration::Camera::board_column_distance_ and
 *              \ref Calibration::Camera::board_row_distance_ are
 *              calculated from the model resolution and
 *              \ref Calibration::Camera::board_rows_ and
 *              \ref Calibration::Camera::board_columns_
 *  @param[in]  settings    \ref dlp::Parameters object to retrieve settings from
 *  @retval     PARAMETERS_EMPTY                                    Supplied dlp::Parameters object is empty
 *  @retval     CALIBRATION_PARAMETERS_MODEL_SIZE_MISSING             The model resolution settings are missing (\ref Calibration::Camera::model_rows_ and/or \ref Calibration::Camera::model_columns_)
 *  @retval     CALIBRATION_PARAMETERS_IMAGE_SIZE_MISSING             The image resolution settings are missing (\ref Calibration::Camera::image_rows_ and/or \ref Calibration::Camera::image_columns_)
 *  @retval     CALIBRATION_PARAMETERS_NUMBER_BOARDS_MISSING          The number of calibration board images is missing (\ref Calibration::Camera::board_number_required_)
 *  @retval     CALIBRATION_PARAMETERS_BOARD_FEATURE_SIZE_MISSING     The board feature resolution settings are missing (\ref Calibration::Camera::board_rows_ and/or \ref Calibration::Camera::board_columns_))
 *  @retval     CALIBRATION_PARAMETERS_TANGENT_DISTORTION_MISSING     The tangent distortion setting is missing (\ref Calibration::Camera::zero_tangent_distortion_)
 *  @retval     CALIBRATION_PARAMETERS_SIXTH_ORDER_DISTORTION_MISSING The sixth order distortion setting is missing (\ref Calibration::Camera::fix_sixth_order_distortion_)
 */
ReturnCode Calibration::Projector::Setup(const dlp::Parameters &settings){
    ReturnCode ret;

    this->debug_.Msg("Setting up projector calibration object...");

    // Check that settings is not empty
    if(settings.isEmpty()){
        this->debug_.Msg("Setting list is EMPTY!");
        return ret.AddError(PARAMETERS_EMPTY);
    }

    // Reset flags
    this->is_setup_ = false;
    this->board_number_successes_ = 0;

    // If the camera has not been set get the image resolutions
    if(!this->camera_set_){
        if(settings.Get(&this->image_rows_).hasErrors()){
            this->debug_.Msg("Calibration image rows MISSING!");
            ret.AddError(CALIBRATION_PARAMETERS_IMAGE_SIZE_MISSING);
        }

        if(settings.Get(&this->image_columns_).hasErrors()){
            this->debug_.Msg("Calibration image columns MISSING!");
            ret.AddError(CALIBRATION_PARAMETERS_IMAGE_SIZE_MISSING);
        }
    }

    // If a projector has not been set get the projector resolution
    if(!this->projecter_set_){
        this->debug_.Msg("Camera was not set, retrieving model and image resolution...");

        if(settings.Get(&this->model_rows_).hasErrors()){
            this->debug_.Msg("Calibration model rows MISSING!");
            ret.AddError(CALIBRATION_PARAMETERS_MODEL_SIZE_MISSING);
        }

        if(settings.Get(&this->model_columns_).hasErrors()){
            this->debug_.Msg("Calibration model columns MISSING!");
            ret.AddError(CALIBRATION_PARAMETERS_MODEL_SIZE_MISSING);
        }


        if(settings.Get(&this->pixel_size_).hasErrors()){
            this->debug_.Msg("Calibration model pixel size MISSING!");
            ret.AddError(CALIBRATION_PARAMETERS_MODEL_SIZE_MISSING);
        }
    }

    if(settings.Get(&this->focal_length_).hasErrors()){
        this->debug_.Msg("Calibration model focal length MISSING!");
        ret.AddError(CALIBRATION_PARAMETERS_MODEL_SIZE_MISSING);
    }

    this->debug_.Msg("Image resolution = " +
                     this->image_columns_.GetEntryValue() +
                     " by " +
                     this->image_rows_.GetEntryValue());

    this->debug_.Msg("Model resolution = " +
                     this->model_columns_.GetEntryValue() +
                     " by " +
                     this->model_rows_.GetEntryValue());

    // Save model and image resolution to calibration data
    this->debug_.Msg("Saving model and image resolution to calibration data...");
    this->calibration_data_.image_rows_     = this->image_rows_.Get();
    this->calibration_data_.image_columns_  = this->image_columns_.Get();
    this->calibration_data_.model_rows_     = this->model_rows_.Get();
    this->calibration_data_.model_columns_  = this->model_columns_.Get();

    // Save the effective model size
    this->effective_model_height_ = this->model_rows_.Get();
    this->effective_model_width_  = this->model_columns_.Get();



    // Get pixel size and focal lenght
    this->effective_pixel_size_um_ = this->pixel_size_.Get();
    this->estimated_focal_length_mm_ = this->focal_length_.Get();


    // Get the vertical and horizontal offset percentages
    settings.Get(&this->offset_vertical_);
    settings.Get(&this->offset_horizontal_);

    this->debug_.Msg("Model vertical offset percent   = " + this->offset_vertical_.GetEntryValue());
    this->debug_.Msg("Model horizontal offset percent = " + this->offset_horizontal_.GetEntryValue());


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

    // Get the calibraiton board feature number of columns
    if(settings.Get(&this->board_columns_).hasErrors())
        ret.AddError(CALIBRATION_PARAMETERS_BOARD_FEATURE_SIZE_MISSING);
    this->debug_.Msg("Calibration board column features = " + this->board_columns_.GetEntryValue());

    // Calculate the board_column_distance in pixels
    this->board_column_distance_.Set((double)(this->model_columns_.Get()/(this->board_columns_.Get() + 1)));
    this->debug_.Msg("Calibration board column feature distance = " + this->board_column_distance_.GetEntryValue());

    // board_column_distance_in_pixels_ equal to distance because distance is already in pixels
    this->board_column_distance_in_pixels_.Set((unsigned int)this->board_column_distance_.Get());
    this->debug_.Msg("Calibration board column feature distance in pixels = " + this->board_column_distance_in_pixels_.GetEntryValue());

    // Get the calibration board feature number of rows
    if(settings.Get(&this->board_rows_).hasErrors())
        ret.AddError(CALIBRATION_PARAMETERS_BOARD_FEATURE_SIZE_MISSING);
    this->debug_.Msg("Calibration board row features = " + this->board_rows_.GetEntryValue());

    // Calculate the board row distance, this is dependent of DMD mirror orientation!
    // First assume the mirrors are orthogonal so that distance_in_pixels can be set
    // equal to the distance because it is currently in pixels
    this->board_row_distance_.Set((double)(this->model_rows_.Get()/(this->board_rows_.Get() + 1)));
    this->board_row_distance_in_pixels_.Set((unsigned int)this->board_row_distance_.Get());


    // If the projector mirror type is DIAMOND correct for the row shifting
    if(this->projector_mirror_type_ == DLP_Platform::Mirror::DIAMOND){
        // Compensate for the shifted mirrors
        double corrected_row_distance = this->board_row_distance_.Get() * 0.5;
        this->board_row_distance_.Set(corrected_row_distance);

        // Adjust effective model height
        this->effective_model_height_ = this->effective_model_height_ * 0.5;
    }

    this->debug_.Msg("Calibration board row feature distance = " + this->board_row_distance_.GetEntryValue());
    this->debug_.Msg("Calibration board row feature distance in pixels = " + this->board_row_distance_in_pixels_.GetEntryValue());

    // Get the calibration board color information (not required)
    settings.Get(&this->board_color_foreground_);
    settings.Get(&this->board_color_background_);
    this->debug_.Msg("Calibration foreground color = " + this->board_color_foreground_.GetEntryValue());
    this->debug_.Msg("Calibration background color = " + this->board_color_background_.GetEntryValue());


    // Create calibration_board_feature_points_xyz_ in real x, y, z space
    // Assume that calibration board is both planar and at z = 0
    this->debug_.Msg("Generating projector calibration board points in real space (x,y,z)...");

    // Clear pattern points in case it has already been setup
    this->calibration_board_feature_points_xyz_.clear();

    // Assume that the first feature is located NOT at 0,0 because the chessboard
    // must be constructed on the DMD and the first feature must account for a border
    // equal to the distance between feature points both vertically and horizontally
    for(     unsigned int iRow = 1; iRow <= this->board_rows_.Get();    iRow++ ){
        for( unsigned int iCol = 1; iCol <= this->board_columns_.Get();  iCol++ ){

            // Calculate the feature point locations
            float x_pos = (iCol * this->board_column_distance_.Get());
            float y_pos = (iRow * this->board_row_distance_.Get());

            // If the DMD mirrors have a DIAMOND orientation and the y_pos is on a row
            // not divisible by two account for the x shift
            if((this->projector_mirror_type_ == DLP_Platform::Mirror::DIAMOND) &&
              (( (unsigned int)y_pos) % 2 == 1)){
                x_pos = x_pos + 0.5;
            }

            cv::Point3f feature_point = cv::Point3f(x_pos,y_pos,0.0f);
            this->calibration_board_feature_points_xyz_.push_back(feature_point);
        }
    }


    // Check for tangent distortion setting
    if(settings.Get(&this->zero_tangent_distortion_).hasErrors())
        ret.AddError(CALIBRATION_PARAMETERS_TANGENT_DISTORTION_MISSING);
    this->debug_.Msg("Calibration tangent distortion = " + this->zero_tangent_distortion_.GetEntryValue());

    // Check for sixth order distortion setting
    if(settings.Get(&this->fix_sixth_order_distortion_).hasErrors())
        ret.AddError(CALIBRATION_PARAMETERS_SIXTH_ORDER_DISTORTION_MISSING);
    this->debug_.Msg("Calibration sixth order distortion = " + this->fix_sixth_order_distortion_.GetEntryValue());


    if(settings.Get(&this->fix_aspect_ratio_).hasErrors()){
        this->debug_.Msg("Calibration model fix aspect ratio flag MISSING!");
        ret.AddError(CALIBRATION_PARAMETERS_FIX_ASPECT_RATIO_MISSING);
    }
    this->debug_.Msg("Calibration fix aspect ratio = " + this->fix_aspect_ratio_.GetEntryValue());


    // Check for errors
    if(ret.hasErrors()) return ret;

    // Set flag that camera calibration is setup
    this->debug_.Msg("Projector calibration setup completed");
    this->is_setup_ = true;

    return ret;
}

/** @brief      Generates the calibration chessboard board
 *
 *  Uses \ref Calibration::Camera::board_rows_, \ref Calibration::Camera::board_columns_,
 *  \ref Calibration::Camera::board_row_distance_in_pixels_, and
 *  \ref Calibration::Camera::board_column_distance_in_pixels_
 *  to determine the returned image's resolution.
 *
 *  @param[out] calibration_pattern     dlp::Image object pointer to store calibration image in
 *  @retval     CALIBRATION_NOT_SETUP   Calibration has not been setup
 *  @retval     CALIBRATION_NULL_POINTER_CALIBRATION_IMAGE  Input argument is NULL
 */
ReturnCode Calibration::Projector::GenerateCalibrationBoard( dlp::Image *calibration_pattern ) const{
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

    // Check that total chessboard size is not larger than the resolution
    if(total_chessboard_pixel_columns > this->model_columns_.Get())
        return ret.AddError(CALIBRATION_IMAGE_RESOLUTION_INVALID);

    if(total_chessboard_pixel_rows > this->model_rows_.Get())
        return ret.AddError(CALIBRATION_IMAGE_RESOLUTION_INVALID);

    // Clear the image
    calibration_pattern->Clear();

    // Create the image
    calibration_pattern->Create(this->model_columns_.Get(),this->model_rows_.Get(),dlp::Image::Format::RGB_UCHAR);

    // Fill it with foreground color
    calibration_pattern->FillImage(this->board_color_foreground_.Get());

    unsigned int black_chess_square_col  = 0;
    unsigned int black_chess_square_row  = 0;
    bool         black_chess_square_draw = true;

    // Create the pattern (only draw within the border)
    // Start just after the border and end before the border starts again
    for(     unsigned int yRow = 0; yRow < total_chessboard_pixel_rows;    yRow++){

        // Draw a line in the image
        for( unsigned int xCol = 0; xCol < total_chessboard_pixel_columns; xCol++){

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



/** @brief  Separates the projected calibration board from the printed calibration board
 *          image and analyzes the projected calibration board for the feature points
 *  @param[in]  projector_all_on                  \ref dlp::Image object of when projector is projection full-on white pattern
 *  @param[in]  projector_all_off                 \ref dlp::Image object of when projector is projecting black/blank pattern
 *  @param[in]  board_image_printed_and_projected   \ref dlp::Image object of the projected calibration board on top of the printed calibration board
 *  @param[out] board_image_projected               Returned \ref dlp::Image object of the projected calibration board and is analyzed for calibration board feature points
 *  @param[out] success                             Method returns true if the calibration board feature points were found
 *  @retval     CALIBRATION_NOT_SETUP                       Calibration has not been setup
 *  @retval     CALIBRATION_PRINTED_IMAGE_EMPTY             Supplied printed calibriaton board image is empty
 *  @retval     CALIBRATION_COMBO_IMAGE_EMPTY               Supplied printed and projected (combination) calibration board image is empty
 *  @retval     CALIBRATION_NULL_POINTER_SUCCESS            Pointer argument is NULL
 *  @retval     CALIBRATION_NULL_POINTER_PROJECTED_BOARD    Pointer argument is NULL
 *  @retval     CALIBRATION_IMAGE_RESOLUTION_MISMATCH       The supplied printed and combo calibration images do NOT have the same resolution
 *  @retval     CALIBRATION_IMAGE_RESOLUTION_INVALID        Supplied image resolution does not match the resolution saved during \ref Setup()
 *  @retval     CALIBRATION_IMAGE_CONVERT_TO_MONO_FAILED    Converting one or both of the supplied images to monochrome failed
 *  @retval     CALIBRATION_BOARD_NOT_DETECTED              Method did NOT find the calibration board feature points
 */
ReturnCode Calibration::Projector::RemovePrinted_AddProjectedBoard(const Image &projector_all_on,
                                                                   const Image &projector_all_off,
                                                                   const Image &board_image_printed_and_projected,
                                                                         Image *board_image_projected,
                                                                          bool *success){

    ReturnCode ret;

    // Check that calibration has been setup
    if(!this->isSetup()) return ret.AddError(CALIBRATION_NOT_SETUP);

    // Check that the images are not empty
    if(projector_all_on.isEmpty())
        ret.AddError(CALIBRATION_PRINTED_IMAGE_EMPTY);
    if(projector_all_off.isEmpty())
        ret.AddError(CALIBRATION_PRINTED_IMAGE_EMPTY);
    if(board_image_printed_and_projected.isEmpty())
        ret.AddError(CALIBRATION_COMBO_IMAGE_EMPTY);

    // Check pointers
    if(!board_image_projected) ret.AddError(CALIBRATION_NULL_POINTER_PROJECTED_BOARD);
    if(!success)               ret.AddError(CALIBRATION_NULL_POINTER_SUCCESS);

    // Check for errors
    if(ret.hasErrors()) return ret;

    // Check that the combo and printed image have the same resolution
    unsigned int rows_all_on;
    unsigned int cols_all_on;
    unsigned int rows_all_off;
    unsigned int cols_all_off;
    unsigned int rows_combo;
    unsigned int columns_combo;


    projector_all_on.GetRows(&rows_all_on);
    projector_all_on.GetColumns(&cols_all_on);

    projector_all_off.GetRows(&rows_all_off);
    projector_all_off.GetColumns(&cols_all_off);

    board_image_printed_and_projected.GetRows(&rows_combo);
    board_image_printed_and_projected.GetColumns(&columns_combo);


    if( (rows_combo    != rows_all_on)  ||
        (columns_combo != cols_all_on)  ||
        (rows_combo    != rows_all_off) ||
        (columns_combo != cols_all_off))
        return ret.AddError(CALIBRATION_IMAGE_RESOLUTION_MISMATCH);

    // Check that these images are the same size specificed by the projector calibration board image resolution
    if( (rows_combo    != this->image_rows_.Get())   ||
        (columns_combo != this->image_columns_.Get()))
        return ret.AddError(CALIBRATION_IMAGE_RESOLUTION_INVALID);

    // Copy calibration images
    dlp::Image all_on(projector_all_on);
    dlp::Image all_off(projector_all_off);
    dlp::Image combo(board_image_printed_and_projected);

    // Convert the images to grayscale
    if(all_on.ConvertToMonochrome().hasErrors())
        return ret.AddError(CALIBRATION_IMAGE_CONVERT_TO_MONO_FAILED);
    if(all_off.ConvertToMonochrome().hasErrors())
        return ret.AddError(CALIBRATION_IMAGE_CONVERT_TO_MONO_FAILED);
    if(combo.ConvertToMonochrome().hasErrors())
        return ret.AddError(CALIBRATION_IMAGE_CONVERT_TO_MONO_FAILED);

    dlp::Image albedo(columns_combo, rows_combo,
                      dlp::Image::Format::MONO_UCHAR);

    ret = board_image_projected->Create(columns_combo, rows_combo,
                                        dlp::Image::Format::MONO_UCHAR);

    // Check for errors
    if(ret.hasErrors()) return ret;

    // Decode each pixel
    for(     unsigned int yRow = 0; yRow < rows_combo;    yRow++){
        for( unsigned int xCol = 0; xCol < columns_combo; xCol++){

            // Get the pixel from the all off and on images
            unsigned char val_all_off = 0;
            unsigned char val_all_on  = 0;
            unsigned char val_albedo  = 0;

            all_off.Unsafe_GetPixel( xCol, yRow, &val_all_off);
            all_on.Unsafe_GetPixel(  xCol, yRow, &val_all_on);

            // Save albedo threshold value
            val_albedo = (val_all_on + val_all_off) / 2;
            albedo.Unsafe_SetPixel( xCol, yRow, val_albedo );
        }
    }


    // Decode each pixel
    for(     unsigned int yRow = 0; yRow < rows_combo;    yRow++){
        for( unsigned int xCol = 0; xCol < columns_combo; xCol++){

            // Get the coded pixel value and the albedo threshold value
            unsigned char val_coded  = 0;
            unsigned char val_albedo = 0;

            combo.Unsafe_GetPixel( xCol, yRow, &val_coded );
            albedo.Unsafe_GetPixel( xCol, yRow, &val_albedo );

            // Check that the difference meets the threshold requirements for an ON pixel
            if((val_coded > val_albedo) ||
               (abs(val_coded - val_albedo) < 5)){  // If there isn't enough difference the pixel is set to white
                board_image_projected->Unsafe_SetPixel(xCol,yRow,(unsigned char)255);
            }
            else{
                board_image_projected->Unsafe_SetPixel(xCol,yRow,(unsigned char)0);
            }
        }
    }

    // Create a cv::Mat for the projected calibration image to be stored
    cv::Mat cv_projected;

    // Get the OpenCV data
    board_image_projected->GetOpenCVData(&cv_projected);

    // Look for minimum threshold value
    std::vector<cv::Point2f> board_feature_locations_xy;
    cv::Size board_feature_size(this->board_columns_.Get(),this->board_rows_.Get());

    // Look for the projected chessboard corners
    this->debug_.Msg("Looking for chessboard corners in calibration image...");
    if (cv::findChessboardCorners(cv_projected,
                                  board_feature_size,
                                  board_feature_locations_xy,
                                  CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS )){

        this->debug_.Msg("Chessboard corners found");
        this->debug_.Msg("Refining corner locations...");

        // Board was found. Refine the corner positions
        // NOTE : Many of these arguments would be good parameter settings for calibration
        cv::cornerSubPix(cv_projected,
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

    // Release cv::Mat objects
    all_on.Clear();
    all_off.Clear();
    combo.Clear();
    cv_projected.release();

    return ret;
}


/** @brief Calibrates the projector using OpenCV routines as an inverse camera. Updates all calibraiton data.
 *  \note Reference http://docs.opencv.org/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html
 *        for more information about camera calibration using OpenCV.
 *  \note Experimental results show that reprojection errors under 1.0 are acceptable
 *  @param[out] reprojection_error      Returns the reprojection error from the calibration routine. This number should be as close to zero as possible.
 *  @retval     CALIBRATION_NOT_SETUP                           Calibration has NOT been setup
 *  @retval     CALIBRATION_NOT_COMPLETE                        Calibration has NOT been completed
 *  @retval     CALIBRATION_NULL_POINTER_REPROJECTION_ERROR     Input argument is NULL
 *  @retval     CALIBRATION_CAMERA_CALIBRATION_MISSING          Camera calibraition has not been added using Calibration::Projector::SetCameraCalibration()
 */
ReturnCode Calibration::Projector::Calibrate(double *reprojection_error){
    return this->Calibrate(reprojection_error,true,true,true);
}

/** @brief Calibrates the projector using OpenCV routines as an inverse camera
 *  \note Reference http://docs.opencv.org/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html
 *        for more information about camera calibration using OpenCV.
 *  \note Experimental results show that reprojection errors under 1.0 are acceptable
 *  @param[out] reprojection_error      Returns the reprojection error from the calibration routine. This number should be as close to zero as possible.
 *  @param[in]  update_intrinsic        If true this method updates the stored intrinsic parameters of the camera
 *  @param[in]  update_distortion       If true this method updates the stored distortion coefficients of the camera
 *  @param[in]  update_extrinsic        If true this method updates the stored extrinsic parameters of the camera
 *  @retval     CALIBRATION_NOT_SETUP                           Calibration has NOT been setup
 *  @retval     CALIBRATION_NOT_COMPLETE                        Calibration has NOT been completed
 *  @retval     CALIBRATION_NULL_POINTER_REPROJECTION_ERROR     Input argument is NULL
 *  @retval     CALIBRATION_CAMERA_CALIBRATION_MISSING                  Camera calibraition has not been added using Calibration::Projector::SetCameraCalibration()
 *  @retval     CALIBRATION_CAMERA_CALIBRATION_HOMOGRAPHIES_MISSING     The supplied camera calibration does not have enough homography matrices to transform the observed projected feature points into real space
 */
ReturnCode Calibration::Projector::Calibrate(double    *reprojection_error,
                                            const bool &update_intrinsic,
                                            const bool &update_distortion,
                                            const bool &update_extrinsic){

    ReturnCode ret;

    this->debug_.Msg("Calibrating projector...");

    // Check that calibration has been setup
    if(!this->isSetup())
        return ret.AddError(CALIBRATION_NOT_SETUP);

    // Check that the camera calibration has been added
    if(!this->camera_calibration_data_.isComplete())
        return ret.AddError(CALIBRATION_CAMERA_CALIBRATION_MISSING);

    // Check that camera calibration contains enough homography matrices
    if(this->camera_calibration_data_.homography_.size() < this->board_number_required_.Get())
        return ret.AddError(CALIBRATION_CAMERA_CALIBRATION_HOMOGRAPHIES_MISSING);


    // Check that enough boards have been added
    if(this->board_number_successes_ < this->board_number_required_.Get())
        return ret.AddError(CALIBRATION_NOT_COMPLETE);

    // Create calibration flags
    int cv_calibration_flags = CV_CALIB_USE_INTRINSIC_GUESS;

    if(this->fix_aspect_ratio_.Get()){
        this->debug_.Msg("Fix aspect ratio");
        cv_calibration_flags += CV_CALIB_FIX_ASPECT_RATIO;
    }

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

    if(update_intrinsic){
        this->debug_.Msg("Update stored intrinsic calibration data");
        intrinsic = this->calibration_data_.intrinsic_;     // creates link to calibration objects data
    }
    else{
        intrinsic.create(DLP_CV_INTRINSIC_SETUP);
    }

    if(update_distortion){
        this->debug_.Msg("Update stored lens distortion calibration data");
        distortion = this->calibration_data_.distortion_;   // creates link to calibration objects data
    }
    else{
        distortion.create(DLP_CV_DISTORTION_SETUP);
    }

    if(update_extrinsic){
        this->debug_.Msg("Update stored extrinsic calibration data");
        extrinsic  = this->calibration_data_.extrinsic_;    // creates link to calibration objects data
    }
    else{
        extrinsic.create(DLP_CV_EXTRINSIC_SETUP);
    }

    // Clear the calibration parameters
    intrinsic.setTo(  cv::Scalar(0) );
    distortion.setTo( cv::Scalar(0) );
    extrinsic.setTo(  cv::Scalar(0) );

    // Convert focal length in mm to pixels
    float focal_length_pixels = this->estimated_focal_length_mm_/ (this->effective_pixel_size_um_/1000);
    float focal_point_x = (this->effective_model_width_  / 2) + ( (this->effective_model_width_  / 2) * (this->offset_horizontal_.Get() / 100));
    float focal_point_y = (this->effective_model_height_ / 2) + ( (this->effective_model_height_ / 2) * (this->offset_vertical_.Get()   / 100));

    if(focal_point_x <= 0) focal_point_x = 0;
    if(focal_point_y <= 0) focal_point_y = 0;

    if(focal_point_x >= this->effective_model_width_)  focal_point_x = this->effective_model_width_ - 1;
    if(focal_point_y >= this->effective_model_height_) focal_point_y = this->effective_model_height_ - 1;

    // Load guess values for intrinsic parameters
    intrinsic.setTo(cv::Scalar(0.0));
    intrinsic.at<double>(0,0) = (double) focal_length_pixels;
    intrinsic.at<double>(1,1) = (double) focal_length_pixels;
    intrinsic.at<double>(0,2) = (double) focal_point_x;
    intrinsic.at<double>(1,2) = (double) focal_point_y;
    intrinsic.at<double>(2,2) = (double) 1.0;

    // Convert the observed feature locations in the camera images to real x,y,z coordinates
    // using the homography matrices from the camera calibration
    std::vector<std::vector<cv::Point2f>> image_feature_points_xy;

    for(unsigned int iBoard = 0; iBoard < this->board_number_required_.Get(); iBoard++){
        cv::Mat image_points_xy_undistorted( this->board_columns_.Get() * this->board_rows_.Get(), 1, CV_32FC2);
        cv::Mat image_points_xy_transformed( this->board_columns_.Get() * this->board_rows_.Get(), 1, CV_32FC2);

        // Undistort the projectors image points with the camera calibration
        cv::undistortPoints(cv::Mat(this->image_points_xy_.at(iBoard)),        // camera pixels                     // source
                            image_points_xy_undistorted,                            // returns normalized camera pixels            // destination
                            this->camera_calibration_data_.intrinsic_.clone(),           // pixels
                            this->camera_calibration_data_.distortion_.clone());         // pixels

        // Transform the projectors undistorted image points with the camera's homography
        cv::perspectiveTransform(image_points_xy_undistorted,     // camera pixels
                                 image_points_xy_transformed,   // mm in space (assuming checkerboard is z = 0
                                 this->camera_calibration_data_.homography_.at(iBoard).clone()); // camera pixels

        // Convert cv::Mat to vector of vectors
        std::vector<cv::Point2f> image_feature_point_xy;
        for(int iRow = 0; iRow < image_points_xy_transformed.rows; iRow++){
            image_feature_point_xy.push_back(image_points_xy_transformed.at<cv::Point2f>(iRow));
        }

        image_feature_points_xy.push_back(image_feature_point_xy);

        // Release memory
        image_points_xy_undistorted.release();
        image_points_xy_transformed.release();
    }


    // Convert xyz to xy for object points and xy to xyz for image points
    std::vector<std::vector<cv::Point2f>> dmd_feature_points_xy;
    std::vector<std::vector<cv::Point3f>> projected_feature_points_xyz;

    // Clear the vectors
    dmd_feature_points_xy.clear();
    projected_feature_points_xyz.clear();

    for( unsigned int iBoard = 0; iBoard < this->board_number_required_.Get(); iBoard++){
        std::vector<cv::Point2f> dmd_feature_point_xy;
        std::vector<cv::Point3f> image_points_temp;
        for( unsigned int iPoint = 0; iPoint < image_feature_points_xy.at(iBoard).size();iPoint++){
            cv::Point2f dmd_feature_point_xy_temp;
            cv::Point3f projected_feature_point_xyz_temp;

            // Grab the x and y components
            dmd_feature_point_xy_temp.x = this->object_points_xyz_.at(iBoard).at(iPoint).x;    // dmd pixels (projected)
            dmd_feature_point_xy_temp.y = this->object_points_xyz_.at(iBoard).at(iPoint).y;    // dmd pixels (projected)

            // Add point to list
            dmd_feature_point_xy.push_back(dmd_feature_point_xy_temp);

            // Grab the x and y components from the image
            projected_feature_point_xyz_temp.x = image_feature_points_xy.at(iBoard).at(iPoint).x;         // dmd pixels (camera captured)
            projected_feature_point_xyz_temp.y = image_feature_points_xy.at(iBoard).at(iPoint).y;         // dmd pixels (camera captured)
            projected_feature_point_xyz_temp.z = 0;

            // Add point to list
            image_points_temp.push_back(projected_feature_point_xyz_temp);
        }

        // Add the points for the board
        dmd_feature_points_xy.push_back(dmd_feature_point_xy);
        projected_feature_points_xyz.push_back(image_points_temp);
    }


    // Find the projector's calibration information
    double               reproj_error;
    std::vector<cv::Mat> rotation_vector;
    std::vector<cv::Mat> translation_vector;
    cv::Size calibration_model_size = cv::Size(this->effective_model_width_,
                                               this->effective_model_height_);

    // Clear the vectors
    rotation_vector.clear();
    translation_vector.clear();


    // Perform the calibration
    reproj_error = cv::calibrateCamera(projected_feature_points_xyz,
                                       dmd_feature_points_xy,
                                       calibration_model_size,
                                       intrinsic,
                                       distortion,
                                       rotation_vector,
                                       translation_vector,
                                       cv_calibration_flags);

     (*reprojection_error) = reproj_error;
    this->calibration_data_.reprojection_error_ = reproj_error;

    // Copy the rotation and translation vector to the
    // extrinsic calibration data object
    // Only copys the vectors from the first pattern board
    cv::transpose(rotation_vector.at(0).clone(),    this->calibration_data_.extrinsic_.row(dlp::Calibration::Data::EXTRINSIC_ROW_ROTATION));
    cv::transpose(translation_vector.at(0).clone(), this->calibration_data_.extrinsic_.row(dlp::Calibration::Data::EXTRINSIC_ROW_TRANSLATION));

    // Display the calibration data
    std::stringstream msg;
    msg << "Projector calibration complete" <<
           "\n\nIntrinsic (pixels)  = \n" << intrinsic  <<
           "\n\nDistortion(pixels)  = \n" << distortion <<
           "\n\nExtrinsic (real)    = \n" << extrinsic  << std::endl;
    this->debug_.Msg(msg);

    // Homography is not needed for projector calibration

    // Update calibration data complete
    this->calibration_data_.calibration_complete_  = true;
    this->calibration_data_.calibration_of_camera_ = false;

    return ret;
}

}
