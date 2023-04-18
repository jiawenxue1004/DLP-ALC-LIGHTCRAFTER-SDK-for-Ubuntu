/** @file   calibration_data.cpp
 *  @brief  Contains methods for Calibration::Data class
 *  @copyright 2014 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

// DLP Structured Light SDK header files
#include <common/debug.hpp>                     // Adds dlp::Debug
#include <common/returncode.hpp>                // Adds dlp::ReturnCode
#include <common/image/image.hpp>               // Adds dlp::Image
#include <common/parameters.hpp>                // Adds dlp::Parameter
#include <calibration/calibration.hpp>          // Adds dlp::Calibration::Data

// OpenCV header files
#include <opencv2/opencv.hpp>                   // Adds OpenCV image container

// C++ standard header files
#include <vector>                               // Adds std:vector
#include <string>                               // Adds std::string

/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{

/** @brief  Constructs empty object and allocates memory for calibration data */
Calibration::Data::Data(){
    // Set the default values
    this->calibration_complete_  = false;
    this->calibration_of_camera_ = true;
    this->image_columns_         = 0;
    this->image_rows_            = 0;
    this->model_columns_         = 0;
    this->model_rows_            = 0;
    this->reprojection_error_    = 0.0;

    // Allocate memory for the calibration data
    this->intrinsic_.create(  DLP_CV_INTRINSIC_SETUP  );
    this->distortion_.create( DLP_CV_DISTORTION_SETUP );
    this->extrinsic_.create(  DLP_CV_EXTRINSIC_SETUP  );

    this->homography_.clear();

    // Set all calibration data to zero
    this->intrinsic_.setTo(cv::Scalar(0.0));
    this->extrinsic_.setTo(cv::Scalar(0.0));
    this->distortion_.setTo(cv::Scalar(0.0));
}

/** @brief Destroys object and releases all allocated memory for calibration data */
Calibration::Data::~Data(){
    // Release all of the allocated memory for the calibration data
    this->intrinsic_.release();
    this->distortion_.release();
    this->extrinsic_.release();

    // Release all of the homography data
    for(unsigned int iBoard = 0; iBoard < this->homography_.size(); iBoard++){
        this->homography_.at(iBoard).release();
    }

    // Clear the homography vector
    this->homography_.clear();
}

/** @brief      Constructs object with copied values from data
 *  @param[in]  data \ref dlp::Calibration::Data object source
 */
Calibration::Data::Data(const Data &data){
    // Copy scalar values
    this->calibration_complete_  = data.calibration_complete_;
    this->calibration_of_camera_ = data.calibration_of_camera_;
    this->image_columns_         = data.image_columns_;
    this->image_rows_            = data.image_rows_;
    this->model_columns_         = data.model_columns_;
    this->model_rows_            = data.model_rows_;
    this->reprojection_error_    = data.reprojection_error_;

    // Clone the calibration cv::Mat data
    this->intrinsic_             = data.intrinsic_.clone();
    this->extrinsic_             = data.extrinsic_.clone();
    this->distortion_            = data.distortion_.clone();


    // Release all of the homography data
    for(unsigned int iBoard = 0; iBoard < this->homography_.size(); iBoard++){
        this->homography_.at(iBoard).release();
    }

    // Clear the homography vector
    this->homography_.clear();

    // Copy homograpy vector from data
    for(unsigned int iBoard = 0; iBoard < data.homography_.size(); iBoard++){
        cv::Mat homography = data.homography_.at(iBoard).clone();
        this->homography_.push_back(homography);
        homography.release();
    }
}

/** @brief      Copies all content from a \ref dlp::Calibration::Data object
 *  @param[in]  data    \ref dlp::Calibration::Data object source
 */
Calibration::Data & Calibration::Data::operator=(const Data &data){
    // Copy scalar values
    this->calibration_complete_  = data.calibration_complete_;
    this->calibration_of_camera_ = data.calibration_of_camera_;
    this->image_columns_         = data.image_columns_;
    this->image_rows_            = data.image_rows_;
    this->model_columns_         = data.model_columns_;
    this->model_rows_            = data.model_rows_;
    this->reprojection_error_    = data.reprojection_error_;

    // Clone the calibration cv::Mat data
    this->intrinsic_             = data.intrinsic_.clone();
    this->extrinsic_             = data.extrinsic_.clone();
    this->distortion_            = data.distortion_.clone();

    // Release all of the homography data
    for(unsigned int iBoard = 0; iBoard < this->homography_.size(); iBoard++){
        this->homography_.at(iBoard).release();
    }

    // Clear the homography vector
    this->homography_.clear();

    // Copy homograpy vector from data
    for(unsigned int iBoard = 0; iBoard < data.homography_.size(); iBoard++){
        cv::Mat homography = data.homography_.at(iBoard).clone();
        this->homography_.push_back(homography);
        homography.release();
    }

    return *this;
}

/** @brief Clears all calibration data to zero
 *  \note Does NOT deallocate calibration data memory
 */
void Calibration::Data::Clear(){
    // Reset boolean values
    this->calibration_complete_   = false;
    this->calibration_of_camera_  = true;

    // Clear the calibration data
    this->reprojection_error_     = 0.0;
    this->intrinsic_.setTo(  cv::Scalar(0) );
    this->extrinsic_.setTo(  cv::Scalar(0) );
    this->distortion_.setTo( cv::Scalar(0) );

    // Clear the homography data
    for(unsigned int iBoard = 0; iBoard < this->homography_.size(); iBoard++){
        this->homography_.at(iBoard).release();
    }
    this->homography_.clear();
}

/** @brief Returns true if instance has complete calibration data
 */
bool Calibration::Data::isComplete()const{
    return this->calibration_complete_;
}

/** @brief Returns true if instance contains calibration data from a \ref dlp::Calibration::Camera object
 */
bool Calibration::Data::isCamera()const{
    return this->calibration_of_camera_;
}

/** @brief      Retrieves calibration data in cv::Mat format
 *  @param[out] intrinsic            Calibration model matrix that contains focal length and focal point
 *  @param[out] extrinsic            Translation and rotation of calibration model
 *  @param[out] distortion           Distortion coefficients of calibration model lens
 *  @param[out] reprojection_error   Total sum of squared distances between the observed and known calibration board feature points
 *  @retval     CALIBRATION_DATA_NULL_POINTER_INTRINSIC             Input argument NULL - no data returned through pointers
 *  @retval     CALIBRATION_DATA_NULL_POINTER_EXTRINSIC             Input argument NULL - no data returned through pointers
 *  @retval     CALIBRATION_DATA_NULL_POINTER_DISTORTION            Input argument NULL - no data returned through pointers
 *  @retval     CALIBRATION_DATA_NULL_POINTER_REPROJECTION_ERROR    Input argument NULL - no data returned through pointers
 *  @retval     CALIBRATION_DATA_NOT_COMPLETE                       Object did not have complete calibration data - no data returned through pointers
 */
ReturnCode Calibration::Data::GetData(cv::Mat *intrinsic,
                                      cv::Mat *extrinsic,
                                      cv::Mat *distortion,
                                      double  *reprojection_error)const{
    ReturnCode ret;

    // Check for null pointer arguments and return error if present
    if(!intrinsic)          ret.AddError(CALIBRATION_DATA_NULL_POINTER_INTRINSIC);
    if(!extrinsic)          ret.AddError(CALIBRATION_DATA_NULL_POINTER_EXTRINSIC);
    if(!distortion)         ret.AddError(CALIBRATION_DATA_NULL_POINTER_DISTORTION);
    if(!reprojection_error) ret.AddError(CALIBRATION_DATA_NULL_POINTER_REPROJECTION_ERROR);

    // If data is not complete return error
    if(!this->isComplete()) ret.AddError(CALIBRATION_DATA_NOT_COMPLETE);

    // Return if errors are present
    if(ret.hasErrors()) return ret;

    // Clone the data into pointers.
    (*intrinsic)            = this->intrinsic_.clone();
    (*extrinsic)            = this->extrinsic_.clone();
    (*distortion)           = this->distortion_.clone();
    (*reprojection_error)   = this->reprojection_error_;

    return ret;
}

/** @brief      Retrieves image resolution of the images used for calibration
 *  @param[out] columns  %Number of pixel columns calibration images contained (width)
 *  @param[out] rows     %Number of pixel rows calibration images contained (height)
 *  @retval     CALIBRATION_DATA_NULL_POINTER_COLUMNS   Input argument NULL - no data returned through pointers
 *  @retval     CALIBRATION_DATA_NULL_POINTER_ROWS      Input argument NULL - no data returned through pointers
 *  @retval     CALIBRATION_DATA_NOT_COMPLETE           Object did not have complete calibration data - no data returned through pointers
 */
ReturnCode Calibration::Data::GetImageResolution(unsigned int *columns, unsigned int *rows) const{
    ReturnCode ret;

    // Check for null pointer arguments and return error if present
    if(!columns) ret.AddError(CALIBRATION_DATA_NULL_POINTER_COLUMNS);
    if(!rows)    ret.AddError(CALIBRATION_DATA_NULL_POINTER_ROWS);

    // If data is not complete return error
    if(!this->isComplete()) ret.AddError(CALIBRATION_DATA_NOT_COMPLETE);

    // Return if errors are present
    if(ret.hasErrors()) return ret;

    // Copy the data into pointers.
    (*columns) = this->image_columns_;
    (*rows)    = this->image_rows_;

    return ret;
}

/** @brief      Retrieves model resolution in pixels
 *  @param[out] columns  %Number of pixel columns model contains (width)
 *  @param[out] rows     %Number of pixel rows model contains (height)
 *  @retval     CALIBRATION_DATA_NULL_POINTER_COLUMNS   Input argument NULL - no data returned through pointers
 *  @retval     CALIBRATION_DATA_NULL_POINTER_ROWS      Input argument NULL - no data returned through pointers
 *  @retval     CALIBRATION_DATA_NOT_COMPLETE           Object did not have complete calibration data - no data returned through pointers
 */
ReturnCode Calibration::Data::GetModelResolution(unsigned int *columns, unsigned int *rows) const{
    ReturnCode ret;

    // Check for null pointer arguments and return error if present
    if(!columns) ret.AddError(CALIBRATION_DATA_NULL_POINTER_COLUMNS);
    if(!rows)    ret.AddError(CALIBRATION_DATA_NULL_POINTER_ROWS);

    // If data is not complete return error
    if(!this->isComplete()) ret.AddError(CALIBRATION_DATA_NOT_COMPLETE);

    // Return if errors are present
    if(ret.hasErrors()) return ret;

    // Copy the data into pointers.
    (*columns) = this->model_columns_;
    (*rows)    = this->model_rows_;

    return ret;
}

/** @brief      Saves calibration data to XML file
 *  @warning    Overwrites preexisting files
 *  @warning    Modifying the saved files is NOT recommended
 *  @param[in]  filename  Name of file to save calibration to
 *  @retval     CALIBRATION_DATA_FILE_SAVE_FAILED           Could not open or create file to save calibraiton data
 *  @retval     CALIBRATION_DATA_FILE_EXTENSION_INVALID     Filename argument did not have .xml extension
 *  @retval     CALIBRATION_DATA_NOT_COMPLETE               Object did not have complete calibration data
 */
ReturnCode Calibration::Data::Save(const std::string &filename){
    ReturnCode ret;

    // If data is not complete return error
    if(!this->isComplete()) ret.AddError(CALIBRATION_DATA_NOT_COMPLETE);

    // Check that file has .xml extension
    if( dlp::String::ToUpperCase(filename).rfind(".XML") == std::string::npos)
        ret.AddError(CALIBRATION_DATA_FILE_EXTENSION_INVALID);

    // Check for errors
    if(ret.hasErrors()) return ret;

    // Open XML file
    cv::FileStorage file(filename,cv::FileStorage::WRITE);

    // Check that the file is open
    if(!file.isOpened())
        return ret.AddError(CALIBRATION_DATA_FILE_SAVE_FAILED);


    // Write calibration data to file
    file << "DLP_CALIBRATION_DATA"  << true;
    file << "calibration_complete"  << this->calibration_complete_;
    file << "calibration_of_camera" << this->calibration_of_camera_;
    file << "image_columns"         << (int) this->image_columns_;
    file << "image_rows"            << (int) this->image_rows_;
    file << "model_columns"         << (int) this->model_columns_;
    file << "model_rows"            << (int) this->model_rows_;
    file << "reprojection_error"    << this->reprojection_error_;
    file << "intrinsic"             << this->intrinsic_;
    file << "distortion"            << this->distortion_;
    file << "extrinsic"             << this->extrinsic_;

    // Close the file
    file.release();

    return ret;
}

/** @brief      Loads calibration data from XML file
 *  @warning    There is NO error or content checking to determine if data is truly complete.
 *              Object is marked as complete if successfully loaded.
 *  @param[in]  filename  Name of file to load calibration data from
 *  @retval     CALIBRATION_DATA_FILE_LOAD_FAILED           Could not open file to load calibraiton data
 *  @retval     CALIBRATION_DATA_FILE_INVALID               File did not contain "DLP_CALIBRATION_DATA" flag
 *  @retval     CALIBRATION_DATA_FILE_EXTENSION_INVALID     Filename argument did not have .xml extension
 */
ReturnCode Calibration::Data::Load(const std::string &filename){
    ReturnCode ret;

    // Check that file has .xml extension
    if( dlp::String::ToUpperCase(filename).rfind(".XML") == std::string::npos)
        return ret.AddError(CALIBRATION_DATA_FILE_EXTENSION_INVALID);


    // Open XML file
    cv::FileStorage file(filename,cv::FileStorage::READ);

    // Check that the file is open
    if(!file.isOpened())
        return ret.AddError(CALIBRATION_DATA_FILE_LOAD_FAILED);


    // Write calibration data to file
    bool is_dlp_calibration_data;
    file["DLP_CALIBRATION_DATA"] >> is_dlp_calibration_data;

    // Check that this is a dlp claibraiton data file
    if(!is_dlp_calibration_data)
        return ret.AddError(CALIBRATION_DATA_FILE_INVALID);

    // Save the calibration data
    int temp;
    file["calibration_complete"]  >> this->calibration_complete_;
    file["calibration_of_camera"] >> this->calibration_of_camera_;
    file["image_columns"]         >> temp;
    this->image_columns_ = (unsigned int) temp;

    file["image_rows"]            >> temp;
    this->image_rows_ = (unsigned int) temp;

    file["model_columns"]         >> temp;
    this->model_columns_ = (unsigned int) temp;

    file["model_rows"]            >> temp;
    this->model_rows_ = (unsigned int) temp;

    file["reprojection_error"]    >> this->reprojection_error_;
    file["intrinsic"]             >> this->intrinsic_;
    file["distortion"]            >> this->distortion_;
    file["extrinsic"]             >> this->extrinsic_;

    // Mark the calibration data as complete
    this->calibration_complete_ = true;

    // Close the file
    file.release();

    return ret;
}

} // End namespace dlp
