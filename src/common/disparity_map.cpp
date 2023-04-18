/** @file       disparity_map.cpp
 *  @ingroup    Common
 *  @brief      Contains methods for \ref dlp::DisparityMap
 *  @copyright 2014 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#include <common/debug.hpp>
#include <common/returncode.hpp>
#include <common/image/image.hpp>
#include <common/other.hpp>
#include <common/parameters.hpp>
#include <common/disparity_map.hpp>

/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{

const int DisparityMap::INVALID_PIXEL   = 0xFFFF;
const int DisparityMap::EMPTY_PIXEL     = -1;

/** @brief  Object constructor */
DisparityMap::DisparityMap(){
    this->Clear();
    this->orientation_ = dlp::Pattern::Orientation::INVALID;
}

/** @brief  Destroys object and releases image memory */
DisparityMap::~DisparityMap(){
    this->Clear();
}


/** @brief Initializes empty object with specified resolution
 *  @param[in] columns      %Number of columns
 *  @param[in] rows         %Number of rows
 *  @param[in] orientation  %Orientation Horizontal or Vertical
 */
DisparityMap::DisparityMap(const unsigned int &columns, const unsigned int &rows, const dlp::Pattern::Orientation &orientation){
    this->Create(columns,rows,orientation,1);
}

/** @brief Initializes empty object with specified resolution
 *  @param[in] columns      %Number of columns
 *  @param[in] rows         %Number of rows
 *  @param[in] orientation      %Orientation Horizontal or Vertical
 *  @param[in] over_sample      %Oversample
 */
DisparityMap::DisparityMap(const unsigned int &columns, const unsigned int &rows, const dlp::Pattern::Orientation &orientation, const unsigned int &over_sample){
    this->Create(columns,rows,orientation,over_sample);
}

/** @brief Allocates memory for object
 *  @warning This method clears any previous data stored in the object
 *  @param[in] columns  %Number of columns
 *  @param[in] rows     %Number of rows
 *  @param[in] orientation      %Orientation Horizontal or Vertical
 *  @param[in] over_sample      %Oversample
 *  @retval IMAGE_CREATION_FAILED   Memory allocation failed
 */
ReturnCode DisparityMap::Create(const unsigned int &columns, const unsigned int &rows, const dlp::Pattern::Orientation &orientation, const unsigned int &over_sample){
    ReturnCode ret = this->map_.Create(columns,rows,dlp::Image::Format::MONO_INT);

    this->orientation_ = orientation;

    if(over_sample >= 1){
        this->over_sample_ = over_sample;
    }
    else{
        ret.AddWarning("Oversampling set to 1");
        this->over_sample_ = 1;
    }

    if(!ret.hasErrors()) this->map_.FillImage(EMPTY_PIXEL);
    return ret;
}

/** @brief Allocates memory for object
 *  @warning This method clears any previous data stored in the object
 *  @param[in] columns     %Number of columns
 *  @param[in] rows        %Number of rows
 *  @param[in] orientation %Orientation Horizontal or Vertical
 *  @retval IMAGE_CREATION_FAILED   Memory allocation failed
 */
ReturnCode DisparityMap::Create(const unsigned int &columns, const unsigned int &rows, const dlp::Pattern::Orientation &orientation){
    return this->Create(columns,rows,orientation,1);
}

/** @brief  Allocates memory for object with the same resolution as the
 *          supplied \ref dlp::DisparityMap object
 *  @retval IMAGE_CREATION_FAILED   Memory allocation failed
 */
ReturnCode DisparityMap::Create(DisparityMap map){
    ReturnCode ret;

    // Check if map is empty
    if(map.isEmpty())
        return ret.AddError(DISPARITY_MAP_EMPTY);

    this->Clear();

    ret = this->map_.Create(map.map_);

    this->orientation_ = map.orientation_;
    this->over_sample_ = map.over_sample_;

    return ret;
}

/** @brief Deallocates object data memory
 *  @warning Create() must be called after this method before the object can be used again
 */
void DisparityMap::Clear(){
    this->map_.Clear();
    this->orientation_ = dlp::Pattern::Orientation::INVALID;
    this->over_sample_ = 1;
}

/** @brief Returns true if the disparity map has no data */
bool DisparityMap::isEmpty()const{
    return this->map_.isEmpty();
}

/**
 *  @brief Retrieves %number of columns
 *  @param[out] columns Pointer for return value
 *  @retval DISPARITY_MAP_NULL_POINTER_COLUMNS  Return argument NULL
 *  @retval DISPARITY_MAP_EMPTY                 DisparityMap has NOT been created
 */
ReturnCode DisparityMap::GetColumns(unsigned int *columns){
    ReturnCode ret;

    // Check if map is empty
    if(this->isEmpty())
        return ret.AddError(DISPARITY_MAP_EMPTY);

    if(!columns)
        return ret.AddError(DISPARITY_MAP_NULL_POINTER_COLUMNS);

    this->map_.GetColumns(columns);

    return ret;
}

/**
 *  @brief Retrieves %Number of rows
 *  @param[out] rows Pointer for return value
 *  @retval DISPARITY_MAP_NULL_POINTER_ROWS  Return argument NULL
 *  @retval DISPARITY_MAP_EMPTY                 DisparityMap has NOT been created
 */
ReturnCode DisparityMap::GetRows(unsigned int *rows){
    ReturnCode ret;

    // Check if map is empty
    if(this->isEmpty())
        return ret.AddError(DISPARITY_MAP_EMPTY);

    if(!rows)
        return ret.AddError(DISPARITY_MAP_NULL_POINTER_ROWS);

    this->map_.GetRows(rows);

    return ret;
}

/**
 *  @brief Retrieves the orientation of disparity map
 *  @param[out] orientation Pointer for return value
 *  @retval DISPARITY_MAP_NULL_POINTER_ORIENTATION  Return argument NULL
 *  @retval DISPARITY_MAP_EMPTY                     DisparityMap has NOT been created
 */
ReturnCode DisparityMap::GetOrientation(dlp::Pattern::Orientation *orientation){
    ReturnCode ret;

    (*orientation) = dlp::Pattern::Orientation::INVALID;

    // Check if map is empty
    if(this->isEmpty())
        return ret.AddError(DISPARITY_MAP_EMPTY);

    if(!orientation)
        return ret.AddError(DISPARITY_MAP_NULL_POINTER_ORIENTATION);

    (*orientation) = this->orientation_;

    return ret;
}

ReturnCode DisparityMap::GetDisparitySampling(unsigned int *over_sample){
    ReturnCode ret;

    // Check if map is empty
    if(this->isEmpty())
        return ret.AddError(DISPARITY_MAP_EMPTY);

    if(!over_sample)
        return ret.AddError(DISPARITY_MAP_NULL_POINTER_ORIENTATION);

    (*over_sample) = this->over_sample_;

    return ret;
}

/** @brief      Clones the object's data into an OpenCV cv::Mat object
 *  \note       This method is a deep copy
 *  @param[out] data cv::Mat object pointer to copy image data to
 *  @retval     IMAGE_NULL_POINTER_ARGUMENT_DATA    Input argument NULL
 *  @retval     IMAGE_EMPTY                         Object has no data to copy
 */
ReturnCode DisparityMap::GetOpenCVData(cv::Mat *data)const{
    ReturnCode ret;

    // Check that pointer is NOT null
    if(!data) return ret.AddError(IMAGE_NULL_POINTER_ARGUMENT_DATA);

    // Check that the image has data (a.k.a. it is NOT empty)
    if(this->isEmpty() != true ){
        // Clone the image data
        cv::Mat cv_data;
        this->map_.GetOpenCVData(&cv_data);
        (*data) = cv_data;
    }
    else{
        // The image is empty
        ret.AddError(IMAGE_EMPTY);
    }

    return ret;
}

/** @brief      Copies the object's data pointer into an OpenCV cv::Mat object
 *  @warning    This method is a shallow copy of the image data! The cv::Mat pointer has direct access to the \ref dlp::Image data and CAN modify it
 *  @param[out] data        cv::Mat object pointer to copy image data to
 *  @retval     IMAGE_NULL_POINTER_ARGUMENT_DATA    Input argument NULL
 *  @retval     IMAGE_EMPTY                         Object has no data to copy
 */
ReturnCode DisparityMap::Unsafe_GetOpenCVData(cv::Mat *data){
    ReturnCode ret;

    // Check that pointer is NOT null
    if(!data) return ret.AddError(IMAGE_NULL_POINTER_ARGUMENT_DATA);


    // Check that the image has data (a.k.a. it is NOT empty)
    if(this->isEmpty() != true ){
        cv::Mat cv_data;
        this->map_.Unsafe_GetOpenCVData(&cv_data);
        (*data) = cv_data;
    }
    else{
        // The image is empty
        ret.AddError(IMAGE_EMPTY);
    }

    return ret;
}


ReturnCode DisparityMap::OversampleAndSmooth(const unsigned int &over_sample){
    ReturnCode ret;

    // Check if map is empty
    if(this->isEmpty())
        return ret.AddError(DISPARITY_MAP_EMPTY);

    if(over_sample <= 1)
        return ret;

    cv::Mat original;
    cv::Mat map;
    cv::Mat smooth;
    this->map_.Unsafe_GetOpenCVData(&map);

    // Multiply all pixel values times the over sampling value
    map = map * over_sample;

    // Clone the original data
    original = map.clone();


    unsigned int i = over_sample;

    if((i%2)!=1)i++;

    original.convertTo(original,CV_32F);

    cv::bilateralFilter ( original, smooth, over_sample, over_sample*3, over_sample*3);

    smooth.convertTo(map,CV_32S);

    this->over_sample_ = over_sample;

    return ret;
}


/**
 * @brief Sets the pixel value at the specified coordinate
 * @param[in]   x_col           Column coordinate of desired pixel
 * @param[in]   y_row           Row coordinate of desired pixel
 * @param[in]   value       Value to set pixel to
 * @retval      DISPARITY_MAP_EMPTY                 Image has NOT been created
 * @retval      IMAGE_PIXEL_OUT_OF_RANGE            Supplied coordinates are NOT valid
 */
ReturnCode DisparityMap::SetPixel(const unsigned int &x_col, const unsigned int &y_row, const int &value){
    ReturnCode ret;

    // Check if map is empty
    if(this->isEmpty())
        return ret.AddError(DISPARITY_MAP_EMPTY);

    return this->map_.SetPixel(x_col,y_row,value);
}

/**
 * @brief Sets the pixel value at the specified coordinate as \ref dlp::DisparityMap::INVALID_PIXEL
 * @param[in]   x_col           Column coordinate of desired pixel
 * @param[in]   y_row           Row coordinate of desired pixel
 * @retval      DISPARITY_MAP_EMPTY                 Image has NOT been created
 * @retval      IMAGE_PIXEL_OUT_OF_RANGE            Supplied coordinates are NOT valid
 */
ReturnCode DisparityMap::SetPixelInvalid(const unsigned int &x_col, const unsigned int &y_row){
    ReturnCode ret;

    // Check if map is empty
    if(this->isEmpty())
        return ret.AddError(DISPARITY_MAP_EMPTY);

    return this->map_.SetPixel(x_col,y_row,INVALID_PIXEL);
}

/**
 * @brief Retrieves pixel value from specific coordinate
 * @param[in]   x_col           Column coordinate of desired pixel
 * @param[in]   y_row           Row coordinate of desired pixel
 * @param[out]  value           Pointer for return value
 * @retval      DISPARITY_MAP_EMPTY                     dlp::DisparityMap has NOT been created
 * @retval      IMAGE_NULL_POINTER_ARGUMENT_RET_VAL     Return argument NULL
 * @retval      IMAGE_PIXEL_OUT_OF_RANGE                Supplied coordinates are NOT valid
 */
ReturnCode DisparityMap::GetPixel(const unsigned int &x_col, const unsigned int &y_row, int *value){
    ReturnCode ret;

    (*value) = INVALID_PIXEL;

    // Check if map is empty
    if(this->isEmpty())
        return ret.AddError(DISPARITY_MAP_EMPTY);

    return this->map_.GetPixel(x_col,y_row,value);
}

/**
 * @brief Retrieves pixel value from specific coordinate
 * @param[in]   x_col           Column coordinate of desired pixel
 * @param[in]   y_row           Row coordinate of desired pixel
 * @param[out]  value       Pointer for return value
 * @param[out]  valid       Returns true if the pixel value is NOT equal to \ref dlp::DisparityMap::INVALID_PIXEL
 * @retval      DISPARITY_MAP_EMPTY                     dlp::DisparityMap has NOT been created
 * @retval      IMAGE_NULL_POINTER_ARGUMENT_RET_VAL     Return argument NULL
 * @retval      IMAGE_PIXEL_OUT_OF_RANGE                Supplied coordinates are NOT valid
 */
ReturnCode DisparityMap::GetPixel(const unsigned int &x_col, const unsigned int &y_row, int *value, bool *valid){
    ReturnCode ret;

    // Check if map is empty
    if(this->isEmpty())
        return ret.AddError(DISPARITY_MAP_EMPTY);

    ret = this->map_.GetPixel(x_col,y_row,value);

    if(ret.hasErrors())
        return ret;

    if((*value) == INVALID_PIXEL){
        (*valid) = false;
    }
    else{
        (*valid) = true;
    }

    return ret;
}


/**
 * @brief Sets the pixel value at the specified coordinate
 * @warning NO error checking is performed and attempting to access an empty image or out of index pixel will crash the program!
 * @param[in]   x_col           Column coordinate of desired pixel
 * @param[in]   y_row           Row coordinate of desired pixel
 * @param[in]   value       Value to set pixel to
 */
void DisparityMap::Unsafe_SetPixel(const unsigned int &x_col, const unsigned int &y_row, const int &value){

    this->map_.Unsafe_SetPixel(x_col,y_row,value);

    return;
}

/**
 * @brief Sets the pixel value at the specified coordinate as \ref dlp::DisparityMap::INVALID_PIXEL
 * @warning NO error checking is performed and attempting to access an empty image or out of index pixel will crash the program!
 * @param[in]   x_col           Column coordinate of desired pixel
 * @param[in]   y_row           Row coordinate of desired pixel
 */
void DisparityMap::Unsafe_SetPixelInvalid(const unsigned int &x_col, const unsigned int &y_row){

    this->map_.Unsafe_SetPixel(x_col,y_row,INVALID_PIXEL);

    return;
}

/**
 * @brief Retrieves pixel value from specific coordinate
 * @warning NO error checking is performed and attempting to access an empty image or out of index pixel will crash the program!
 * @param[in]   x_col           Column coordinate of desired pixel
 * @param[in]   y_row           Row coordinate of desired pixel
 * @param[out]  value       Pointer for return value
 */
void DisparityMap::Unsafe_GetPixel(const unsigned int &x_col, const unsigned int &y_row, int *value){
    this->map_.Unsafe_GetPixel(x_col,y_row,value);

    return;
}

/**
 * @brief Retrieves pixel value from specific coordinate
 * @warning NO error checking is performed and attempting to access an empty image or out of index pixel will crash the program!
 * @param[in]   x_col           Column coordinate of desired pixel
 * @param[in]   y_row           Row coordinate of desired pixel
 * @param[out]  value       Pointer for return value
 * @param[out]  valid       Returns true if the pixel value is NOT equal to \ref dlp::DisparityMap::INVALID_PIXEL
 */
void DisparityMap::Unsafe_GetPixel(const unsigned int &x_col, const unsigned int &y_row, int *value, bool *valid){

    this->map_.Unsafe_GetPixel(x_col,y_row,value);

    if((*value) == INVALID_PIXEL){
        (*valid) = false;
    }
    else{
        (*valid) = true;
    }

    return;
}

/**
 * @brief   Flips the \ref dlp::DisparityMap across one or both axis
 * @param[in] flip_x    Flip the image across the x axis (vertical flip)
 * @param[in] flip_y    Flip the image across the y axis (horizontal flip)
 * @retval  IMAGE_EMPTY                 Image has NOT been created
 */
ReturnCode DisparityMap::FlipImage( const bool &flip_x, const bool &flip_y){
    ReturnCode ret;

    ret = this->map_.FlipImage(flip_x,flip_y);

    return ret;
}


/** @brief Returns the DisparityMap as a \ref dlp::Image */
dlp::Image DisparityMap::GetImage(){
    // Make deep copy of disparity map
    dlp::Image ret(this->map_);

    return ret;
}


}
