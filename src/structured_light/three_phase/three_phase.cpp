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
#include <structured_light/three_phase/three_phase.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{

/** @brief Constructs object */
ThreePhase::ThreePhase(){
    this->debug_.SetName("STRUCTURED_LIGHT_THREE_PHASE(" + dlp::Number::ToString(this)+ "): ");
    this->debug_.Msg("Constructing object...");
    this->is_setup_ = false;
    this->disparity_map_.Clear();

    this->frequency_.Set(2.0);
    //this->period_pixels_.Set(10);
    this->bitdepth_.Set(dlp::Pattern::Bitdepth::MONO_8BPP);
    this->use_hybrid_.Set(true);
    this->pattern_color_.Set(dlp::Pattern::Color::WHITE);
    this->pattern_orientation_.Set(dlp::Pattern::Orientation::VERTICAL);

    this->debug_.Msg("Object constructed");
}

/** @brief Destroys object and deallocates memory */
ThreePhase::~ThreePhase(){
    this->debug_.Msg("Deconstructing object...");
    this->disparity_map_.Clear();
    this->debug_.Msg("Object deconstructed");
}

/** @brief Retrieves settings from \ref dlp::Parameters object to configure
 *  \ref dlp::Pattern::Sequence generation and \ref dlp::Capture::Sequence decoding
 *  @param[in]  settings    \ref dlp::Parameters object to retrieve settings from
 *
 * @retval STRUCTURED_LIGHT_SETTINGS_PATTERN_COLOR_MISSING              \ref dlp::Parameters list missing \ref dlp::StructuredLight::pattern_color_
 * @retval STRUCTURED_LIGHT_SETTINGS_PATTERN_ROWS_MISSING               \ref dlp::Parameters list missing \ref dlp::StructuredLight::pattern_rows_
 * @retval STRUCTURED_LIGHT_SETTINGS_PATTERN_COLUMNS_MISSING            \ref dlp::Parameters list missing \ref dlp::StructuredLight::pattern_columns_
 * @retval STRUCTURED_LIGHT_SETTINGS_PATTERN_ORIENTATION_MISSING        \ref dlp::Parameters list missing \ref dlp::StructuredLight::pattern_orientation_
 */
ReturnCode ThreePhase::Setup(const dlp::Parameters &settings){
    ReturnCode ret;

    if(!this->projector_set_){
        if(settings.Get(&this->pattern_rows_).hasErrors())
            return ret.AddError(STRUCTURED_LIGHT_SETTINGS_PATTERN_ROWS_MISSING);

       if((settings.Get(&this->pattern_columns_).hasErrors()))
            return ret.AddError(STRUCTURED_LIGHT_SETTINGS_PATTERN_COLUMNS_MISSING);
    }

    ret = settings.Get(&this->over_sample_);
    if(this->over_sample_.Get() < 1) this->over_sample_.Set(1);


    ret = settings.Get(&this->pattern_orientation_);
    if(ret.hasErrors())
        return ret.AddError(STRUCTURED_LIGHT_SETTINGS_PATTERN_ORIENTATION_MISSING);

    ret = settings.Get(&this->pattern_color_);
    if(ret.hasErrors())
        return ret.AddError(STRUCTURED_LIGHT_SETTINGS_PATTERN_COLOR_MISSING);

    ret = settings.Get(&this->bitdepth_);
    if(ret.hasErrors())
        return ret.AddError(THREE_PHASE_BITDEPTH_MISSING);

    switch(this->bitdepth_.Get()){
        case dlp::Pattern::Bitdepth::MONO_8BPP:
            this->maximum_value_ = 255;
            break;
        case dlp::Pattern::Bitdepth::MONO_7BPP:
            this->maximum_value_ = 127;
            break;
        case dlp::Pattern::Bitdepth::MONO_6BPP:
            this->maximum_value_ = 63;
            break;
        case dlp::Pattern::Bitdepth::MONO_5BPP:
            this->maximum_value_ = 31;
            break;
        default:
            return ret.AddError(THREE_PHASE_BITDEPTH_TOO_SMALL);
    }

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

    ret = settings.Get(&this->pixels_per_period_);
    if(ret.hasErrors())
        return ret.AddError(THREE_PHASE_PIXELS_PER_PERIOD_MISSING);

    this->frequency_.Set( (float) this->resolution_ / this->pixels_per_period_.Get());
    this->phase_counts_ = this->frequency_.Get() * 2;

    // Store the number of three phase patterns
    this->sequence_count_total_ = 3;

    ret = settings.Get(&this->use_hybrid_);
    if(ret.hasErrors())
        return ret.AddError(THREE_PHASE_USE_HYBRID_UNWRAP_MISSING);

    if(settings.Contains(this->repeat_phases_))
        settings.Get(&this->repeat_phases_);

    if(!this->use_hybrid_.Get()){
        //this->sequence_count_total_ = 3;
        return ret.AddError(THREE_PHASE_ONLY_HYBRID_UNWRAP_SUPPORTED);
    }
    else{
        // Check that the pixels per period is divisible by 8 since
        // there will be 8 regions per period. If the region size
        // is not a whole number the GrayCode module will return an error
        if((this->pixels_per_period_.Get() < 8) ||
          ((this->pixels_per_period_.Get() % 8) > 0))
            return ret.AddError(THREE_PHASE_PIXELS_PER_PERIOD_NOT_DIVISIBLE_BY_EIGHT);

        // Calculate the number of binary patterns needed
        // Multiplying the phase counts times four allows the decoding
        // to correct for slightly shifted values that may be misclassified
        // during the unwrapping process
        this->hybrid_region_count_.Set(float(this->phase_counts_*4));

        // Check for additional GrayCode module parameters in settings
        settings.Get(&this->hybrid_include_inverted_);
        settings.Get(&this->hybrid_pixel_threshold_);

        // Setup the GrayCode module for hybrid unwrapping
        dlp::Parameters hybrid_settings;
        hybrid_settings.Set(this->pattern_columns_);
        hybrid_settings.Set(this->pattern_rows_);
        hybrid_settings.Set(this->pattern_orientation_);
        hybrid_settings.Set(this->pattern_color_);
        hybrid_settings.Set(this->hybrid_region_count_);
        hybrid_settings.Set(this->hybrid_include_inverted_);
        hybrid_settings.Set(this->hybrid_pixel_threshold_);

        ret = this->hybrid_unwrap_module_.Setup(hybrid_settings);
        if(ret.hasErrors())
            return ret.AddError(THREE_PHASE_HYBRID_UNWRAP_MODULE_SETUP_FAILED);

        // Add the number of patterns from the hybrid unwrap module to
        // the total sequence pattern count
        this->sequence_count_total_ = (3*this->repeat_phases_.Get()) + this->hybrid_unwrap_module_.GetTotalPatternCount();
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
ReturnCode ThreePhase::GeneratePatternSequence(Pattern::Sequence *pattern_sequence){
    ReturnCode ret;

    // Check that ThreePhase object is setup
    if((!this->isSetup()) || !this->hybrid_unwrap_module_.isSetup())
        return ret.AddError(STRUCTURED_LIGHT_NOT_SETUP);

    // Check that argument is not null
    if(!pattern_sequence)
        return ret.AddError(STRUCTURED_LIGHT_NULL_POINTER_ARGUMENT);

    // Generate the three phase code values
    std::vector< unsigned char > sine_phase_value_0;
    std::vector< unsigned char > sine_phase_value_p120;
    std::vector< unsigned char > sine_phase_value_n120;

    // Clear the pattern sequence
    pattern_sequence->Clear();

    // First create a set of binary patterns
    sine_phase_value_0.clear();
    sine_phase_value_p120.clear();
    sine_phase_value_n120.clear();
    float period_pixels = ((float)this->resolution_) / this->frequency_.Get();
    float angular_frequency = 2 * THREE_PHASE_PI / period_pixels;
    float amplitude = this->maximum_value_/2;
    float offset    = amplitude;    // Sets minimum value to zero

    for(unsigned int iPoint = 0; iPoint < this->resolution_; iPoint++){
        sine_phase_value_0.push_back(    lroundf( amplitude * sin(  angular_frequency * ((float)iPoint) ) + offset ) );
        sine_phase_value_p120.push_back( lroundf( amplitude * sin( (angular_frequency * ((float)iPoint) ) + THREE_PHASE_TWO_THIRDS_PI) + offset ) );
        sine_phase_value_n120.push_back( lroundf( amplitude * sin( (angular_frequency * ((float)iPoint) ) - THREE_PHASE_TWO_THIRDS_PI) + offset ) );
    }

    // Get the image resolution
    unsigned int rows     = this->pattern_rows_.Get();
    unsigned int columns  = this->pattern_columns_.Get();

    // Create the three phase images
    dlp::Image sine_phase_image_0;
    dlp::Image sine_phase_image_p120;
    dlp::Image sine_phase_image_n120;

    sine_phase_image_0.Create(    columns, rows, dlp::Image::Format::MONO_UCHAR );
    sine_phase_image_p120.Create( columns, rows, dlp::Image::Format::MONO_UCHAR );
    sine_phase_image_n120.Create( columns, rows, dlp::Image::Format::MONO_UCHAR );


    switch(this->pattern_orientation_.Get()){
    case dlp::Pattern::Orientation::VERTICAL:
        for(     unsigned int yRow = 0; yRow < rows;    yRow++){
            for( unsigned int xCol = 0; xCol < columns; xCol++){
                sine_phase_image_0.Unsafe_SetPixel(    xCol, yRow, sine_phase_value_0.at(xCol) );
                sine_phase_image_p120.Unsafe_SetPixel( xCol, yRow, sine_phase_value_p120.at(xCol) );
                sine_phase_image_n120.Unsafe_SetPixel( xCol, yRow, sine_phase_value_n120.at(xCol) );
            }
        }

        break;
    case dlp::Pattern::Orientation::HORIZONTAL:
        for(     unsigned int yRow = 0; yRow < rows;    yRow++){
            for( unsigned int xCol = 0; xCol < columns; xCol++){
                sine_phase_image_0.Unsafe_SetPixel(    xCol, yRow, sine_phase_value_0.at(yRow) );
                sine_phase_image_p120.Unsafe_SetPixel( xCol, yRow, sine_phase_value_p120.at(yRow) );
                sine_phase_image_n120.Unsafe_SetPixel( xCol, yRow, sine_phase_value_n120.at(yRow) );
            }
        }
        break;
    case dlp::Pattern::Orientation::DIAMOND_ANGLE_2:
        // Set the pixels
        for(     unsigned int yRow = 0; yRow < rows;    yRow++){
            for( unsigned int xCol = 0; xCol < columns; xCol++){
                unsigned int code = ((rows - yRow)/2) + xCol;

                // Save the pixel value
                sine_phase_image_0.Unsafe_SetPixel(    xCol, yRow, sine_phase_value_0.at(code) );
                sine_phase_image_p120.Unsafe_SetPixel( xCol, yRow, sine_phase_value_p120.at(code) );
                sine_phase_image_n120.Unsafe_SetPixel( xCol, yRow, sine_phase_value_n120.at(code) );
            }
        }
        break;
    case dlp::Pattern::Orientation::DIAMOND_ANGLE_1:
        // Set the pixels
        for(     unsigned int yRow = 0; yRow < rows;    yRow++){
            for( unsigned int xCol = 0; xCol < columns; xCol++){
                unsigned int code = (yRow/2) + xCol;

                // Save the pixel value
                sine_phase_image_0.Unsafe_SetPixel(    xCol, yRow, sine_phase_value_0.at(code) );
                sine_phase_image_p120.Unsafe_SetPixel( xCol, yRow, sine_phase_value_p120.at(code) );
                sine_phase_image_n120.Unsafe_SetPixel( xCol, yRow, sine_phase_value_n120.at(code) );
            }
        }
        break;
    case dlp::Pattern::Orientation::INVALID:
    default:
        return ret.AddError(STRUCTURED_LIGHT_NOT_SETUP);
        break;
    }

    // Create the patterns
    dlp::Pattern sine_phase_pattern_0;
    dlp::Pattern sine_phase_pattern_p120;
    dlp::Pattern sine_phase_pattern_n120;

    sine_phase_pattern_0.bitdepth  = this->bitdepth_.Get();
    sine_phase_pattern_0.color     = this->pattern_color_.Get();
    sine_phase_pattern_0.data_type = dlp::Pattern::DataType::IMAGE_DATA;
    sine_phase_pattern_0.image_data.Create(sine_phase_image_0);

    sine_phase_pattern_p120.bitdepth  = this->bitdepth_.Get();
    sine_phase_pattern_p120.color     = this->pattern_color_.Get();
    sine_phase_pattern_p120.data_type = dlp::Pattern::DataType::IMAGE_DATA;
    sine_phase_pattern_p120.image_data.Create(sine_phase_image_p120);

    sine_phase_pattern_n120.bitdepth  = this->bitdepth_.Get();
    sine_phase_pattern_n120.color     = this->pattern_color_.Get();
    sine_phase_pattern_n120.data_type = dlp::Pattern::DataType::IMAGE_DATA;
    sine_phase_pattern_n120.image_data.Create(sine_phase_image_n120);

    // Add the patterns to the return sequence
    for(unsigned int iCount = 0; iCount < this->repeat_phases_.Get();iCount++){
        pattern_sequence->Add(sine_phase_pattern_0);
    }
    for(unsigned int iCount = 0; iCount < this->repeat_phases_.Get();iCount++){
        pattern_sequence->Add(sine_phase_pattern_p120);
    }
    for(unsigned int iCount = 0; iCount < this->repeat_phases_.Get();iCount++){
        pattern_sequence->Add(sine_phase_pattern_n120);
    }

    // Clear the images
    sine_phase_image_0.Clear();
    sine_phase_image_p120.Clear();
    sine_phase_image_n120.Clear();

    // Generate the hybrid GrayCode patterns
    dlp::Pattern::Sequence hybrid_sequence;
    ret = this->hybrid_unwrap_module_.GeneratePatternSequence(&hybrid_sequence);
    if(ret.hasErrors()) return ret;

    // Add the GrayCode patterns to the return sequence
    pattern_sequence->Add(hybrid_sequence);

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
ReturnCode ThreePhase::DecodeCaptureSequence(Capture::Sequence *capture_sequence, dlp::DisparityMap *disparity_map){
    ReturnCode ret;

    // Check the pointers
    if(!capture_sequence || !disparity_map)
        return ret.AddError(STRUCTURED_LIGHT_NULL_POINTER_ARGUMENT);

    // Check that ThreePhase object is setup
    if(!this->isSetup() || !this->hybrid_unwrap_module_.isSetup())
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

    // Seperate the GrayCode captures
    dlp::Capture::Sequence gray_code_sequence;

    // Check and copy the image data for the three phase patterns
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

        // The first three captures are the sinusoidal patterns
        if(iCapture < (3*this->repeat_phases_.Get())){

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
        else{
            // Add all GrayCode captures to sequence for seperate decoding
            gray_code_sequence.Add(capture);
        }
    }

    // Decode the GrayCode sequence
    dlp::DisparityMap gray_code_disparity;
    ret = this->hybrid_unwrap_module_.DecodeCaptureSequence(&gray_code_sequence,&gray_code_disparity);

    if(ret.hasErrors())
        return ret;

    // Check the resolution of the GrayCode disparity map
    unsigned int gray_code_disparity_rows;
    unsigned int gray_code_disparity_columns;
    gray_code_disparity.GetColumns(&gray_code_disparity_columns);
    gray_code_disparity.GetRows(&gray_code_disparity_rows);

    if((gray_code_disparity_columns != image_columns) ||
       (gray_code_disparity_rows    != image_rows))
        return ret.AddError(STRUCTURED_LIGHT_PATTERN_SIZE_INVALID);

    // Allocate memory for the disparity map
    ret = this->disparity_map_.Create( image_columns, image_rows, this->pattern_orientation_.Get(),this->over_sample_.Get());

    if(ret.hasErrors())
        return ret;


    // Decode each pixel
    int   disparity_value;
    int   gray_code_disparity_value;
    float phase_value;
    unsigned char intensity_0;
    unsigned char intensity_p120;
    unsigned char intensity_n120;

    float over_sample = float(this->over_sample_.Get());

    for(     unsigned int yRow = 0; yRow < image_rows;    yRow++){
        for( unsigned int xCol = 0; xCol < image_columns; xCol++){

            // Get the sinusoidal intensity values
            float intensity_phase_0    = 0;
            float intensity_phase_p120 = 0;
            float intensity_phase_n120 = 0;

            for(unsigned int iCount = 0; iCount < this->repeat_phases_.Get();iCount++){
                unsigned char temp_0    = 0;
                unsigned char temp_p120 = 0;
                unsigned char temp_n120 = 0;

                unsigned char image_0    = (this->repeat_phases_.Get()*0) + iCount;
                unsigned char image_p120 = (this->repeat_phases_.Get()*1) + iCount;
                unsigned char image_n120 = (this->repeat_phases_.Get()*2) + iCount;

                images_coded.at(image_0   ).Unsafe_GetPixel(xCol,yRow,&temp_0);
                images_coded.at(image_p120).Unsafe_GetPixel(xCol,yRow,&temp_p120);
                images_coded.at(image_n120).Unsafe_GetPixel(xCol,yRow,&temp_n120);

                intensity_phase_0 += temp_0;
                intensity_phase_p120 += temp_p120;
                intensity_phase_n120 += temp_n120;

            }

            intensity_phase_0    = intensity_phase_0    / this->repeat_phases_.Get();
            intensity_phase_p120 = intensity_phase_p120 / this->repeat_phases_.Get();
            intensity_phase_n120 = intensity_phase_n120 / this->repeat_phases_.Get();


//            images_coded.at(0).Unsafe_GetPixel(xCol,yRow,&intensity_0);
//            images_coded.at(1).Unsafe_GetPixel(xCol,yRow,&intensity_p120);
//            images_coded.at(2).Unsafe_GetPixel(xCol,yRow,&intensity_n120);

            // Calculate the wrapped phase
            phase_value = atan( sqrt(3.0) * (intensity_phase_n120 - intensity_phase_p120) /
                               (2.0*(intensity_phase_0)-(intensity_phase_n120)-(intensity_phase_p120)) )
                                / THREE_PHASE_PI;

            if((phase_value >= 0.5) || (phase_value <= -0.5)){
                // Pixel is invalid
                disparity_value = dlp::DisparityMap::INVALID_PIXEL;
            }
            else{
                // Convert the phase to a wrapped pixel value
                disparity_value = lroundf(over_sample*(phase_value + 0.5) * ((float)this->resolution_) / this->phase_counts_);

                if(this->use_hybrid_.Get()){
                    // Get the gray code disparity pixel value
                    gray_code_disparity.Unsafe_GetPixel(xCol,yRow,&gray_code_disparity_value);

                    //disparity_vals[(unsigned int)gray_code_disparity_value]++;

                    if((gray_code_disparity_value != dlp::DisparityMap::INVALID_PIXEL) &&
                       (gray_code_disparity_value != dlp::DisparityMap::EMPTY_PIXEL)){


                        // Check that the phase change regions are correct
                        if(((gray_code_disparity_value+1) % 4) == 0){
                            // The fourth region of a period should be greater than
                            // 0.25 and absolutely greater than 0. If it is less than
                            // zero the phase has been missclassified
                            if(phase_value < 0) gray_code_disparity_value++;
                        }
                        else if(((gray_code_disparity_value+1) % 4) == 1){
                            // The first region of a period should be less than
                            // -0.25 and absolutely less than 0. If it is greater than
                            // zero the phase has been missclassified
                            if(phase_value > 0) gray_code_disparity_value--;
                        }

                        // Adjust the GrayCode disparity value to the phase regions
                        gray_code_disparity_value = gray_code_disparity_value / 4;

                        // Add the GrayCode disparity value to unwrap the values
                        disparity_value += (over_sample*gray_code_disparity_value*this->resolution_/this->phase_counts_);
                    }
                    else{
                        disparity_value = dlp::DisparityMap::INVALID_PIXEL;
                    }
                }
                else{
                    // Non hybrid method not implemented
                    disparity_value = dlp::DisparityMap::INVALID_PIXEL;
                }
            }

            // Save the calculated pixel value
            this->disparity_map_.Unsafe_SetPixel(xCol,yRow,disparity_value);
        }
    }

//    std::ofstream myfile;
//      myfile.open ("disparity.txt");
//      for(unsigned int i=0;i<10000;i++){
//          myfile << i << ", " <<  disparity_vals[i];
//      }
//      myfile.close();

    // Copy the disparity map to the pointer
    ret = disparity_map->Create(this->disparity_map_);

    return ret;
}

/** @brief      Retrieves module settings
 *  @param[in]  settings Pointer to return settings
 *  @retval     STRUCTURED_LIGHT_NULL_POINTER_ARGUMENT  Input argument is NULL
 */
ReturnCode ThreePhase::GetSetup( dlp::Parameters *settings)const{
    ReturnCode ret;

    if(!settings)
        return ret.AddError(STRUCTURED_LIGHT_NULL_POINTER_ARGUMENT);

    settings->Set(this->pattern_rows_);
    settings->Set(this->pattern_columns_);
    settings->Set(this->pattern_color_);
    settings->Set(this->pattern_orientation_);
    settings->Set(this->frequency_);
    settings->Set(this->bitdepth_);
    settings->Set(this->use_hybrid_);

    if(this->use_hybrid_.Get()){
        settings->Set(this->hybrid_region_count_);
        settings->Set(this->hybrid_include_inverted_);
        settings->Set(this->hybrid_pixel_threshold_);
    }

    return ret;
}

}
