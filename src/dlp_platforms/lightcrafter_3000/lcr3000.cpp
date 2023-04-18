/** \file       lcr3000.cpp
 *  @brief      Contains definitions for the DLP SDK LightCrafter 3000 class
 *  \copyright  2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#define ASIO_STANDALONE
#include <string>
#include <asio.hpp>

#include <common/returncode.hpp>
#include "dlp_platforms/lightcrafter_3000/lcr3000.hpp"
#include "dlp_platforms/lightcrafter_3000/lcr3000_definitions.hpp"

namespace dlp{

LCr3000::LCr3000(){

    //Default IP Adress and Port numbers
    std::string ip = "192.168.1.100";
    unsigned long int port = 0x5555;

    //Set the debug name
    this->debug_.SetName("LCR3000_DEBUG: ");

    //Set the platform
    this->SetPlatform(Platform::LIGHTCRAFTER_3000);

    this->ip = ip;
    this->port = port;


    this->previous_sequence_start_ = 0;
    this->previous_sequence_patterns_ = 0;

    this->debug_.Msg(1,"Object Constructed");
}

LCr3000::~LCr3000(){

    this->io_service.stop();
}

ReturnCode LCr3000::Connect(std::string id){
    ReturnCode ret;

    /* Open tcp connection with LCr */
    try{

        std::size_t delimiter = id.find_first_of(":");

        // Check that the delimiter exists
        if(delimiter == std::string::npos){
            ret.AddError(LCR3000_INVALID_IP_PORT_NUM);
            return ret;
        }

        std::string ip_address = dlp::String::Trim(id.substr(0,delimiter));

        std::string temp_string = id.substr(delimiter+1);
        if(temp_string.size() <= 0) {
            ret.AddError(LCR3000_INVALID_IP_PORT_NUM);
            return ret;
        }

        std::string port_num = dlp::String::Trim(id.substr(delimiter+1));

        if(ip_address.empty ()|| port_num.empty()){
            ret.AddError(LCR3000_INVALID_IP_PORT_NUM);
            return ret;
        }

        //std::cout << "ip_address - " << ip_address << " port_num - " << std::stoul(port_num,nullptr,0) << std::endl;

        this->ip = ip_address;
        this->port = std::stoul(port_num,nullptr,0);

        this->LCR_PKT_Socket = TCP_Connect(this->ip,this->port);
    }

    catch(...){
    //     if((this->LCR_PKT_Socket == NULL) ||
    //        (this->LCR_PKT_Socket < 0))
    //     {
    //         ret.AddError(LCR3000_COULD_NOT_CONNECT);
    //         return ret;
    //     }

    // }

    // if((this->LCR_PKT_Socket == NULL) ||
    //    (this->LCR_PKT_Socket < 0))
    // {
    //     ret.AddError(LCR3000_COULD_NOT_CONNECT);
    //     return ret;
    // }

        if((this->LCR_PKT_Socket == NULL))
        {
            ret.AddError(LCR3000_COULD_NOT_CONNECT);
            return ret;
        }

    }

    if((this->LCR_PKT_Socket == NULL))
    {
        ret.AddError(LCR3000_COULD_NOT_CONNECT);
        return ret;
    }


    this->is_connected_ = true;

    this->StopPatternSequence();

    return ret;
}

ReturnCode LCr3000::Disconnect(){
    ReturnCode ret;

    this->StopPatternSequence();

    TCP_Disconnect(this->LCR_PKT_Socket);

    this->is_connected_ = false;

    return ret;
}

bool LCr3000::isConnected() const{
    return this->is_connected_;
}

ReturnCode LCr3000::Setup(const dlp::Parameters &settings){
    ReturnCode ret;

    if(!this->isConnected()) return ret.AddError(LCR3000_NOT_CONNECTED);


    ret = this->LCR_CMD_SetDisplayMode(dlp::LCr3000::DISP_MODE_PTN_SEQ);
    if(ret.hasErrors()) return ret;

    this->StopPatternSequence();

    if(settings.Contains(this->sequence_exposure_))
        settings.Get(&this->sequence_exposure_);

    if(settings.Contains(this->sequence_period_))
        settings.Get(&this->sequence_period_);

    if(settings.Contains(this->led_current_red_)   ||
       settings.Contains(this->led_current_green_) ||
       settings.Contains(this->led_current_blue_)){

        // Retrieve the values
        settings.Get(&this->led_current_red_);
        settings.Get(&this->led_current_green_);
        settings.Get(&this->led_current_blue_);

        LCR_LEDCurrent_t currents;
        currents.Red    = this->led_current_red_.Get();
        currents.Green  = this->led_current_green_.Get();
        currents.Blue   = this->led_current_blue_.Get();

        // Check for max values //////////////////////////////////////////////////////////

        // Send the values
        ret = this->LCR_CMD_SetLEDCurrent(&currents);
        if(ret.hasErrors()) return ret;
    }

    if(settings.Contains(this->image_flip_short_axis_) ||
       settings.Contains(this->image_flip_long_axis_)){
        settings.Get(&this->image_flip_short_axis_);
        settings.Get(&this->image_flip_long_axis_);

        LCR_DisplaySetting_t disp_setting;

        disp_setting.Rotate = 0x00; //No rotate
        disp_setting.ShortAxisFlip = this->image_flip_short_axis_.Get() & 0xFF;
        disp_setting.LongAxisFlip = this->image_flip_long_axis_.Get() & 0xFF;

        ret = LCR_CMD_SetDisplaySetting(&disp_setting);
        if(ret.hasErrors()) return ret;
    }

    if(settings.Contains(this->trigger_input_type_))
        settings.Get(&this->trigger_input_type_);

    if(settings.Contains(this->trigger_input_delay_))
        settings.Get(&this->trigger_input_delay_);

    if(settings.Contains(this->trigger_output_enable_))
        settings.Get(&this->trigger_output_enable_);

    if(settings.Contains(this->trigger_output_polarity_))
        settings.Get(&this->trigger_output_polarity_);

    if(settings.Contains(this->trigger_output_pulse_width_))
        settings.Get(&this->trigger_output_pulse_width_);

    if(settings.Contains(this->trigger_output_delay_))
        settings.Get(&this->trigger_output_delay_);


    // Setup the output trigger
    dlp::LCr3000::LCR_CamTriggerSetting_t trigger_settings;
    trigger_settings.Delay      = this->trigger_output_delay_.Get();
    trigger_settings.Enable     = this->trigger_output_enable_.Get();
    trigger_settings.Polarity   = this->trigger_output_polarity_.Get();
    trigger_settings.PulseWidth = this->trigger_output_pulse_width_.Get();
    trigger_settings.Source = 0 ;
    ret = this->LCR_CMD_SetCamTriggerSetting(&trigger_settings);
    if(ret.hasErrors()) return ret;

    if(settings.Contains(this->pattern_led_select_)){
        settings.Get(&this->pattern_led_select_);

        switch(this->pattern_led_select_.Get()){
        case dlp::Pattern::Color::RED:
            this->lcr3000_led_select_ = dlp::LCr3000::LED_RED;
            break;
        case dlp::Pattern::Color::GREEN:
            this->lcr3000_led_select_ = dlp::LCr3000::LED_GREEN;
            break;
        case dlp::Pattern::Color::BLUE:
            this->lcr3000_led_select_ = dlp::LCr3000::LED_BLUE;
            break;
        default:
            this->lcr3000_led_select_ = dlp::LCr3000::LED_GREEN;
            return ret.AddError(PATTERN_COLOR_INVALID); // UPDATE ERROR MESSAGE TO REFLECT THIS IS A LCR3000 limitation
        }

    }

    return ret;
}

ReturnCode LCr3000::GetSetup(dlp::Parameters* settings)const{
    ReturnCode ret;

    if(!settings) return ret.AddError("NULL ARGUMENT");

    settings->Clear();

    settings->Set(this->led_current_red_);
    settings->Set(this->led_current_green_);
    settings->Set(this->led_current_blue_);
    settings->Set(this->image_flip_short_axis_);
    settings->Set(this->image_flip_long_axis_);
    settings->Set(this->trigger_input_type_);
    settings->Set(this->trigger_input_delay_);
    settings->Set(this->trigger_output_enable_);
    settings->Set(this->trigger_output_polarity_);
    settings->Set(this->trigger_output_pulse_width_);
    settings->Set(this->trigger_output_delay_);
    settings->Set(this->pattern_led_select_);
    settings->Set(this->sequence_period_);
    settings->Set(this->sequence_exposure_);

    return ret;
}

ReturnCode LCr3000::ProjectSolidWhitePattern(){
    ReturnCode ret;

    // Check that LCr3000 is connected
    if(!this->isConnected())
        return ret.AddError(LCR3000_NOT_CONNECTED);

    // Set the mode to pattern sequence
    ret = this->LCR_CMD_SetDisplayMode(dlp::LCr3000::DISP_MODE_PTN_SEQ);
    if(ret.hasErrors()) return ret;

    // Setup the sequence settings
    dlp::LCr3000::LCR_PatternSeqSetting_t sequence_settings;
    sequence_settings.BitDepth = 1;
    sequence_settings.NumPatterns = 1;
    sequence_settings.PatternType = dlp::LCr3000::PTN_TYPE_NORMAL;
    sequence_settings.InputTriggerType = (LCR_TriggerType_t)this->trigger_input_type_.Get();
    sequence_settings.InputTriggerDelay = this->trigger_input_delay_.Get();
    sequence_settings.AutoTriggerPeriod = this->sequence_period_.Get();
    sequence_settings.ExposureTime = this->sequence_exposure_.Get();
    sequence_settings.LEDSelect = this->lcr3000_led_select_;
    sequence_settings.Repeat = 1;
    ret = this->LCR_CMD_SetPatternSeqSetting(&sequence_settings);
    if(ret.hasErrors()) return ret;

    // Upload the white BMP file
    this->LCR_CMD_DefinePatternBMP(0,"resources/lcr3000/white.bmp");

    // Start the sequence
    this->LCR_CMD_StartPatternSeq(1);

    // Reset the previous sequence settings since an
    // image has been uploaded
    this->previous_sequence_start_ = 0;
    this->previous_sequence_patterns_ = 0;
    this->previous_sequence_repeat_ = false;

    return ret;
}

ReturnCode LCr3000::ProjectSolidBlackPattern(){
    ReturnCode ret;

    // Check that LCr3000 is connected
    if(!this->isConnected())
        return ret.AddError(LCR3000_NOT_CONNECTED);

    // Set the mode to pattern sequence
    ret = this->LCR_CMD_SetDisplayMode(dlp::LCr3000::DISP_MODE_PTN_SEQ);
    if(ret.hasErrors()) return ret;

    // Setup the sequence settings
    dlp::LCr3000::LCR_PatternSeqSetting_t sequence_settings;
    sequence_settings.BitDepth = 1;
    sequence_settings.NumPatterns = 1;
    sequence_settings.PatternType = dlp::LCr3000::PTN_TYPE_NORMAL;
    sequence_settings.InputTriggerType = (LCR_TriggerType_t)this->trigger_input_type_.Get();
    sequence_settings.InputTriggerDelay = this->trigger_input_delay_.Get();
    sequence_settings.AutoTriggerPeriod = this->sequence_period_.Get();
    sequence_settings.ExposureTime = this->sequence_exposure_.Get();
    sequence_settings.LEDSelect = this->lcr3000_led_select_;
    sequence_settings.Repeat = 1;
    ret = this->LCR_CMD_SetPatternSeqSetting(&sequence_settings);
    if(ret.hasErrors()) return ret;

    // Upload the black BMP file
    this->LCR_CMD_DefinePatternBMP(0,"resources/lcr3000/black.bmp");

    // Start the sequence
    this->LCR_CMD_StartPatternSeq(1);

    // Reset the previous sequence settings since an
    // image has been uploaded
    this->previous_sequence_start_ = 0;
    this->previous_sequence_patterns_ = 0;
    this->previous_sequence_repeat_ = false;

    return ret;
}

ReturnCode LCr3000::PreparePatternSequence(const Pattern::Sequence &arg_pattern_sequence){
    ReturnCode ret;

    // Check that LCr3000 is connected
    if(!this->isConnected())
        return ret.AddError(LCR3000_NOT_CONNECTED);

    // Check that the sequence is NOT empty
    if(arg_pattern_sequence.GetCount() == 0)
        return ret.AddError(PATTERN_SEQUENCE_EMPTY);


    // Clear the previously prepared sequence
    this->prepared_sequence_.Clear();


    // Reset the previous sequence settings
    this->previous_sequence_start_ = 0;
    this->previous_sequence_patterns_ = 0;
    this->previous_sequence_repeat_ = false;

    // Add patterns from supplied sequence into the internal sequence
    // Load image files if necessary and return error if type is parameters
    for(unsigned int iPattern = 0; iPattern < arg_pattern_sequence.GetCount(); iPattern++){
        dlp::Pattern temp;

        // Retrieve pattern
        arg_pattern_sequence.Get(iPattern, &temp);

        // Override the pattern color
        temp.color = this->pattern_led_select_.Get();

        // Check that the bitdepth is monochrome
        if(!((temp.bitdepth == dlp::Pattern::Bitdepth::MONO_1BPP) ||
             (temp.bitdepth == dlp::Pattern::Bitdepth::MONO_2BPP) ||
             (temp.bitdepth == dlp::Pattern::Bitdepth::MONO_3BPP) ||
             (temp.bitdepth == dlp::Pattern::Bitdepth::MONO_4BPP) ||
             (temp.bitdepth == dlp::Pattern::Bitdepth::MONO_5BPP) ||
             (temp.bitdepth == dlp::Pattern::Bitdepth::MONO_6BPP) ||
             (temp.bitdepth == dlp::Pattern::Bitdepth::MONO_7BPP) ||
             (temp.bitdepth == dlp::Pattern::Bitdepth::MONO_8BPP)))
            return ret.AddError(PATTERN_BITDEPTH_INVALID);

        // Check that the exposure and period have been set
        if((temp.exposure == 0) || (temp.period == 0)){
            // Use exposure and period settings from configuration file
            temp.exposure = this->sequence_exposure_.Get();
            temp.period   = this->sequence_period_.Get();
        }

        // Pattern has specified exposure and period
        // Check that they are valid
        // Refer to LightCrafter User's Guide Table 4-3 Internal Exposure Time for reference
        // The Extended Pattern Mode allows displaying any number of patterns and upto 5.0Second
        // Exposure time for bit-depth 2 to 8; for 1-bit it is upto 2.0 second

        // Check the exposure time
        switch (temp.bitdepth) {
        case dlp::Pattern::Bitdepth::MONO_1BPP:
            if(temp.exposure > 2000000 ) return ret.AddError(PATTERN_EXPOSURE_TOO_LONG);
            if(temp.exposure <    250 ) return ret.AddError(PATTERN_EXPOSURE_TOO_SHORT);
            break;
        case dlp::Pattern::Bitdepth::MONO_2BPP:
            if(temp.exposure > 5000000 ) return ret.AddError(PATTERN_EXPOSURE_TOO_LONG);
            if(temp.exposure <   666 ) return ret.AddError(PATTERN_EXPOSURE_TOO_SHORT);
            break;
        case dlp::Pattern::Bitdepth::MONO_3BPP:
            if(temp.exposure > 5000000 ) return ret.AddError(PATTERN_EXPOSURE_TOO_LONG);
            if(temp.exposure <  1470 ) return ret.AddError(PATTERN_EXPOSURE_TOO_SHORT);
            break;
        case dlp::Pattern::Bitdepth::MONO_4BPP:
            if(temp.exposure > 5000000 ) return ret.AddError(PATTERN_EXPOSURE_TOO_LONG);
            if(temp.exposure <  1600 ) return ret.AddError(PATTERN_EXPOSURE_TOO_SHORT);
            break;
        case dlp::Pattern::Bitdepth::MONO_5BPP:
            if(temp.exposure > 5000000 ) return ret.AddError(PATTERN_EXPOSURE_TOO_LONG);
            if(temp.exposure <  2000 ) return ret.AddError(PATTERN_EXPOSURE_TOO_SHORT);
            break;
        case dlp::Pattern::Bitdepth::MONO_6BPP:
            if(temp.exposure > 5000000 ) return ret.AddError(PATTERN_EXPOSURE_TOO_LONG);
            if(temp.exposure <  2500 ) return ret.AddError(PATTERN_EXPOSURE_TOO_SHORT);
            break;
        case dlp::Pattern::Bitdepth::MONO_7BPP:
            if(temp.exposure > 5000000 ) return ret.AddError(PATTERN_EXPOSURE_TOO_LONG);
            if(temp.exposure <  4500 ) return ret.AddError(PATTERN_EXPOSURE_TOO_SHORT);
            break;
        case dlp::Pattern::Bitdepth::MONO_8BPP:
            if(temp.exposure > 5000000 ) return ret.AddError(PATTERN_EXPOSURE_TOO_LONG);
            if(temp.exposure <  8333 ) return ret.AddError(PATTERN_EXPOSURE_TOO_SHORT);
            break;
        default:
            return ret.AddError(PATTERN_BITDEPTH_INVALID);
        }

        // Check the period time
        if(temp.period < temp.exposure ) return ret.AddError(PATTERN_PERIOD_TOO_SHORT);

        // Check the data type and load image files if required
        switch(temp.data_type){
        case dlp::Pattern::DataType::IMAGE_FILE:
            // Load the image file
            ret = temp.image_data.Load(temp.image_file);
            if(ret.hasErrors()) return ret;
        case dlp::Pattern::DataType::IMAGE_DATA:
            // Add the verified pattern to the internal sequence
            this->prepared_sequence_.Add(temp);
            break;
        case dlp::Pattern::DataType::PARAMETERS:
        case dlp::Pattern::DataType::INVALID:
            return ret.AddError(PATTERN_DATA_TYPE_INVALID);
            break;
        }
    }

    // Set flag that the sequence has been prepared
    this->sequence_prepared_.Set(true);

    return ret;
}

ReturnCode LCr3000::StartPatternSequence(const unsigned int &start, const unsigned int &patterns, const bool &repeat){
    ReturnCode ret;

    // Check that sequence has been prepared
    if(!this->sequence_prepared_.Get())
        return ret.AddError(DLP_PLATFORM_PATTERN_SEQUENCE_NOT_PREPARED);

    if(repeat && (patterns > 1))
        return ret.AddError("REPEAT SEQUENCES CURRENTLY NOT AVAILABLE");

    // Grab the requested patterns from the internal sequence
    dlp::Pattern::Sequence sequence;
    for(unsigned int iPattern = start; iPattern < (start+patterns); iPattern++){
        dlp::Pattern temp;

        ret = this->prepared_sequence_.Get(iPattern,&temp);
        if(ret.hasErrors()) return ret;

        sequence.Add(temp);
    }

    // Check that all patterns in sequence have the same bitdepth, color, exposure,
    // and period because the LightCrafter 3000 without the MBMC can only use identical
    // settings for each pattern
    if(!sequence.EqualBitDepths())
        return ret.AddError(PATTERN_SEQUENCE_BITDEPTHS_NOT_EQUAL);

    if(!sequence.EqualColors())
        return ret.AddError(PATTERN_SEQUENCE_COLORS_NOT_EQUAL);

    if(!sequence.EqualExposures())
        return ret.AddError(PATTERN_SEQUENCE_EXPOSURES_NOT_EQUAL);

    if(!sequence.EqualPeriods())
        return ret.AddError(PATTERN_SEQUENCE_PERIODS_NOT_EQUAL);

    if(!sequence.EqualDataTypes())
        return ret.AddError(PATTERN_SEQUENCE_PATTERN_TYPES_NOT_EQUAL);

    // Retrieve the first pattern to check its settings
    dlp::Pattern temp;
    sequence.Get(0,&temp);

    unsigned char sequence_bitdepth = ConvertDlpPatternBitdepthToLCr3000Bitdetph(temp.bitdepth);

    // Check that the pattern count is valid
    if((patterns*sequence_bitdepth)>96)
        return ret.AddError(PATTERN_SEQUENCE_TOO_LONG);

    std::vector<std::string> image_names;

    if((start    != this->previous_sequence_start_)    ||
       (patterns != this->previous_sequence_patterns_) ||
       (repeat   != this->previous_sequence_repeat_)){

    ImageBuffer image_buffer;
    image_buffer.Clear();

    unsigned int columns = 0;
    unsigned int rows = 0;
    unsigned int images = 0;

    this->GetColumns(&columns);
    this->GetRows(&rows);


    for(unsigned int iPattern = 0; iPattern < patterns; iPattern++){
        dlp::Pattern temp;

        ret = sequence.Get(iPattern,&temp);
        if(ret.hasErrors()) return ret;

        if(iPattern % 8 == 0) images++;

        for(unsigned int xCol = 0; xCol < columns; xCol++){
            for(unsigned int yRow = 0; yRow < rows; yRow++){

                // Grab the pixel value
                unsigned char value;
                temp.image_data.Unsafe_GetPixel(xCol,yRow, &value);

                if((sequence_bitdepth == 5) || (sequence_bitdepth == 7)){
                    image_buffer.Add(xCol,
                                     yRow,
                                     iPattern*(sequence_bitdepth+1),
                                     (sequence_bitdepth+1),
                                     value << 1);
                }
                else{
                    image_buffer.Add(xCol,
                                     yRow,
                                     iPattern*sequence_bitdepth,
                                     sequence_bitdepth,
                                     value);

                }
            }
        }
    }


    // Save all 12 8-bit images
    image_buffer.SaveImages("lcr3000_sequence_images_", image_names);

    // Check that LCr3000 is connected
    if(!this->isConnected())
        return ret.AddError(LCR3000_NOT_CONNECTED);

    // Set the mode to pattern sequence
    ret = this->LCR_CMD_SetDisplayMode(dlp::LCr3000::DISP_MODE_PTN_SEQ);
    if(ret.hasErrors()) return ret;

    // Setup the sequence for 12 8-bit patterns to upload the image data

    // Setup the sequence settings
    dlp::LCr3000::LCR_PatternSeqSetting_t sequence_settings;

        sequence_settings.BitDepth = 8;
        sequence_settings.NumPatterns = image_names.size();
        sequence_settings.PatternType = dlp::LCr3000::PTN_TYPE_NORMAL;
        sequence_settings.InputTriggerType = dlp::LCr3000::TRIGGER_TYPE_AUTO;   // These settings are temporary
        sequence_settings.InputTriggerDelay = 0;                                // These settings are temporary
        sequence_settings.AutoTriggerPeriod = 20000;                            // These settings are temporary
        sequence_settings.ExposureTime = 20000;                                 // These settings are temporary
        sequence_settings.LEDSelect = dlp::LCr3000::LED_GREEN;                  // These settings are temporary
        sequence_settings.Repeat = 0;

        ret = this->LCR_CMD_SetPatternSeqSetting(&sequence_settings);
        if(ret.hasErrors()) return ret;

        // Upload the BMP files
        if(patterns > 1){
            //for(unsigned int iBMP = 0;iBMP < images;iBMP++){
            for(unsigned int iBMP = 0;iBMP < image_names.size();iBMP++){
                this->LCR_CMD_DefinePatternBMP(iBMP,image_names.at(iBMP).c_str());
            }
        }
        else{
            this->LCR_CMD_DefinePatternBMP(0,image_names.at(0).c_str());
        }



    // Send true sequence settings
    sequence_settings.BitDepth = sequence_bitdepth;
    sequence_settings.NumPatterns = 96/sequence_bitdepth; // Always set for maximum number of patterns
    if(patterns == 1) sequence_settings.NumPatterns = 1;
    sequence_settings.PatternType = dlp::LCr3000::PTN_TYPE_NORMAL;
    sequence_settings.InputTriggerType = (LCR_TriggerType_t)this->trigger_input_type_.Get();
    sequence_settings.InputTriggerDelay = this->trigger_input_delay_.Get();
    sequence_settings.AutoTriggerPeriod = this->sequence_period_.Get();
    sequence_settings.ExposureTime = this->sequence_exposure_.Get();
    sequence_settings.LEDSelect = this->lcr3000_led_select_;
    if(repeat)  sequence_settings.Repeat = 1;
    else        sequence_settings.Repeat = 0;

    ret = this->LCR_CMD_SetPatternSeqSetting(&sequence_settings);
    if(ret.hasErrors()) return ret;

    }

    // Start sequence
    ret = this->LCR_CMD_StartPatternSeq(1); //1 means start


    this->previous_sequence_start_ = start;
    this->previous_sequence_patterns_ = patterns;
    this->previous_sequence_repeat_ = repeat;

    return ret;
}

ReturnCode LCr3000::DisplayPatternInSequence(const unsigned int &pattern_index, const bool &repeat){
    return this->StartPatternSequence(pattern_index, 1, repeat);
}

ReturnCode LCr3000::StopPatternSequence(){
    //check one is running, stop it
    return this->LCR_CMD_StartPatternSeq(0); //0 means stop
}

        /* *** END OF VIRTUAL FUNCTION IMPLEMENTATION ***/


LCr3000::ImageBuffer::ImageBuffer(){
    this->images_.clear();
    this->rows_ = 684;
    this->columns_ = 608;
    this->total_bitplanes_ = 96;
    this->image_bitplanes_ = 8;

    for(unsigned int iImages = 0; iImages < (this->total_bitplanes_/this->image_bitplanes_); iImages++){
        dlp::Image temp(this->columns_, this->rows_, dlp::Image::Format::MONO_UCHAR);
        temp.FillImage((unsigned char)0);
        this->images_.push_back(temp);
        temp.Clear();
    }
}

LCr3000::ImageBuffer::~ImageBuffer(){
    this->Clear();
}

void LCr3000::ImageBuffer::Clear(){
    for(unsigned int iImages = 0; iImages < this->images_.size(); iImages++){
        this->images_.at(iImages).FillImage((unsigned char)0);
    }
}

bool LCr3000::ImageBuffer::Add( const unsigned int &xCol,
                                const unsigned int &yRow,
                                const unsigned int &bitplane_offset,
                                const unsigned int &bitdepth,
                                const unsigned int &value ){

    if((bitplane_offset + bitdepth) > this->total_bitplanes_)
        return false;

    if(bitdepth > 8) return false;

    unsigned int value_masked = value & ((1<<bitdepth)-1);

    unsigned int byte_1        = bitplane_offset / this->image_bitplanes_;
    unsigned int byte_1_offset = bitplane_offset % this->image_bitplanes_;
    unsigned int byte_2        = byte_1 + 1;

    unsigned int value_shifted =  value_masked << byte_1_offset;
    unsigned int byte_1_value  =  value_shifted & 0x00FF;
    unsigned int byte_2_value  = (value_shifted & 0xFF00) >> 8;

    unsigned char value_old = 0;
    unsigned char value_new = 0;

    // Update the first byte
    this->images_.at(byte_1).Unsafe_GetPixel(xCol,yRow,&value_old);
    value_new = value_old + byte_1_value;
    this->images_.at(byte_1).Unsafe_SetPixel(xCol,yRow,value_new);

    // Update the second byte
    if(byte_2_value){
        this->images_.at(byte_2).Unsafe_GetPixel(xCol,yRow,&value_old);
        value_new = value_old + byte_2_value;
        this->images_.at(byte_2).Unsafe_SetPixel(xCol,yRow,value_new);
    }

    return true;
}

ReturnCode LCr3000::ImageBuffer::SaveImages(std::string basename,  std::vector<std::string> &ret_names){
    ReturnCode ret;

    ret_names.clear();

    for(unsigned int iImages = 0; iImages < this->images_.size(); iImages++){
        std::string image_name = basename + dlp::Number::ToString(iImages+10) + ".bmp";
        ret = this->images_.at(iImages).Save(image_name);
        ret_names.push_back(image_name);
        if(ret.hasErrors()) return ret;
    }

    return ret;
}

unsigned char LCr3000::ConvertDlpPatternBitdepthToLCr3000Bitdetph(dlp::Pattern::Bitdepth bitdepth){
    // Check the exposure time
    switch (bitdepth) {
    case dlp::Pattern::Bitdepth::MONO_1BPP: return 1;
    case dlp::Pattern::Bitdepth::MONO_2BPP: return 2;
    case dlp::Pattern::Bitdepth::MONO_3BPP: return 3;
    case dlp::Pattern::Bitdepth::MONO_4BPP: return 4;
    case dlp::Pattern::Bitdepth::MONO_5BPP: return 5;
    case dlp::Pattern::Bitdepth::MONO_6BPP: return 6;
    case dlp::Pattern::Bitdepth::MONO_7BPP: return 7;
    case dlp::Pattern::Bitdepth::MONO_8BPP: return 8;
    default:                                return 0;
    }
}

bool LCr3000::ValidSequenceLength(const unsigned char &bitdepth, const unsigned char &pattern_count){
    bool ret = true;
    unsigned int bitplanes = bitdepth * pattern_count;

    if(bitplanes <= 96){

        if(bitplanes <= 24){
            ret = true;
        }
        else if(((bitplanes % 2) == 0) && ((bitplanes/2)<=24) && (((bitplanes/2)%bitdepth)==0)){
            ret = true;
        }
        else if(((bitplanes % 4) == 0) && ((bitplanes/4)<=24) && (((bitplanes/4)%bitdepth)==0)){
            ret = true;
        }
        else{
            ret = false;
        }
    }
    else{
        // More than 96 bitplanes requested when the buffer only has 96
        ret = false;
    }

    return ret;
}

ReturnCode LCr3000::LCR_CMD_Open(){
    return LCR_CMD_PKT_ConnectToLCR();
}

ReturnCode LCr3000::LCR_CMD_Close(){
    ReturnCode ret;

    LCR_CMD_PKT_DisconnectLCR();

    return ret; //no possible error
}

ReturnCode LCr3000::LCR_CMD_GetRevision(LCR_Revision_t Which, char* VersionStr){
    ReturnCode ret;
    //Frame the packet
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_READ, 0x0100);

    if(Which != REV_DM365 && Which != REV_FPGA && Which != REV_MSP430){
        ret.AddError(LCR3000_INVALID_REVISION);
        return ret;
    }

    LCR_CMD_PKT_PutInt(Which,1);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    LCR_CMD_PKT_GetData((uint8_t*)VersionStr,LCR_CMD_VERSION_STR_LEN);

    return ret;
}

ReturnCode LCr3000::LCR_CMD_SetDisplayMode(LCR_DisplayMode_t Mode){
    ReturnCode ret;

    if(Mode > DISP_MODE_PTN_SEQ){
        ret.AddError(LCR3000_INVALID_DISPLAY_MODE);
        return ret;
    }

    /* Generate packet */
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_WRITE, 0x0101);
    LCR_CMD_PKT_PutInt((int)Mode,1);
    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    return ret;
}

LCr3000::LCR_DisplayMode_t LCr3000::LCR_CMD_GetDisplayMode(){
    //dlp::ReturnCode ret;
    uint8_t data;

    /* Read display mode */
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_READ, 0x0101);

    LCR_CMD_PKT_SendCommand();

    LCR_CMD_PKT_GetData((uint8_t*)&data,1);

    return (LCR_DisplayMode_t)data;
}

ReturnCode LCr3000::LCR_CMD_SetPowerMode(LCR_PowerMode_t Mode){
    ReturnCode ret;
    //TODO
    return ret;
}

LCr3000::LCR_PowerMode_t LCr3000::LCR_CMD_GetPowerMode(){
    uint8_t data = 0;
    //TODO
    return (LCR_PowerMode_t) data;
}

ReturnCode LCr3000::LCR_CMD_SetTestPattern(LCR_TestPattern_t TestPtn){
    ReturnCode ret;

    if(TestPtn > TEST_PTN_ANXI_CHECKER){
        ret.AddError(LCR3000_TEST_PATTER_OUT_OF_BOUNDS);
        return ret;
    }

    /* Generate packet */
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_WRITE, 0x0103);

    LCR_CMD_PKT_PutInt((int)TestPtn,1);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    return ret;
}

LCr3000::LCR_TestPattern_t LCr3000::LCR_CMD_GetTestPattern(){
    ReturnCode ret;

    uint8_t data;

    /* Read Test Pattern set */
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_READ, 0x0103);
    LCR_CMD_PKT_SendCommand();
    LCR_CMD_PKT_GetData((uint8_t*)&data,1);

    return (LCR_TestPattern_t)data;
}

ReturnCode LCr3000::LCR_CMD_SetLEDCurrent(LCR_LEDCurrent_t *LEDSetting){
    ReturnCode ret;

    /* Write LED current */
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_WRITE, 0x0104);
    LCR_CMD_PKT_PutInt(LEDSetting->Red, 2);
    LCR_CMD_PKT_PutInt(LEDSetting->Green, 2);
    LCR_CMD_PKT_PutInt(LEDSetting->Blue, 2);
    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    return ret;
}

ReturnCode LCr3000::LCR_CMD_GetLEDCurrent(LCR_LEDCurrent_t *LEDSetting){
    ReturnCode ret;
    /* Read LED current */
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_READ, 0x0104);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    LEDSetting->Red = LCR_CMD_PKT_GetInt(2);
    LEDSetting->Green = LCR_CMD_PKT_GetInt(2);
    LEDSetting->Blue = LCR_CMD_PKT_GetInt(2);

    return ret;
}

ReturnCode LCr3000::LCR_CMD_DisplayStaticImage(char const *fileNameWithPath){
    ReturnCode ret;
    /* Generate packet */
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_WRITE, 0x0105);

    /*TBD - Check for return error*/
    LCR_CMD_PKT_PutFile(fileNameWithPath);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    return ret;
}

ReturnCode LCr3000::LCR_CMD_DisplayStaticColor(uint32_t Color){
    ReturnCode ret;
    /* Generate packet */
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_WRITE, 0x0106);
    LCR_CMD_PKT_PutInt((uint32_t)Color,4);
    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    return ret;
}

ReturnCode LCr3000::LCR_CMD_SetDisplaySetting(LCR_DisplaySetting_t const *Setting){
    ReturnCode ret;
    /* Generate packet */
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_WRITE, 0x0107);

    LCR_CMD_PKT_PutInt((int)Setting->LongAxisFlip,1);

    LCR_CMD_PKT_PutInt((int)Setting->ShortAxisFlip,1);

    LCR_CMD_PKT_PutInt((int)Setting->Rotate,1);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    return ret;
}

    ReturnCode LCr3000::LCR_CMD_GetDisplaySetting(LCR_DisplaySetting_t *Setting){
        ReturnCode ret;

        uint8_t data;

        /* Display Settings */
        LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_READ, 0x0107);

        if(LCR_CMD_PKT_SendCommand())
        {
            ret.AddError(LCR3000_SEND_COMMAND_FAILED);
            return ret;
        }

        LCR_CMD_PKT_GetData(&data,1);

        Setting->LongAxisFlip = data;

        LCR_CMD_PKT_GetData(&data,1);

        Setting->ShortAxisFlip = data;

        LCR_CMD_PKT_GetData(&data,1);

        Setting->Rotate = data;

        return ret;
    }

ReturnCode LCr3000::LCR_CMD_SetVideoSetting(LCr3000::LCR_VideoSetting_t const *Setting){
    ReturnCode ret;

    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_WRITE, 0x0200);
    LCR_CMD_PKT_PutInt(Setting->ResolutionX,2);
    LCR_CMD_PKT_PutInt(Setting->ResolutionY,2);
    LCR_CMD_PKT_PutInt(Setting->FirstPix,2);
    LCR_CMD_PKT_PutInt(Setting->FirstLine,2);
    LCR_CMD_PKT_PutInt(Setting->ActiveWidth,2);
    LCR_CMD_PKT_PutInt(Setting->ActiveHeight,2);
    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    return ret;
}

ReturnCode LCr3000::LCR_CMD_GetVideoSetting(LCR_VideoSetting_t *Setting){
    ReturnCode ret;

    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_READ, 0x0200);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    //Assign to the structures
    Setting->ResolutionX = LCR_CMD_PKT_GetInt(2);
    Setting->ResolutionY = LCR_CMD_PKT_GetInt(2);
    Setting->FirstPix = LCR_CMD_PKT_GetInt(2);
    Setting->FirstLine = LCR_CMD_PKT_GetInt(2);
    Setting->ActiveWidth = LCR_CMD_PKT_GetInt(2);
    Setting->ActiveHeight = LCR_CMD_PKT_GetInt(2);

    return ret;
}

ReturnCode LCr3000::LCR_CMD_SetVideoMode(LCR_VideoModeSetting_t *Setting){
    ReturnCode ret;
    /*Frame the packet*/
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_WRITE, 0x0201);
    LCR_CMD_PKT_PutInt((int)Setting->FrameRate,1);
    LCR_CMD_PKT_PutInt((int)Setting->BitDepth,1);
    LCR_CMD_PKT_PutInt((int)Setting->RGB,1);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    return ret;
}

ReturnCode LCr3000::LCR_CMD_GetVideoMode(LCR_VideoModeSetting_t *Setting){
    ReturnCode ret;

    uint8_t data;

    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_READ, 0x0201);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    LCR_CMD_PKT_GetData(&data,1);
    Setting->FrameRate = data;
    LCR_CMD_PKT_GetData(&data,1);
    Setting->BitDepth = data;
    LCR_CMD_PKT_GetData(&data,1);
    Setting->RGB = data;

    return ret;
}

ReturnCode LCr3000::LCR_CMD_SetPatternSeqSetting(LCR_PatternSeqSetting_t const *Setting){
    ReturnCode ret;

    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_WRITE, 0x0480); //The old cmd 0x0400 is kept for backward compatibility only use the new one
    LCR_CMD_PKT_PutInt((int)Setting->BitDepth,1);
    LCR_CMD_PKT_PutInt((int)Setting->NumPatterns,2);
    LCR_CMD_PKT_PutInt((int)Setting->PatternType,1);
    LCR_CMD_PKT_PutInt((int)Setting->InputTriggerType,1);
    LCR_CMD_PKT_PutInt((uint32_t)Setting->InputTriggerDelay,4);
    LCR_CMD_PKT_PutInt((uint32_t)Setting->AutoTriggerPeriod,4);
    LCR_CMD_PKT_PutInt((uint32_t)Setting->ExposureTime,4);
    LCR_CMD_PKT_PutInt((int)Setting->LEDSelect,1);
    LCR_CMD_PKT_PutInt((int)Setting->Repeat,1);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    return ret;
}

ReturnCode LCr3000::LCR_CMD_GetPatternSeqSetting(LCR_PatternSeqSetting_t *Setting){
    ReturnCode ret;

    uint8_t  data;
    uint32_t data32;

    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_READ, 0x0480);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    LCR_CMD_PKT_GetData(&data,1);
    Setting->BitDepth = data;

    Setting->NumPatterns = LCR_CMD_PKT_GetInt(2);

    LCR_CMD_PKT_GetData(&data,1);
    Setting->PatternType = (LCR_PatternType_t) data;

    LCR_CMD_PKT_GetData(&data,1);
    Setting->InputTriggerType = (LCR_TriggerType_t)data;

    LCR_CMD_PKT_GetData((uint8_t*)&data32,4);
    Setting->InputTriggerDelay = data32;

    LCR_CMD_PKT_GetData((uint8_t*)&data32,4);
    Setting->AutoTriggerPeriod = data32;

    LCR_CMD_PKT_GetData((uint8_t*)&data32,4);
    Setting->ExposureTime = data32;

    LCR_CMD_PKT_GetData(&data,1);
    Setting->LEDSelect = (LCR_LEDSelect_t) data;

    LCR_CMD_PKT_GetData(&data,1);
    Setting->Repeat = data;

    return ret;
}

ReturnCode LCr3000::LCR_CMD_DefinePatternBMP(LCR_PatternCount_t PatternNum, char const *fileNameWithPath){
    ReturnCode ret;
    /* Generate packet */
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_WRITE, 0x0401);
    LCR_CMD_PKT_PutInt((int)PatternNum, 1);
    LCR_CMD_PKT_PutFile(fileNameWithPath);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    return ret;
}

ReturnCode LCr3000::LCR_CMD_ReadPattern(LCR_PatternCount_t PatternNum, char *fileName){
    ReturnCode ret;

    unsigned long int numbytes;

    //Determine the bit depth
    LCR_PatternSeqSetting_t Setting;
    LCR_CMD_GetPatternSeqSetting(&Setting);
    switch(Setting.BitDepth)
    {
    case 1:
        numbytes = ONE_BPP_PTN_SIZE;
        break;
    case 2:
        numbytes = TWO_BPP_PTN_SIZE;
        break;
    case 3:
        numbytes = THREE_BPP_PTN_SIZE;
        break;
    case 4:
        numbytes = FOUR_BPP_PTN_SIZE;
        break;
    case 5:
        numbytes = FIVE_BPP_PTN_SIZE;
        break;
    case 6:
        numbytes = SIX_BPP_PTN_SIZE;
        break;
    case 7:
        numbytes = SEVEN_BPP_PTN_SIZE;
        break;
    case 8:
        numbytes = EIGHT_BPP_PTN_SIZE;
        break;
    default:
        return ret.AddError("ERROR");
    }

    /* Generate packet */
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_READ, 0x0401);
    LCR_CMD_PKT_PutData((uint8_t*) &PatternNum, 1);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    LCR_CMD_PKT_GetFile(fileName,numbytes);

    return ret;
}

ReturnCode LCr3000::LCR_CMD_StartPatternSeq(uint8_t Start){
    ReturnCode ret;
    /* Generate packet */
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_WRITE, 0x0402);
    LCR_CMD_PKT_PutInt(Start,1);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    return ret;
}

ReturnCode LCr3000::LCR_CMD_AdvancePatternSeq(){
    ReturnCode ret;
    /* Generate packet */
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_WRITE, 0x0403);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    return ret;
}

ReturnCode LCr3000::LCR_CMD_GetCamTriggerSetting(LCR_CamTriggerSetting_t *Setting){
    ReturnCode ret;

    uint8_t  data;
    uint32_t data32;

    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_READ, 0x0404);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    LCR_CMD_PKT_GetData(&data,1);
    Setting->Enable = data;
    LCR_CMD_PKT_GetData(&data,1);
    Setting->Source = data;
    LCR_CMD_PKT_GetData(&data,1);
    Setting->Polarity = data;
    LCR_CMD_PKT_GetData((uint8_t*)&data32,4);
    Setting->Delay = data32;
    LCR_CMD_PKT_GetData((uint8_t*)&data32,4);
    Setting->PulseWidth = data32;

    return ret;
}

ReturnCode LCr3000::LCR_CMD_SetCamTriggerSetting(LCR_CamTriggerSetting_t *Setting){
    ReturnCode ret;

    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_WRITE, 0x0404);
    LCR_CMD_PKT_PutInt((int)Setting->Enable,1);
    LCR_CMD_PKT_PutInt((int)Setting->Source,1);
    LCR_CMD_PKT_PutInt((int)Setting->Polarity,1);
    LCR_CMD_PKT_PutInt((uint32)Setting->Delay,4);
    LCR_CMD_PKT_PutInt((uint32)Setting->PulseWidth,4);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    return ret;
}

ReturnCode LCr3000::LCR_CMD_DefineHWPatSequence(LCR_HWPatternSeqDef_t *hwPatSeqDef){
    ReturnCode ret;

    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_WRITE, 0x0406);
    LCR_CMD_PKT_PutInt((int)hwPatSeqDef->index,1);
    LCR_CMD_PKT_PutInt((int)hwPatSeqDef->numOfPatn,1);
    for(int i=0; i<hwPatSeqDef->numOfPatn;i++)
    {
        LCR_CMD_PKT_PutInt((int)hwPatSeqDef->hwPatArray[i].Number,1);
        LCR_CMD_PKT_PutInt((int)hwPatSeqDef->hwPatArray[i].Invert,1);
    }

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    return ret;
}

ReturnCode LCr3000::LCR_CMD_SaveSolution(char *SolutionName){
    ReturnCode ret;

    uint8_t tempName[LCR_CMD_SOLUTION_NAME_LEN];

    //copy the string into temporary buffer
    strcpy((char*)&tempName[0],SolutionName);

    //Frame the packet
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_WRITE, 0x0600);
    LCR_CMD_PKT_PutData((uint8_t*)&tempName[0],LCR_CMD_SOLUTION_NAME_LEN);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    return ret;
}

ReturnCode LCr3000::LCR_CMD_GetSolutionNames(uint8_t *Count, uint8_t *DefaultSolution, char *SolutionName){
    ReturnCode ret;

    //Frame the packet
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_READ, 0x0600);


    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    LCR_CMD_PKT_GetData(Count,1);
    LCR_CMD_PKT_GetData(DefaultSolution,1);
    LCR_CMD_PKT_GetData((uint8_t*)SolutionName,((*Count)*LCR_CMD_SOLUTION_NAME_LEN));

    return ret;
}

ReturnCode LCr3000::LCR_CMD_ManageSolution(LCR_SolutionCommand_t Cmd, char *SolutionName){
    ReturnCode ret;

    uint8_t tempName[LCR_CMD_SOLUTION_NAME_LEN];

    //copy the string into temporary buffer
    strcpy((char*)&tempName[0],SolutionName);

    //Frame the packet
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_WRITE, 0x0601);
    LCR_CMD_PKT_PutData((uint8_t*)&Cmd,1);
    LCR_CMD_PKT_PutData((uint8_t*)&tempName[0],LCR_CMD_SOLUTION_NAME_LEN);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    return ret;
}

ReturnCode LCr3000::LCR_CMD_LoadCustomSequence(char *seqBinFileName){
    ReturnCode ret;
    /* Generate packet */
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_WRITE, 0x0A00);

    /*TBD - Check for return error*/
    LCR_CMD_PKT_PutFile(seqBinFileName);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    return ret;
}

/*Setup the vectors*/
ReturnCode LCr3000::LCR_CMD_SetupCustomSequencevectors(uint8_t startVector, uint8_t numOfvectors){
    ReturnCode ret;
    /* Generate packet */
    LCR_CMD_PKT_CommandInit(LCR_CMD_PKT_TYPE_WRITE, 0x0A01);
    LCR_CMD_PKT_PutData((uint8_t*)&startVector,1);
    LCR_CMD_PKT_PutData((uint8_t*)&numOfvectors,1);

    if(LCR_CMD_PKT_SendCommand()){
        ret.AddError(LCR3000_SEND_COMMAND_FAILED);
        return ret;
    }

    return ret;
}

/********* Begin Packetizer Methods ********/

ReturnCode LCr3000::LCR_CMD_PKT_ConnectToLCR(){
    ReturnCode ret;
    /* Open tcp connection with LCr */
    LCR_PKT_Socket = TCP_Connect(ip, port);

    /*if(LCR_PKT_Socket < 0)
        return FAIL;*/
    if(LCR_PKT_Socket == NULL)
    {
        ret.AddError(LCR3000_COULD_NOT_CONNECT);
        return ret;
    }

    this->is_connected_ = true;

    return ret;
}

ReturnCode LCr3000::LCR_CMD_PKT_DisconnectLCR(){
    ReturnCode ret;
    TCP_Disconnect(LCR_PKT_Socket);

    this->is_connected_ = false;

    return ret;
}

int LCr3000::LCR_CMD_ReadData(uint8_t *data, uint32_t size){
    return TCP_Receive(LCR_PKT_Socket,data,size);
}

int LCr3000::LCR_CMD_WriteData(uint8_t *data, uint32_t size){
    return TCP_Send(LCR_PKT_Socket, data, size);
}

int LCr3000::LCR_CMD_PKT_CommandInit(LCR_CommandType_t cmdType, uint16_t cmdId){
    if(cmdType == LCR_CMD_PKT_TYPE_WRITE)
        LCR_PacketType = PKT_TYPE_WRITE;
    else if(cmdType == LCR_CMD_PKT_TYPE_READ)
        LCR_PacketType = PKT_TYPE_READ;
    else
        return -1;

    contFlag = 0;
    commandId = cmdId;
    dataLength = 0;

    return 0;
}

int LCr3000::LCR_CMD_PKT_PutData(uint8_t *data, unsigned long int size){
    if(data == NULL)
        return -1;

    while(size)
    {
        int  copySize = size;
        if(dataLength == MAX_PACKET_SIZE)
        {
            if(LCR_CMD_PKT_SendPacket(1))
                return -1;
            dataLength = 0;
        }

        if(dataLength + copySize > MAX_PACKET_SIZE)
        {
            copySize = MAX_PACKET_SIZE - dataLength;
        }

        memcpy(packetData + dataLength, data, copySize);
        dataLength += copySize;
        size -= copySize;
    }

    return 0;
}

int LCr3000::LCR_CMD_PKT_PutInt(uint32_t value, unsigned int size){
    uint8_t data[4];
    unsigned int i;

    if(size > 4)
        return -1;

    for(i = 0; i < size; i++)
    {
        data[i] = (value & 0xFF);
        value >>= 8;
    }

    if(LCR_CMD_PKT_PutData(data, size))
        return -1;

    return 0;
}

int LCr3000::LCR_CMD_PKT_PutFile(char const *fileName){
    int error = 0;
    FILE *fp;
    int copySize;

    fp = fopen(fileName, "rb");
    if(fp == NULL)
        return -1;

    while(!feof(fp))
    {
        if(dataLength == MAX_PACKET_SIZE)
        {
            if(LCR_CMD_PKT_SendPacket(1))
            {
                error = -1;
                break;
            }
        }

        copySize = MAX_PACKET_SIZE - dataLength;

        copySize = fread(packetData + dataLength,
            1, copySize, fp);
        if(copySize <= 0)
            break;

        dataLength += copySize;
    }

    fclose(fp);

    return error;
}

int LCr3000::LCR_CMD_PKT_GetFile(char const *fileName,uint32_t size){
    FILE *fp;
    int ret = 0;
    uint32_t remSize = size;

    if(packetBuffer[0] != PKT_TYPE_READ_RESP)
    {
        return -1;
    }

    fp = fopen(fileName, "wb");
    if(fp == NULL)
    {
        return -1;
    }

    if(remSize == 0)
        remSize = MAX_PACKET_SIZE;

    while(remSize)
    {
        unsigned int copySize = dataLength - parseIndex;

        if(copySize == 0)
        {
            if(LCR_CMD_PKT_ReceivePacket(0))
            {
                if(size != 0)
                    ret = -1;
                break;
            }

            copySize = dataLength - parseIndex;
        }

        if(copySize >= remSize)
        {
            copySize = remSize;
        }

        if(fwrite(packetData + parseIndex, 1, copySize, fp) != copySize)
        {
            ret = -1;
            break;
        }

        parseIndex += copySize;

        if(size != 0)
            remSize -= copySize;
    }

    fclose(fp);

    return ret;
}

int LCr3000::LCR_CMD_PKT_GetData(uint8_t *data, unsigned long int size){
    if(packetBuffer[0] != PKT_TYPE_READ_RESP)
    {
        return -1;
    }

    while(size)
    {
        unsigned int copySize = dataLength - parseIndex;

        if(copySize == 0)
        {
            if(LCR_CMD_PKT_ReceivePacket(0))
                return -1;

            copySize = dataLength - parseIndex;
        }

        if(copySize >= size)
        {
            copySize = size;
        }

        memcpy(data, packetData + parseIndex, copySize);
        parseIndex += copySize;
        size -= copySize;
        data += copySize;
    }

    return 0;
}

uint32_t LCr3000::LCR_CMD_PKT_GetInt(unsigned int size){
    uint32_t value = 0;
    uint8_t data[4];
    unsigned int i;

    if(size > 4)
        return 0;

    if(LCR_CMD_PKT_GetData(data, size))
        return 0;

    for(i = 0; i < size; i++)
    {
        value |= data[i] << (i * 8);
    }

    return value;
}

uint8_t LCr3000::LCR_CMD_PKT_CalcChecksum(){
    int i;
    int sum = 0;

    for(i = 0; i < dataLength + HEADER_SIZE; i++)
    {
        sum += packetBuffer[i];
    }
    return (uint8_t)(sum & 0xFF);
}

int LCr3000::LCR_CMD_PKT_ReceivePacket(bool firstPkt){
    unsigned long int mask;
    int i;

    dataLength = 0;
    parseIndex = 0;

    if(firstPkt == 0) //this is not the first packet
    {
        if(recvFlag == 0 ||  recvFlag == 3) //the previous packet contained all data or last packet
        {
            return -1;
        }
    }

    if(LCR_CMD_ReadData(packetBuffer, HEADER_SIZE))
    {
        //ret.AddError(LCR3000_HEADER_READ_FAIL);
        return -1;
    }

    dataLength = packetBuffer[4] | packetBuffer[5] << 8;

    if(LCR_CMD_ReadData(packetData, dataLength + 1))
    {
        return -1;
    }

    if(packetData[dataLength] != LCR_CMD_PKT_CalcChecksum())
    {
        //ret.AddError(LCR3000_CHECKSUM_RESPONSE_FAILED);
        return -1;
    }

    if(packetBuffer[0] != LCR_PacketType + 1)
    {
        //Determine the response packet
        switch(packetBuffer[0])
        {

        case 0x00:
            //ret.AddError(LCR3000_CMD_NOT_EXCECUTED_REASON_BUSY);
            break;

        case 0x01:
            //ret.AddError(LCR3000_PACKET_CONTAINS_ERROR);

            //Set flag based on the error #
            mask = 0;
            i = 0;
            while(dataLength--)
            {
                if(packetBuffer[(6+i)] != 0)
                {
                    mask |= (1<<(packetBuffer[(6+i)] - 1));
                }

                i++;
            }

            //print each error
            if((mask & 0x01) == 0x01)
            {
                //ret.AddError(LCR3000_CMD_FAILED);
            }
            else if((mask & 0x02) == 0x02)
            {
                //ret.AddError(LCR3000_UNSUPPORTED_COMMAND);
            }
            else if((mask & 0x04) == 0x04)
            {
                //ret.AddError(LCR3000_INVALID_PARAM);
            }
            else if((mask & 0x08) == 0x08)
            {
                //ret.AddError(LCR3000_OUT_OF_RESOURCE);
            }
            else if((mask & 0x10) == 0x10)
            {
                //ret.AddError(LCR3000_DEVICE_FAIL);
            }
            else if((mask & 0x20) == 0x20)
            {
                //ret.AddError(LCR3000_DEVICE_BUSY);
            }
            else if((mask & 0x40) == 0x40)
            {
                //ret.AddError(LCR3000_NOT_INITIALIZED);
            }
            else if((mask & 0x80) == 0x80)
            {
                //ret.AddError(LCR3000_NOT_FOUND);
            }
            else if((mask & 0x100) == 0x100)
            {
                //ret.AddError(LCR3000_CHECKSUM_FAILED);
            }
            else if((mask & 0x200) == 0x200)
            {
                //ret.AddError(LCR3000_PACKET_FORMAT_ERROR);
            }
            else if((mask & 0x400) == 0x400)
            {
                //ret.AddError(LCR3000_COMMAND_CONTINUE_FAILED);
            }
            else
            {
                //ret.AddError(LCR3000_UNDOCUMENTED_ERROR);
            }

            break;

        case 0x02:
        case 0x04:
            //ret.AddWarning(LCR3000_RESPONSE_WRITE_OR_READ);
            break;

        default:
            //ret.AddError(LCR3000_UNKNOWN_PACKET_TYPE);
            break;
        }

        return -1;
    }

    recvFlag = packetBuffer[3];

    if(firstPkt != (recvFlag == 0 || recvFlag == 1))
    {
        return -1;
    }

    if(recvFlag == 3)
    {
        /* Command SUCCESS */
    }

    return 0;
}

int LCr3000::LCR_CMD_PKT_SendPacket(bool more){
    uint8_t flag;

    packetBuffer[0] = LCR_PacketType;
    packetBuffer[1] = (commandId >> 8) & 0xFF;
    packetBuffer[2] = (commandId) & 0xFF;

    if(contFlag)
    {
        if(more)
            flag = 2;
        else
            flag = 3;
    }
    else
    {
        if(more)
            flag = 1;
        else
            flag = 0;
    }
    contFlag = more;

    packetBuffer[3] = flag;
    packetBuffer[4] = dataLength & 0xFF;
    packetBuffer[5] = (dataLength >> 8) & 0xFF;

    packetData[dataLength] = LCR_CMD_PKT_CalcChecksum();

    int length = dataLength + HEADER_SIZE + CHECKSUM_SIZE;

    if(LCR_CMD_WriteData(packetBuffer, length))
        return -1;

    if(LCR_CMD_PKT_ReceivePacket(1))
        return -1;

    if(more == 0 && recvFlag == 0)
    {
        /* SUCCESS */
    }

    return 0;
}

int LCr3000::LCR_CMD_PKT_SendCommand(){
    return LCR_CMD_PKT_SendPacket(0);
}

/******** BEGIN TCP CLIENT METHODS ********/

asio::ip::tcp::tcp::socket* LCr3000::TCP_Connect(std::string host, unsigned long int port){

    //std::cout << "Creating sock..." << std::endl;
    asio::ip::tcp::tcp::socket* sock = new asio::ip::tcp::tcp::socket(io_service);

    //std::cout << "Creating addr..." << std::endl;
    asio::ip::address_v4 addr(asio::ip::address_v4::from_string(host));

    //std::cout << "Creating ep..." << "Addr - " << addr << "Port - " << (unsigned short) port << std::endl;
    asio::ip::tcp::tcp::endpoint ep(addr,(unsigned short) port);

    try
    {
        //std::cout << "Opening sock..." << std::endl;
        sock->open(asio::ip::tcp::v4());
    }
    catch(...)//boost::system::system_error err)
    {
        //std::cout << "Opening sock failed!" << std::endl;
        return NULL;
    }

    try
    {
        //std::cout << "Connecting sock..." << std::endl;
        sock->connect(ep);
    }
    catch(...)//boost::system::system_error err)
    {
        return NULL;
    }

    return sock;
}

int LCr3000::TCP_Send(asio::ip::tcp::tcp::socket* sock, unsigned char *buffer, int length){
    //return asio::write(*sock,asio::buffer(buffer,length));
    size_t ret;

    ret = sock->send(asio::buffer(buffer,length));

    if(ret > 0) return 0;
    else return 1;
}

int LCr3000::TCP_Receive(asio::ip::tcp::tcp::socket* sock, unsigned char *buffer, int length){
    size_t ret;

    ret = sock->read_some(asio::buffer(buffer,length));

    if(ret > 0) return 0;
    else return 1;
}

int LCr3000::TCP_Disconnect(asio::ip::tcp::tcp::socket* sock){
    sock->close();
    //this->io_service.stop();
    return 0;
}

namespace Number{

template <> std::string ToString<dlp::LCr3000::ImageFlip::Enum>( dlp::LCr3000::ImageFlip::Enum value ){
    return dlp::Number::ToString((int) value);
}

template <> std::string ToString<dlp::LCr3000::InputTriggerType::Enum>( dlp::LCr3000::InputTriggerType::Enum value ){
    return dlp::Number::ToString((int) value);
}


template <> std::string ToString<dlp::LCr3000::OutputTriggerPolarity::Enum>( dlp::LCr3000::OutputTriggerPolarity::Enum value ){
    switch(value){
    case dlp::LCr3000::OutputTriggerPolarity::NEGATIVE: return "NEGATIVE";
    case dlp::LCr3000::OutputTriggerPolarity::POSITIVE: return "POSITIVE";
    default:                                            return "POSITIVE";
    }
}

}

namespace String{
    template <> dlp::LCr3000::ImageFlip::Enum ToNumber( const std::string &text, unsigned int base ){
        return (dlp::LCr3000::ImageFlip::Enum)dlp::String::ToNumber<int>(text);
    }

    template <> dlp::LCr3000::InputTriggerType::Enum ToNumber( const std::string &text, unsigned int base ){
        return (dlp::LCr3000::InputTriggerType::Enum)dlp::String::ToNumber<int>(text);
    }

    template <> dlp::LCr3000::OutputTriggerPolarity::Enum ToNumber( const std::string &text, unsigned int base ){
        // Ignore base variable
        if (text.compare("NEGATIVE") == 0){
            return dlp::LCr3000::OutputTriggerPolarity::NEGATIVE;
        }
        else{
            return dlp::LCr3000::OutputTriggerPolarity::POSITIVE;
        }
    }
}

}
