/** @file   gray_code.cpp
 *  @brief  Contains methods to gray code
 *  @copyright 2014 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */


#include <common/returncode.hpp>
#include <common/debug.hpp>
#include <common/parameters.hpp>
#include <common/capture/capture.hpp>
#include <common/pattern/pattern.hpp>
#include <common/disparity_map.hpp>

#include <structured_light/structured_light.hpp>
#include <structured_light/gray_code/gray_code.hpp>

#include <math.h>

/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{

/** @brief Constructs object */
GrayCode::GrayCode(){
    this->debug_.SetName("STRUCTURED_LIGHT_GRAY_CODE(" + dlp::Number::ToString(this)+ "): ");
    this->debug_.Msg("Constructing object...");
    this->is_setup_ = false;
    this->disparity_map_.Clear();
    this->include_inverted_.Set(true);
    this->pattern_color_.Set(dlp::Pattern::Color::WHITE);

    this->debug_.Msg("Object constructed");
}

/** @brief Destroys object and deallocates memory */
GrayCode::~GrayCode(){
    this->debug_.Msg("Deconstructing object...");
    this->disparity_map_.Clear();
    this->debug_.Msg("Object deconstructed");
}

/** @brief Retrieves settings from \ref dlp::Parameters object to configure
 *  \ref dlp::Pattern::Sequence generation and \ref dlp::Capture::Sequence decoding
 *  @param[in]  settings    \ref dlp::Parameters object to retrieve settings from
 *
 * @retval STRUCTURED_LIGHT_SETTINGS_SEQUENCE_COUNT_MISSING             \ref dlp::Parameters list missing \ref dlp::GrayCode::sequence_count_
 * @retval STRUCTURED_LIGHT_SETTINGS_SEQUENCE_INCLUDE_INVERTED_MISSING  \ref dlp::Parameters list missing \ref dlp::GrayCode::include_inverted_
 * @retval STRUCTURED_LIGHT_SETTINGS_PATTERN_COLOR_MISSING              \ref dlp::Parameters list missing \ref dlp::StructuredLight::pattern_color_
 * @retval STRUCTURED_LIGHT_SETTINGS_PATTERN_ROWS_MISSING               \ref dlp::Parameters list missing \ref dlp::StructuredLight::pattern_rows_
 * @retval STRUCTURED_LIGHT_SETTINGS_PATTERN_COLUMNS_MISSING            \ref dlp::Parameters list missing \ref dlp::StructuredLight::pattern_columns_
 * @retval STRUCTURED_LIGHT_SETTINGS_PATTERN_ORIENTATION_MISSING        \ref dlp::Parameters list missing \ref dlp::StructuredLight::pattern_orientation_
 * @retval GRAY_CODE_PIXEL_THRESHOLD_MISSING                            \ref dlp::Parameters list missing \ref dlp::GrayCode::pixel_threshold_
 */
ReturnCode GrayCode::Setup(const dlp::Parameters &settings){
    ReturnCode ret;


    if((settings.Get(&this->pattern_color_)).hasErrors())
        return ret.AddError(STRUCTURED_LIGHT_SETTINGS_PATTERN_COLOR_MISSING);

    if(!this->projector_set_){
        if(settings.Get(&this->pattern_rows_).hasErrors())
            return ret.AddError(STRUCTURED_LIGHT_SETTINGS_PATTERN_ROWS_MISSING);

       if((settings.Get(&this->pattern_columns_).hasErrors()))
            return ret.AddError(STRUCTURED_LIGHT_SETTINGS_PATTERN_COLUMNS_MISSING);
    }

    if(settings.Get(&this->pattern_orientation_).hasErrors())
        return ret.AddError(STRUCTURED_LIGHT_SETTINGS_PATTERN_ORIENTATION_MISSING);

    // If vertical patterns are used the resolution of the gray coded patterns
    // is determined by the number of columns. If horizontal patterns are used
    // the resolution of the gray coded patterns is determined by the number
    // of rows
    switch(this->pattern_orientation_.Get()){
    case dlp::Pattern::Orientation::VERTICAL:
        this->resolution_ = this->pattern_columns_.Get();
        break;
    case dlp::Pattern::Orientation::HORIZONTAL:
        this->resolution_ = this->pattern_rows_.Get();
        break;
    case dlp::Pattern::Orientation::DIAMOND_ANGLE_1:
    case dlp::Pattern::Orientation::DIAMOND_ANGLE_2:
        this->resolution_ = this->pattern_columns_.Get() +
                            (this->pattern_rows_.Get()/2);
        break;
    case dlp::Pattern::Orientation::INVALID:
    default:
        return ret.AddError(STRUCTURED_LIGHT_NOT_SETUP);
        break;
    }

    if(settings.Get(&this->include_inverted_).hasErrors())
        return ret.AddError(STRUCTURED_LIGHT_SETTINGS_SEQUENCE_INCLUDE_INVERTED_MISSING);

    if(settings.Get(&this->pixel_threshold_).hasErrors())
        return ret.AddError(GRAY_CODE_PIXEL_THRESHOLD_MISSING);

    if(settings.Contains(this->measure_regions_)){
        // Module will measure regions rather than pixels
        settings.Get(&this->measure_regions_);

        // Calculate the region size in pixels
        this->region_size_ = (unsigned int) roundf((float(this->resolution_) / this->measure_regions_.Get()));
        std::cout << "Region size = " << this->region_size_ << std::endl;

        // Check that the rounded region size is not too small or large
        unsigned int calculated_resolution = (unsigned int) roundf(this->region_size_*this->measure_regions_.Get());

        if( calculated_resolution != this->resolution_)
            return ret.AddError(GRAY_CODE_REGIONS_REQUIRE_SUB_PIXELS);

        // The maximum disparity is determined by the number of regions and needs to be
        // a power of two for the decoding process
        this->maximum_patterns_  = (unsigned int)ceil(log2((double)this->measure_regions_.Get()));
        this->maximum_disparity_ = 1ul << this->maximum_patterns_;
        this->sequence_count_.Set(this->maximum_patterns_);

        // The MSB pattern value is determined by the maximum disparity value
        this->msb_pattern_value_ = this->maximum_disparity_ >> 1;

        // Set the offset to zero so that regions are not shifted
        this->offset_ = 0;
    }
    else{
        this->measure_regions_.Set(0.0);

        ret = settings.Get(&this->sequence_count_);
        if(ret.hasErrors())
            return ret.AddError(STRUCTURED_LIGHT_SETTINGS_SEQUENCE_COUNT_MISSING);

        // The maximum disparity is determined by the resolution and needs to be
        // a power of two for the decoding process
        this->maximum_patterns_  = (unsigned int)ceil(log2((double)this->resolution_));
        this->maximum_disparity_ = 1ul << this->maximum_patterns_;

        // The MSB pattern value is determined by the maximum disparity value
        this->msb_pattern_value_ = this->maximum_disparity_ >> 1;

        // Since the maximum disparity value will be larger than the resolution
        // an offset must be stored to adjust the decoded values to the correct
        // disparity values on the DMD
        this->offset_ = floor((this->maximum_disparity_ - this->resolution_)/2);

        // Check that request pattern count is valid
        if(this->sequence_count_.Get() > this->maximum_patterns_)
            return ret.AddError(STRUCTURED_LIGHT_CAPTURE_SEQUENCE_SIZE_INVALID);
    }


    if(this->include_inverted_.Get()){
        this->sequence_count_total_ = this->sequence_count_.Get() * 2;
    }
    else{
        this->sequence_count_total_ = this->sequence_count_.Get() + 2; // Add 2 for all on and all off albedo calculation images
    }


    // Setup has been completed
    this->is_setup_ = true;

    return ret;
}

/** @brief Generates a \ref dlp::Pattern::Sequence based on the settings specified
 *  @param[out] pattern_sequence Return pointer to \ref dlp::Pattern::Sequence
 *  @retval STRUCTURED_LIGHT_NULL_POINTER_ARGUMENT  Return argument is NULL
 *  @retval STRUCTURED_LIGHT_NOT_SETUP              Module has NOT been setup
 *  @retval STRUCTURED_LIGHT_CAPTURE_SEQUENCE_SIZE_INVALID  Requested number of patterns is NOT possible with the specified DMD resolution
 */
ReturnCode GrayCode::GeneratePatternSequence(Pattern::Sequence *pattern_sequence){
    ReturnCode ret;

    // Check that GrayCode object is setup
    if(!this->isSetup())
        return ret.AddError(STRUCTURED_LIGHT_NOT_SETUP);

    // Check that argument is not null
    if(!pattern_sequence)
        return ret.AddError(STRUCTURED_LIGHT_NULL_POINTER_ARGUMENT);

    // Generate the binary code values
    std::vector< unsigned int > line;
    std::vector< std::vector< unsigned int > > pattern_line;
    std::vector< std::vector< unsigned int > > pattern_line_gray;

    // Clear the pattern sequence
    unsigned int line_size;
    pattern_sequence->Clear();


    // Check if the generated patterns are for region or pixel measurements
    if(this->measure_regions_.Get() > 0.0){
       \
        // Measure regions
        line_size = this->resolution_;

        // First create a set of binary patterns
        for(unsigned int iPattern = 0; iPattern < this->maximum_patterns_; iPattern++){
            unsigned int column_width = this->region_size_ << (this->maximum_patterns_ - 1 - iPattern );
            bool white_region = false;
            unsigned int  region_count = 0;
            line.clear();
            for(unsigned int iPoint = 0; iPoint < this->resolution_; iPoint++){

                line.push_back( white_region );

                region_count++;

                if(region_count == column_width){
                    white_region = !white_region;
                    region_count = 0;
                }
            }
            pattern_line.push_back(line);
            pattern_line_gray.push_back(line);
        }
    }
    else{
        // Measure pixels
        line_size = this->maximum_disparity_;

        // First create a set of binary patterns
        for(unsigned int iPattern = 0; iPattern < this->maximum_patterns_; iPattern++){
            line.clear();
            for(unsigned int iPoint = 0; iPoint < this->maximum_disparity_; iPoint++){
                line.push_back( (iPoint & ( this->maximum_disparity_ >> (iPattern+1))) > 0);
            }
            pattern_line.push_back(line);
            pattern_line_gray.push_back(line);
        }
    }

    // Convert to binary codes to gray codes
    // First pattern of binary and gray coded sequences are identical so skip first pattern
    for(unsigned int iPattern = 1; iPattern < this->maximum_patterns_; iPattern++){
        // For each point
        for(unsigned int iPoint = 0; iPoint < line_size; iPoint++){
            unsigned int gray_coded_point;
            unsigned int old_point = pattern_line.at(iPattern-1).at(iPoint);
            unsigned int new_point = pattern_line.at(iPattern).at(iPoint);

            // XOR the current value with the previous one
            gray_coded_point = old_point ^ new_point;

            // Overwrite the old value with the new one
            pattern_line_gray.at(iPattern).at(iPoint) = gray_coded_point;
        }
    }

    // Get the image resolution
    unsigned int rows     = this->pattern_rows_.Get();
    unsigned int columns  = this->pattern_columns_.Get();


    // If the inverted patterns are not included add a white and
    // black image for the Albedo threshold
    if(!this->include_inverted_.Get()){
        dlp::Pattern pattern_white;
        dlp::Image   image_white;

        dlp::Pattern pattern_black;
        dlp::Image   image_black;


        // Allocate memory for the albedo images
        image_white.Create(columns,rows,dlp::Image::Format::MONO_UCHAR);
        image_black.Create(columns,rows,dlp::Image::Format::MONO_UCHAR);

        // Create the images
        for(     unsigned int yRow = 0; yRow < rows;    yRow++){
            for( unsigned int xCol = 0; xCol < columns; xCol++){
                image_white.Unsafe_SetPixel(xCol,yRow,(unsigned char)255);
                image_black.Unsafe_SetPixel(xCol,yRow,(unsigned char)0);
            }
        }

        // Create the white pattern
        pattern_white.image_data.Create(image_white);
        pattern_white.color     = this->pattern_color_.Get();
        pattern_white.data_type = dlp::Pattern::DataType::IMAGE_DATA;
        pattern_white.bitdepth  = dlp::Pattern::Bitdepth::MONO_1BPP;

        // Add the inverted pattern to the sequence
        pattern_sequence->Add(pattern_white);

        // Create the white pattern
        pattern_black.image_data.Create(image_black);
        pattern_black.color     = this->pattern_color_.Get();
        pattern_black.data_type = dlp::Pattern::DataType::IMAGE_DATA;
        pattern_black.bitdepth  = dlp::Pattern::Bitdepth::MONO_1BPP;

        // Add the inverted pattern to the sequence
        pattern_sequence->Add(pattern_black);
    }


    // Generate the pattern sequence
    for(unsigned int iPattern = 0; iPattern < this->sequence_count_.Get(); iPattern++){
        dlp::Pattern pattern;
        dlp::Image   image;

        dlp::Pattern pattern_inverted;
        dlp::Image   image_inverted;

        // Create the new image
        image.Create(columns,rows,dlp::Image::Format::MONO_UCHAR);

        switch(this->pattern_orientation_.Get()){
        case dlp::Pattern::Orientation::VERTICAL:
            // Set the pixels
            for(     unsigned int yRow = 0; yRow < rows;    yRow++){
                for( unsigned int xCol = 0; xCol < columns; xCol++){
                    unsigned int value = pattern_line_gray.at(iPattern).at(xCol + this->offset_);
                    unsigned char bitplane_value = 255*value;

                    // Save the pixel value
                    image.Unsafe_SetPixel(xCol,yRow,bitplane_value);
                }
            }

            break;
        case dlp::Pattern::Orientation::HORIZONTAL:
            // Set the pixels
            for(     unsigned int yRow = 0; yRow < rows;    yRow++){
                for( unsigned int xCol = 0; xCol < columns; xCol++){
                    unsigned int value = pattern_line_gray.at(iPattern).at(yRow + this->offset_);
                    unsigned char bitplane_value = 255*value;

                    // Save the pixel value
                    image.Unsafe_SetPixel(xCol,yRow,bitplane_value);
                }
            }
            break;
        case dlp::Pattern::Orientation::DIAMOND_ANGLE_2:
            // Set the pixels
            for(     unsigned int yRow = 0; yRow < rows;    yRow++){
                for( unsigned int xCol = 0; xCol < columns; xCol++){
                    unsigned int code = ((rows - yRow)/2) + xCol;
                    unsigned int value = pattern_line_gray.at(iPattern).at(code + this->offset_);
                    unsigned char bitplane_value = 255*value;

                    // Save the pixel value
                    image.Unsafe_SetPixel(xCol,yRow,bitplane_value);
                }
            }
            break;
        case dlp::Pattern::Orientation::DIAMOND_ANGLE_1:
            // Set the pixels
            for(     unsigned int yRow = 0; yRow < rows;    yRow++){
                for( unsigned int xCol = 0; xCol < columns; xCol++){
                    unsigned int code = (yRow/2) + xCol;
                    unsigned int value = pattern_line_gray.at(iPattern).at(code + this->offset_);
                    unsigned char bitplane_value = 255*value;

                    // Save the pixel value
                    image.Unsafe_SetPixel(xCol,yRow,bitplane_value);
                }
            }
            break;
        case dlp::Pattern::Orientation::INVALID:
        default:
            return ret.AddError(STRUCTURED_LIGHT_NOT_SETUP);
            break;
        }


        // Add this image to the Pattern
        pattern.image_data.Create(image);
        pattern.color     = this->pattern_color_.Get();
        pattern.data_type = dlp::Pattern::DataType::IMAGE_DATA;
        pattern.bitdepth  = dlp::Pattern::Bitdepth::MONO_1BPP;

        // Add this pattern to the sequence
        pattern_sequence->Add(pattern);

        // Create the inverted image if needed
        if(this->include_inverted_.Get()){
            // Allocate memory for the inverted image
            image_inverted.Create(columns,rows,dlp::Image::Format::MONO_UCHAR);

            // Copy and invert the pixel values from the noninverted pattern
            for(     unsigned int yRow = 0; yRow < rows;    yRow++){
                for( unsigned int xCol = 0; xCol < columns; xCol++){
                    unsigned char value;
                    unsigned char value_inverted;
                    // Get the normal pixel value
                    image.Unsafe_GetPixel(xCol,yRow,&value);

                    // Invert the value
                    value_inverted = 255 - value;

                    // Save the inverted value
                    image_inverted.Unsafe_SetPixel(xCol,yRow, value_inverted);
                }
            }

            // Create a pattern for the inverted image
            pattern_inverted.image_data.Create(image_inverted);
            pattern_inverted.color     = this->pattern_color_.Get();
            pattern_inverted.data_type = dlp::Pattern::DataType::IMAGE_DATA;
            pattern_inverted.bitdepth  = dlp::Pattern::Bitdepth::MONO_1BPP;

            // Add the inverted pattern to the sequence
            pattern_sequence->Add(pattern_inverted);
        }

        // Clear the image
        image.Clear();
        image_inverted.Clear();
    }

    return ret;
}



/** @brief Decodes the \ref dlp::Capture::Sequence and returns the \ref dlp::DisparityMap
 *  @param[in] capture_sequence \ref dlp::Capture::Sequence to be decoded
 *  @param[in] disparity_map Return pointer for generated \ref dlp::DisparityMap
 *  @retval STRUCTURED_LIGHT_NULL_POINTER_ARGUMENT      Input arguments NULL
 *  @retval STRUCTURED_LIGHT_NOT_SETUP                  Module has NOT been setup
 *  @retval STRUCTURED_LIGHT_CAPTURE_SEQUENCE_EMPTY     Supplied sequence is empty
 *  @retval STRUCTURED_LIGHT_CAPTURE_SEQUENCE_SIZE_INVALID  Supplied sequence has a difference count than what was generated
 *  @retval STRUCTURED_LIGHT_DATA_TYPE_INVALID          Supplied sequence does NOT contain valid image data or a image file name
*/
ReturnCode GrayCode::DecodeCaptureSequence(Capture::Sequence *capture_sequence, dlp::DisparityMap *disparity_map){
    ReturnCode ret;

    // Check the pointers
    if(!capture_sequence || !disparity_map)
        return ret.AddError(STRUCTURED_LIGHT_NULL_POINTER_ARGUMENT);

    // Check that GrayCode object is setup
    if(!this->isSetup())
        return ret.AddError(STRUCTURED_LIGHT_NOT_SETUP);

    // Check that CaptureSequence is not empty
    if(capture_sequence->GetCount() == 0)
        return ret.AddError(STRUCTURED_LIGHT_CAPTURE_SEQUENCE_EMPTY);

    // Check that correct number of images present
    if(capture_sequence->GetCount() != this->sequence_count_total_)
        return ret.AddError(STRUCTURED_LIGHT_CAPTURE_SEQUENCE_SIZE_INVALID);

    // Create a vector of the images to decode
    std::vector<dlp::Image> images_coded;

    // Store the image resolution
    unsigned int image_rows    = 0;
    unsigned int image_columns = 0;

    for(unsigned int iCapture = 0; iCapture < this->sequence_count_total_; iCapture++){
        dlp::Capture capture;
        dlp::Image   image;
        unsigned int capture_rows;
        unsigned int capture_columns;

        // Grab the capture from the sequence
        ReturnCode ret_error = capture_sequence->Get(iCapture, &capture);

        // Check that capture was grabbed
        if(ret_error.hasErrors())
            return ret_error;

        // Check the capture type
        switch(capture.data_type){
        case dlp::Capture::DataType::IMAGE_FILE:
        {
            // Check that the file exists
            if(!dlp::File::Exists(capture.image_file))
                return ret.AddError(FILE_DOES_NOT_EXIST);

            // Load the file and check the resolution
            ret_error = image.Load(capture.image_file);
            if(ret_error.hasErrors())
                return ret_error;

            break;
        }
        case dlp::Capture::DataType::IMAGE_DATA:
        {
            // Check that the image data is not empty
            if(capture.image_data.isEmpty())
                return ret.AddError(IMAGE_EMPTY);

            // Load the file and check the resolution
            cv::Mat temp_image_data;
            capture.image_data.Unsafe_GetOpenCVData(&temp_image_data);
            ret_error = image.Create(temp_image_data);
            if(ret_error.hasErrors())
                return ret_error;
            temp_image_data.release();
            break;
        }
        case dlp::Capture::DataType::INVALID:
        default:
            return ret.AddError(STRUCTURED_LIGHT_DATA_TYPE_INVALID);
        }

        // Get the image resolution
        image.GetColumns(&capture_columns);
        image.GetRows(&capture_rows);

        // If on the first capture store the resolution
        if(iCapture == 0){
            image_columns = capture_columns;
            image_rows    = capture_rows;
        }

        // Check that each image has the same resolution
        if( (capture_rows    != image_rows) ||
            (capture_columns != image_columns))
            return ret.AddError(STRUCTURED_LIGHT_PATTERN_SIZE_INVALID);

        // Convert the image to monochrome
        image.ConvertToMonochrome();

        // Add the image to the list
        images_coded.push_back(image);

        // Clear the image
        image.Clear();
    }

    // All images from the CaptureSequence have been loaded

    // Allocate memory for the disparity images
    ret = this->disparity_map_.Create( image_columns, image_rows, this->pattern_orientation_.Get());

    if(ret.hasErrors()){
        std::cout << "Disparity map create failed..." << std::endl;
        return ret;
    }

    // Check is the inverted patterns are included
    unsigned int image_increment;
    unsigned int image_start;
    unsigned int pattern_loop_count;
    unsigned int threshold = this->pixel_threshold_.Get();

    dlp::Image image_albedo;
    if(this->include_inverted_.Get()){
        // Each "Pattern" has a normal and an inverted pattern (i.e. 2 images per pattern)
        image_increment = 2;
        image_start     = 0;
        pattern_loop_count = this->sequence_count_.Get();
    }
    else{
        // Each "Pattern" only has a normal pattern (i.e. 1 images per pattern)
        image_increment = 1;
        image_start     = 2;
        pattern_loop_count = this->sequence_count_total_;

        // Grab the max value and min value patterns
        dlp::Image image_max;
        dlp::Image image_min;
        cv::Mat    temp_image_data;

        // Grab the max value image
        images_coded.at(0).Unsafe_GetOpenCVData(&temp_image_data);
        image_max.Create(temp_image_data);
        temp_image_data.release();

        // Grab the min value image
        images_coded.at(1).Unsafe_GetOpenCVData(&temp_image_data);
        image_min.Create(temp_image_data);
        temp_image_data.release();

        // Find the albedo thresholds for each pixel
        image_albedo.Create(image_columns, image_rows,dlp::Image::Format::MONO_UCHAR);

        unsigned char pixel_max;
        unsigned char pixel_min;
        for(     unsigned int yRow = 0; yRow < image_rows;    yRow++){
            for( unsigned int xCol = 0; xCol < image_columns; xCol++){

                // Get the pixel from the normal and inverted image
                image_max.Unsafe_GetPixel(xCol, yRow, &pixel_max);
                image_min.Unsafe_GetPixel(xCol, yRow, &pixel_min);

                // Check that the difference is positive and large enough
                if( pixel_max >= (pixel_min + threshold)){
                //if( pixel_max > pixel_min){
                    //Save the albedo threshold
                    image_albedo.Unsafe_SetPixel(xCol,yRow, (unsigned char) ((pixel_max + pixel_min) / 2) );
                }
                else{
                    //Set the disparity map pixel to invalid
                    this->disparity_map_.Unsafe_SetPixel(xCol,yRow,dlp::DisparityMap::INVALID_PIXEL);
                    image_albedo.Unsafe_SetPixel(xCol,yRow, (unsigned char) 255);
                }
            }
        }
    }

    // Calculate the value the MSB pattern
    unsigned int pattern_value  = this->msb_pattern_value_;
    unsigned int kImage         = image_start;
    int             disparity_value;
    unsigned char   normal_value;
    unsigned char   inverted_value;
    unsigned char   albedo_value;

    cv::Mat      temp_image_data;

    for(unsigned int iPattern = image_start; iPattern < pattern_loop_count; iPattern++){
        dlp::Image image_normal;
        dlp::Image image_inverted;

        // Copy the image
        images_coded.at(kImage).Unsafe_GetOpenCVData(&temp_image_data);
        image_normal.Create(temp_image_data);
        temp_image_data.release();
        images_coded.at(kImage).Clear();

        // If inverted is included load the next image, if not, set to zero
        if(this->include_inverted_.Get()){
            // Add the inverted image and convert it to monochrome
            images_coded.at(kImage+1).Unsafe_GetOpenCVData(&temp_image_data);
            image_inverted.Create(temp_image_data);
            temp_image_data.release();
            images_coded.at(kImage+1).Clear();
        }

        // Decode each pixel
        for(     unsigned int yRow = 0; yRow < image_rows;    yRow++){
            for( unsigned int xCol = 0; xCol < image_columns; xCol++){

                // Get the current disparity pixel's value
                this->disparity_map_.Unsafe_GetPixel(xCol,yRow,&disparity_value);

                // Check that point is not invalid
                if(disparity_value != dlp::DisparityMap::INVALID_PIXEL){
                    int pixel_pattern_code;

                    // If the disparity pixel value is empty set it to zero
                    if(disparity_value == dlp::DisparityMap::EMPTY_PIXEL)
                        disparity_value = 0;

                    // Get the pixel from the normal
                    image_normal.Unsafe_GetPixel(   xCol, yRow, &normal_value);

                    if(this->include_inverted_.Get()){
                        int difference;

                        // Grab the inverted image pixel
                        image_inverted.Unsafe_GetPixel( xCol, yRow, &inverted_value);

                        // Calculate the difference
                        difference = (int)normal_value - (int)inverted_value;

                        // Check that the difference is positive and meets the threshold requirement
                        if(difference > 0){
                            pixel_pattern_code = pattern_value;
                        }
                        else{
                            pixel_pattern_code = 0;
                            difference = -difference;
                        }

                        if( difference >= (int) threshold){
                            // Calculate the new disparity value
                            disparity_value |= pixel_pattern_code ^ ((disparity_value >> 1) & pattern_value);
                        }
                        else{
                            // Set the disparity pixel as invalid
                            disparity_value = dlp::DisparityMap::INVALID_PIXEL;
                        }
                    }
                    else{
                        int difference;

                        // Use the albedo image instead of an inverted pattern
                        image_albedo.Unsafe_GetPixel(xCol,yRow, &albedo_value);

                        // Calculate the difference
                        difference = (int)normal_value - (int)albedo_value;

                        // Check that the difference is positive and meets the threshold requirement
                        if(difference > 0){
                            pixel_pattern_code = pattern_value;
                        }
                        else{
                            pixel_pattern_code = 0;
                            difference = -difference;
                        }

                        if( difference > (int) threshold){
                            // Calculate the new disparity value
                            disparity_value |= pixel_pattern_code ^ ((disparity_value >> 1) & pattern_value);
                        }
                        else{
                            // Set the disparity pixel as invalid
                            disparity_value = dlp::DisparityMap::INVALID_PIXEL;
                        }

                    }

                    // Save the adjusted disparity value
                    this->disparity_map_.Unsafe_SetPixel(xCol,yRow,disparity_value);
                }
            }
        }

        // Shift the pattern value
        pattern_value = pattern_value >> 1;

        // Clear the images
        image_normal.Clear();
        image_inverted.Clear();

        // Increment kImage
        kImage = kImage + image_increment;
    }

    // If there is an offset remove it
    if(this->offset_ > 0){
        for(     unsigned int yRow = 0; yRow < image_rows;    yRow++){
            for( unsigned int xCol = 0; xCol < image_columns; xCol++){

                // Get the current disparity pixel's value
                this->disparity_map_.Unsafe_GetPixel(xCol,yRow,&disparity_value);

                // Check that the pixel is still valid
                if(disparity_value != dlp::DisparityMap::INVALID_PIXEL){

                    // Subtract the offset from to correct for the resolution
                    disparity_value = disparity_value - this->offset_;

                    // Check that value is not above resolution and at least zero
                    if((disparity_value >= (int) this->resolution_) || (disparity_value < 0))
                        disparity_value = dlp::DisparityMap::INVALID_PIXEL;
                }

                // Save the adjusted disparity value
                this->disparity_map_.Unsafe_SetPixel(xCol,yRow,disparity_value);
            }
        }
    }


    // Copy the disparity map to the pointer
    ret = disparity_map->Create(this->disparity_map_);

    temp_image_data.release();

    return ret;
}

/** @brief      Retrieves module settings
 *  @param[in]  settings Pointer to return settings
 *  @retval     STRUCTURED_LIGHT_NULL_POINTER_ARGUMENT  Input argument is NULL
 */
ReturnCode GrayCode::GetSetup( dlp::Parameters *settings)const{
    ReturnCode ret;

    if(!settings)
        return ret.AddError(STRUCTURED_LIGHT_NULL_POINTER_ARGUMENT);

    settings->Set(this->sequence_count_);
    settings->Set(this->include_inverted_);
    settings->Set(this->pattern_rows_);
    settings->Set(this->pattern_columns_);
    settings->Set(this->pattern_color_);
    settings->Set(this->pattern_orientation_);
    settings->Set(this->pixel_threshold_);

    return ret;
}

}
