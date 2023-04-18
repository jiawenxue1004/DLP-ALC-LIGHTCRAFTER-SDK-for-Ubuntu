/** @file   LCr6500.cpp
 *  @brief  Contains methods to interface with DLP LightCrafter 6500 EVM
 *  @copyright 2014 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#include <common/returncode.hpp>
#include <common/debug.hpp>
#include <common/other.hpp>
#include <common/image/image.hpp>
#include <common/parameters.hpp>
#include <common/pattern/pattern.hpp>

#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <atomic>

#include <ctime>

#include <dlp_platforms/dlp_platform.hpp>
#include <dlp_platforms/lightcrafter_6500/lcr6500.hpp>

#include <dlp_platforms/lightcrafter_6500/dlpc900_api.hpp>
#include <dlp_platforms/lightcrafter_6500/common.hpp>
#include <dlp_platforms/lightcrafter_6500/error.hpp>
#include <dlp_platforms/lightcrafter_6500/dlpc900_usb.hpp>
#include <dlp_platforms/lightcrafter_6500/dlpc900_image.hpp>
#include <dlp_platforms/lightcrafter_6500/flashdevice.hpp>

/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{

const unsigned char LCr6500::Led::MAXIMUM_CURRENT               = 255;
const unsigned int  LCr6500::ImageIndex::MAXIMUM_INDEX          = MAX_IMAGE_ENTRIES_ON_THE_FLY;
const unsigned int  LCr6500::Video::TestPattern::COLOR_MAXIMUM  = 1023;

const unsigned long int LCr6500::Pattern::Exposure::MAXIMUM = 200000000;
const unsigned long int LCr6500::Pattern::Exposure::PERIOD_DIFFERENCE_MINIMUM = 105;

const unsigned int LCr6500::PATTERN_LUT_SIZE  = MAX_PAT_LUT_ENTRIES;

unsigned long int LCr6500::Pattern::Exposure::MININUM(const dlp::Pattern::Bitdepth &bitdepth){
    // See the DLPC900 Programmer's Guide for the maximum pattern rate for
    // preloaded patterns with external trigger
    switch(bitdepth){
    case dlp::Pattern::Bitdepth::MONO_1BPP:   return     120;
    case dlp::Pattern::Bitdepth::MONO_2BPP:   return     320;
    case dlp::Pattern::Bitdepth::MONO_3BPP:   return     420;
    case dlp::Pattern::Bitdepth::MONO_4BPP:   return     840;
    case dlp::Pattern::Bitdepth::MONO_5BPP:   return    1230;
    case dlp::Pattern::Bitdepth::MONO_6BPP:   return    1506;
    case dlp::Pattern::Bitdepth::MONO_7BPP:   return    2016;
    case dlp::Pattern::Bitdepth::MONO_8BPP:   return    4065;
    case dlp::Pattern::Bitdepth::RGB_3BPP:    return     360;
    case dlp::Pattern::Bitdepth::RGB_6BPP:    return     960;
    case dlp::Pattern::Bitdepth::RGB_9BPP:    return    1260;
    case dlp::Pattern::Bitdepth::RGB_12BPP:   return    2521;
    case dlp::Pattern::Bitdepth::RGB_15BPP:   return    3690;
    case dlp::Pattern::Bitdepth::RGB_18BPP:   return    4518;
    case dlp::Pattern::Bitdepth::RGB_21BPP:   return    6048;
    case dlp::Pattern::Bitdepth::RGB_24BPP:   return   12195;
    case dlp::Pattern::Bitdepth::INVALID:
    default:
        return MAXIMUM;
    }
}


/** @brief  Constructs empty object */
LCr6500::LCr6500()
{
    // Set the debug name
    this->debug_.SetName("LCR6500_DEBUG: ");

    // Set the platform
    this->SetPlatform(Platform::LIGHTCRAFTER_6500);

//    // Set default values
//    this->previous_command_in_progress      = false;
//    this->firmware_upload_restart_needed    = false;
//    this->pattern_sequence_prepared_        = false;

//    this->firmware_upload_percent_erased_   = 0;
//    this->firmware_upload_percent_complete_ = 0;

    this->image_previous_first_ = -1;
    this->image_previous_last_ = -1;


    // Get DMD columns and rows
    unsigned int dmd_rows = 0;
    unsigned int dmd_cols = 0;
    this->GetColumns(&dmd_cols);
    this->GetRows(&dmd_rows);



    // Compress the black and white images






    this->debug_.Msg(1,"Object constructed");
}




LCr6500::~LCr6500()
{

    // Clear the compressed images vector so it can be refilled
    for(unsigned int iImage = 0; iImage < this->compressed_images_.size(); iImage++){
        free(this->compressed_images_.at(iImage).bitmapImage1);
        //free(this->compressed_images_.at(iImage).bitmapImage2); // Uncomment for DLP9000
    }
    this->compressed_images_.clear();

    if(this->pattern_image_white_.sizeBitmap1>0){
        free(this->pattern_image_white_.bitmapImage1);
    }

    if(this->pattern_image_black_.sizeBitmap1>0){
        free(this->pattern_image_black_.bitmapImage1);
    }
}



/** @brief  Connects to a DLP LightCrafter 6500 EVM
 *  @param[in]  device  assigned ID of individual projector
 *
 *  Functionality of multiple projectors
 *  NOT yet implemented but will allow multiple
 *  projectors to be connected to the DLP SDK.
 */
ReturnCode LCr6500::Connect(std::string id){
    ReturnCode ret;


//    // If A firmware upload is in progress, do NOT send any commands until upload is complete! and does not
//    // need a restard return and error
//    if( this->FirmwareUploadInProgress() && !this->firmware_upload_restart_needed){
//        this->debug_.Msg("Cannot connect because firmware is uploading");
//        return ret.AddError(LCR6500_FIRMWARE_UPLOAD_IN_PROGRESS);
//    }


    // First initialize the HID USB interface
    if(DLPC900_USB_IsConnected()){
        DLPC900_USB_Close();
        DLPC900_USB_Exit();
    }

    // Initialize the USB interface
    this->debug_.Msg("Initializing USB HID interface...");
    DLPC900_USB_Init();

    // If the HID Interface has been connected already close it
    if(DLPC900_USB_IsConnected()) DLPC900_USB_Close();

    // Attempt to open the LCr6500 USB interface
    this->debug_.Msg("Opening USB HID interface...");
    DLPC900_USB_Open();

    // Workaround for DLPC350 error
    DLPC900_USB_Close();
    DLPC900_USB_Open();

    // Check if the connection succeeded
    if(!DLPC900_USB_IsConnected()){
        // Connection failed
        this->debug_.Msg("USB connection failed");
        DLPC900_USB_Close();
        DLPC900_USB_Exit();
        ret.AddError(LCR6500_CONNECTION_FAILED);
    }
    else{
        // Connection was successful
        this->debug_.Msg("USB connected successfully");
        this->SetID(id);
    }

    std::cout << "I2C Configure = " << DLPC900_I2CConfigure(1,0x18,400000)<<std::endl;

    unsigned char data[2];
    data[0] = 0x04;
    data[1] = 0x24;
    std::cout << "I2C Write = " << DLPC900_WriteI2CPassThrough(1,0x18,data,2) << std::endl;

//    DLPC900_SetMode(0);
//    DLPC900_SetInputSource(1, 0);
//    DLPC900_SetTPGSelect(5);


//    dlp::Time::Sleep::Seconds(2);

//    std::vector<std::string> image_list;
//    image_list.push_back("output/test.bmp");
//    image_list.push_back("output/test2.bmp");
//    image_list.push_back("output/test3.bmp");


//    std::cout << "Set mode = " << DLPC900_SetMode(OperatingMode::PATTERN_MODE_ON_THE_FLY) << std::endl;


//    std::cout << "Stop pattern sequence = " <<  DLPC900_PatternDisplay(Pattern::PatternStartStop::STOP) << std::endl;


//    std::cout << "Stop pattern sequence = " <<  DLPC900_PatternDisplay(Pattern::PatternStartStop::STOP) << std::endl;

//    std::cout << "Add to LUT = " <<  DLPC900_AddToPatLut(0,1000000, true, 1, 7, false, 0, true, 0, 0) << std::endl;
//    std::cout << "Add to LUT = " <<  DLPC900_AddToPatLut(1,1000000, true, 1, 7, false, 0, true, 0, 0) << std::endl; // blue
//    std::cout << "Add to LUT = " <<  DLPC900_AddToPatLut(2,1000000, true, 1, 7, false, 0, true, 0, 7) << std::endl; // green
//    std::cout << "Add to LUT = " <<  DLPC900_AddToPatLut(3,1000000, true, 1, 7, false, 0, true, 1, 15) << std::endl; // red
//    std::cout << "Add to LUT = " <<  DLPC900_AddToPatLut(4,1000000, true, 1, 7, false, 0, true, 1, 23) << std::endl; // blue
//    std::cout << "Add to LUT = " <<  DLPC900_AddToPatLut(5,1000000, true, 1, 7, false, 0, true, 2, 15) << std::endl; // blue
//    //std::cout << "Add to LUT = " <<  DLPC900_AddToPatLut(6,1000000, true, 1, 7, false, 0, true, 2, 0) << std::endl; // blue



//    std::cout << "Send LUT = " <<  DLPC900_SendPatLut() << std::endl;
//    std::cout << "Set pattern config = " <<  DLPC900_SetPatternConfig(6,0) << std::endl;


//    std::cout << this->UploadPrestoredCompressedImages(image_list).ToString() << std::endl;



//    std::cout << "Image should have been sent?" << std::endl;

//    std::cout << "Start pattern sequence = " <<  DLPC900_PatternDisplay(Pattern::PatternStartStop::START) << std::endl;


//    dlp::Time::Sleep::Seconds(5);



    return ret;
}

/** @brief  Disconnects from the DLP LightCrafter 6500 EVM
 * @retval  LCR6500_FIRMWARE_UPLOAD_IN_PROGRESS A firmware upload is in progress, do NOT send any commands until upload is complete!
 * @retval  LCR6500_NOT_CONNECTED               A LightCrafter 6500 EVM has NOT enumerated on the USB
 */
ReturnCode LCr6500::Disconnect(){
    ReturnCode ret;

//    // If A firmware upload is in progress and does not
//    // need a restard return and error
//    if( this->FirmwareUploadInProgress() && !this->firmware_upload_restart_needed){
//        this->debug_.Msg("Cannot connect because firmware is uploading");
//        return ret.AddError(LCR6500_FIRMWARE_UPLOAD_IN_PROGRESS);
//    }

    // If the HID Interface has been connected already close it
    if(!DLPC900_USB_IsConnected()){
        // The device wasn't connected
        this->debug_.Msg("Could NOT disconnect USB because it was NOT connected");
        ret.AddError(LCR6500_NOT_CONNECTED);
    }

    // Close the USB interface
    this->debug_.Msg("Closing USB HID interface");
    DLPC900_USB_Close();
    DLPC900_USB_Exit();

    return ret;
}

/** @brief  Returns true if the projector object is connected via USB. */
bool LCr6500::isConnected() const{
    bool ret = DLPC900_USB_IsConnected();

    if(ret){
        this->debug_.Msg("USB is connected");
    }
    else{
        this->debug_.Msg("USB is NOT connected");
    }

    return ret;
}

/** @brief  Sends a parameters object containing LightCrafter 6500 options
 * to a LightCrafter 6500 device.
 * @param[in]   arg_parameters  Takes an input object of dlp::Parameters type to set LightCrafter 6500 EVM
 *
 * @retval  LCR6500_FIRMWARE_UPLOAD_IN_PROGRESS     A firmware upload is in progress, do NOT send any commands until upload is complete!
 * @retval  LCR6500_NOT_CONNECTED                   A LightCrafter 6500 EVM has NOT been enumerated on the USB
 * @retval  LCR6500_SETUP_POWER_STANDBY_FAILED      The power mode could NOT be set on the LightCrafter 6500
 * @retval  LCR6500_SETUP_SHORT_AXIS_FLIP_FAILED    The short axis flip could NOT be set
 * @retval  LCR6500_SETUP_LONG_AXIS_FLIP_FAILED     The long axis flip could NOT be set
 * @retval  LCR6500_SETUP_LED_SEQUENCE_AND_ENABLES_FAILED   LED sequence and enables could NOT be set
 * @retval  LCR6500_SETUP_INVERT_LED_PWM_FAILED     The inversion of the LED PWM signal could NOT be set
 * @retval  LCR6500_SETUP_LED_CURRENTS_FAILED       The LED current values could NOT be set
 * @retval  LCR6500_SETUP_LED_RED_EDGE_DELAYS_FAILED        The red LED edge delay could NOT be set
 * @retval  LCR6500_SETUP_LED_GREEN_EDGE_DELAYS_FAILED      The green LED edge delay could NOT be set
 * @retval  LCR6500_SETUP_LED_BLUE_EDGE_DELAYS_FAILED       The blue LED edge delay could NOT be set
 * @retval  LCR6500_SETUP_INPUT_SOURCE_FAILED           The input source could NOT be set
 * @retval  LCR6500_SETUP_PARALLEL_PORT_CLOCK_FAILED    The parallel port clock could NOT be set
 * @retval  LCR6500_SETUP_DATA_SWAP_FAILED          The color data swap could NOT be set
 * @retval  LCR6500_SETUP_INVERT_DATA_FAILED        The inverted color image could NOT be set
 * @retval  LCR6500_SETUP_DISPLAY_MODE_FAILED       The display mode could NOT be set
 * @retval  LCR6500_SETUP_TEST_PATTERN_COLOR_FAILED The test pattern color could NOT be set
 * @retval  LCR6500_SETUP_POWER_STANDBY_FAILED      The power standby could NOT be set
 * @retval  LCR6500_SETUP_DISPLAY_MODE_FAILED       The display mode could NOT be set
 * @retval  LCR6500_SETUP_INPUT_SOURCE_FAILED       The input source could NOT be set
 * @retval  LCR6500_SETUP_TEST_PATTERN_FAILED       The test pattern could NOT be set
 * @retval  LCR6500_SETUP_FLASH_IMAGE_FAILED        The flash image could NOT be set
 * @retval  LCR6500_SETUP_TRIGGER_INPUT_1_DELAY_FAILED  The input delay for trigger 1 could NOT be set
 * @retval  LCR6500_SETUP_TRIGGER_OUTPUT_1_FAILED   The trigger 1 output could NOT be set
 * @retval  LCR6500_SETUP_TRIGGER_OUTPUT_2_FAILED   The trigger 2 output could NOT be set
 */
ReturnCode LCr6500::Setup(const dlp::Parameters &settings){
    ReturnCode ret;

    // Retreive pattern sequence timing settings
    if(settings.Contains(this->sequence_exposure_))
        settings.Get(&this->sequence_exposure_);

    if(settings.Contains(this->sequence_period_))
        settings.Get(&this->sequence_period_);

    if(settings.Contains(this->pattern_wait_for_trigger_))
        settings.Get(&this->pattern_wait_for_trigger_);

    // Retrive location of BMP files for white and black pattern
    if(settings.Contains(this->image_file_white_))
        settings.Get(&this->image_file_white_);
    if(settings.Contains(this->image_file_black_))
        settings.Get(&this->image_file_black_);

    // Compress those images
    this->CompressImageFile(this->image_file_white_.Get(),&this->pattern_image_white_);
    this->CompressImageFile(this->image_file_black_.Get(),&this->pattern_image_black_);

    // Retrive DMD global settins
    if(settings.Contains(this->invert_data_)){
        settings.Get(&this->invert_data_);
        DLPC900_SetInvertData(this->invert_data_.Get());
    }

    if(settings.Contains(this->short_axis_flip_)){
        settings.Get(&this->short_axis_flip_);
        DLPC900_SetLongAxisImageFlip(this->long_axis_flip_.Get());
    }

    if(settings.Contains(this->long_axis_flip_)){
       settings.Get(&this->long_axis_flip_);
       DLPC900_SetShortAxisImageFlip(this->short_axis_flip_.Get());
    }


    // Retrieve the LED settings
    if( settings.Contains(this->led_sequence_mode_) ||
        settings.Contains(this->led_red_enable_)    ||
        settings.Contains(this->led_green_enable_)  ||
        settings.Contains(this->led_blue_enable_)){

        settings.Get(&this->led_sequence_mode_);
        settings.Get(&this->led_red_enable_);
        settings.Get(&this->led_green_enable_);
        settings.Get(&this->led_blue_enable_);

        DLPC900_SetLedEnables(this->led_sequence_mode_.Get(),
                              this->led_red_enable_.Get(),
                              this->led_green_enable_.Get(),
                              this->led_blue_enable_.Get());
    }


    if(settings.Contains(this->led_invert_pwm_)){
        settings.Get(&this->led_invert_pwm_);
        DLPC900_SetLEDPWMInvert(this->led_invert_pwm_.Get());
    }


    if(settings.Contains(this->led_red_current_)   ||
       settings.Contains(this->led_green_current_) ||
       settings.Contains(this->led_blue_current_)){
        settings.Get(&this->led_red_current_);
        settings.Get(&this->led_green_current_);
        settings.Get(&this->led_blue_current_);
        DLPC900_SetLedCurrents(this->led_red_current_.Get(),
                               this->led_green_current_.Get(),
                               this->led_blue_current_.Get());
    }

    if(settings.Contains(this->led_red_edge_delay_rising_)  ||
       settings.Contains(this->led_red_edge_delay_falling_)){
        settings.Get(&this->led_red_edge_delay_rising_);
        settings.Get(&this->led_red_edge_delay_falling_);
        DLPC900_SetRedLEDStrobeDelay(this->led_red_edge_delay_rising_.Get(),
                                     this->led_red_edge_delay_falling_.Get());
    }

    if(settings.Contains(this->led_green_edge_delay_rising_)  ||
       settings.Contains(this->led_green_edge_delay_falling_)){
        settings.Get(&this->led_green_edge_delay_rising_);
        settings.Get(&this->led_green_edge_delay_falling_);
        DLPC900_SetGreenLEDStrobeDelay(this->led_green_edge_delay_rising_.Get(),
                                       this->led_green_edge_delay_falling_.Get());
    }

    if(settings.Contains(this->led_blue_edge_delay_rising_)  ||
       settings.Contains(this->led_blue_edge_delay_falling_)){
        settings.Get(&this->led_blue_edge_delay_rising_);
        settings.Get(&this->led_blue_edge_delay_falling_);
        DLPC900_SetBlueLEDStrobeDelay(this->led_blue_edge_delay_rising_.Get(),
                                      this->led_blue_edge_delay_falling_.Get());
    }

    // Setup the LEDs





    DLPC900_SetMode(OperatingMode::PATTERN_MODE_ON_THE_FLY);

    // Retrieve the trigger settings
    settings.Get(&this->trigger_in_1_delay_);
    settings.Get(&this->trigger_in_1_invert_);

    settings.Get(&this->trigger_in_2_invert_);

    settings.Get(&this->trigger_out_1_invert_);
    settings.Get(&this->trigger_out_1_rising_);
    settings.Get(&this->trigger_out_1_falling_);

    settings.Get(&this->trigger_out_2_invert_);
    settings.Get(&this->trigger_out_2_rising_);
    settings.Get(&this->trigger_out_2_falling_);

    // Setup the trigger
    DLPC900_SetTrigIn1Config(this->trigger_out_1_invert_.Get(),
                             this->trigger_in_1_delay_.Get());
    DLPC900_SetTrigIn2Config(this->trigger_in_2_invert_.Get());


    DLPC900_SetTrigOutConfig(1,
                             this->trigger_out_1_invert_.Get(),
                             this->trigger_out_1_rising_.Get(),
                             this->trigger_out_1_falling_.Get());
    DLPC900_SetTrigOutConfig(2,
                             this->trigger_out_2_invert_.Get(),
                             this->trigger_out_2_rising_.Get(),
                             this->trigger_out_2_falling_.Get());


//    // If A firmware upload is in progress return error
//    if(this->FirmwareUploadInProgress()){
//        this->debug_.Msg("Cannot connect because firmware is uploading");
//        return ret.AddError(LCR6500_FIRMWARE_UPLOAD_IN_PROGRESS);
//    }

//    // Check that LCr6500 is connected
//    if(!this->isConnected()){
//        this->debug_.Msg("Device is NOT connected");
//        return ret.AddError(LCR6500_NOT_CONNECTED);
//    }

//    // Stop the display
//    this->debug_.Msg("Stopping pattern display");
//    if(DLPC900_PatternDisplay(Pattern::DisplayControl::STOP)<0)
//        return ret.AddError(LCR6500_PATTERN_DISPLAY_FAILED);


//    // Should default values be set?
//    settings.Get(&this->use_default_);
//    if(this->use_default_.Get()){
//        this->debug_.Msg("Setting the default values...");
//        setup_default = this->use_default_.Get();
//    }

//    // Load DLPC350 related file locations
//    if(settings.Contains(this->DLPC900_firmware_))
//        settings.Get(&this->DLPC900_firmware_);

//    if(settings.Contains(this->DLPC900_flash_parameters_))
//        settings.Get(&this->DLPC900_flash_parameters_);

//    if(settings.Contains(this->pattern_sequence_firmware_))
//        settings.Get(&this->pattern_sequence_firmware_);

//    if(settings.Contains(this->verify_image_load_))
//        settings.Get(&this->verify_image_load_);


//    // Check if parameters contains setup instructions
//    if(setup_default || settings.Contains(this->power_standby_)){
//        settings.Get(&this->power_standby_);

//        // Send command and check if it succeeded
//        this->debug_.Msg("Setting power mode = " + this->power_standby_.GetEntryValue());
//        if(DLPC900_SetPowerMode(this->power_standby_.Get()) < 0){
//            this->debug_.Msg("Setting power mode FAILED");
//            return ret.AddError(LCR6500_SETUP_POWER_STANDBY_FAILED);
//        }
//    }


//    if(setup_default || settings.Contains(this->short_axis_flip_)){
//        // Get the value
//        settings.Get(&this->short_axis_flip_);
//        // Send command and check if it succeeded
//        this->debug_.Msg("Setting short axis image flip = " + this->short_axis_flip_.GetEntryValue());
//        if(DLPC900_SetShortAxisImageFlip(this->short_axis_flip_.Get()) < 0 ){
//            this->debug_.Msg("Setting short axis image flip FAILED");
//            return ret.AddError(LCR6500_SETUP_SHORT_AXIS_FLIP_FAILED);
//        }
//    }

//    if(setup_default || settings.Contains(this->long_axis_flip_)){
//        // Get the value
//        settings.Get(&this->long_axis_flip_);

//        // Send command and check if it succeeded
//        this->debug_.Msg("Setting long axis image flip = " + this->long_axis_flip_.GetEntryValue());
//        if(DLPC900_SetLongAxisImageFlip(this->long_axis_flip_.Get()) < 0 ){
//            this->debug_.Msg("Setting long axis image flip FAILED");
//            return ret.AddError(LCR6500_SETUP_LONG_AXIS_FLIP_FAILED);
//        }
//    }

//    if(setup_default || settings.Contains(this->led_sequence_mode_)
//                     || settings.Contains(this->led_red_enable_)
//                     || settings.Contains(this->led_green_enable_)
//                     || settings.Contains(this->led_blue_enable_)){
//        // Get the values
//        settings.Get(&this->led_sequence_mode_);
//        settings.Get(&this->led_red_enable_);
//        settings.Get(&this->led_green_enable_);
//        settings.Get(&this->led_blue_enable_);

//        // Send command and check if it succeeded
//        this->debug_.Msg("Setting LED auto sequence = " + this->led_sequence_mode_.GetEntryValue());
//        this->debug_.Msg("Setting LED enable red    = " + this->led_red_enable_.GetEntryValue());
//        this->debug_.Msg("Setting LED enable green  = " + this->led_green_enable_.GetEntryValue());
//        this->debug_.Msg("Setting LED enable blue   = " + this->led_blue_enable_.GetEntryValue());
//        if(DLPC900_SetLedEnables( this->led_sequence_mode_.Get(),
//                                  this->led_red_enable_.Get(),
//                                  this->led_green_enable_.Get(),
//                                  this->led_blue_enable_.Get()) < 0){
//            this->debug_.Msg("Setting LED sequence and enables FAILED");
//            return ret.AddError(LCR6500_SETUP_LED_SEQUENCE_AND_ENABLES_FAILED);
//        }
//    }

//    if(setup_default || settings.Contains(this->led_invert_pwm_)){
//        // Get the value
//        settings.Get(&this->led_invert_pwm_);
//        // Send command and check if it succeeded
//        if(DLPC900_SetLEDPWMInvert(this->led_invert_pwm_.Get()) < 0)
//            return ret.AddError(LCR6500_SETUP_INVERT_LED_PWM_FAILED);
//    }

//    if(setup_default || settings.Contains(this->led_red_current_)
//                     || settings.Contains(this->led_green_current_)
//                     || settings.Contains(this->led_blue_current_)){
//        // Get the value
//        settings.Get(&this->led_red_current_);
//        settings.Get(&this->led_green_current_);
//        settings.Get(&this->led_blue_current_);

//        // Send command and check if it succeeded
//        // Invert the currents for LightCrafter 6500 circuitry
//        if(DLPC900_SetLedCurrents( Led::MAXIMUM_CURRENT - this->led_red_current_.Get(),
//                                   Led::MAXIMUM_CURRENT - this->led_green_current_.Get(),
//                                   Led::MAXIMUM_CURRENT - this->led_blue_current_.Get())
//                                   < 0)
//            return ret.AddError(LCR6500_SETUP_LED_CURRENTS_FAILED);

//    }


//    if(setup_default || settings.Contains(this->led_red_edge_delay_falling_)
//                     || settings.Contains(this->led_red_edge_delay_rising_)){
//        // Get the settings
//        settings.Get(&this->led_red_edge_delay_rising_);
//        settings.Get(&this->led_red_edge_delay_falling_);

//        // Send command and check if it succeeded
//        if(DLPC900_SetRedLEDStrobeDelay( this->led_red_edge_delay_rising_.Get(),
//                                         this->led_red_edge_delay_falling_.Get()) < 0)
//            return ret.AddError(LCR6500_SETUP_LED_RED_EDGE_DELAYS_FAILED);

//    }

//    if(setup_default || settings.Contains(this->led_green_edge_delay_falling_)
//                     || settings.Contains(this->led_green_edge_delay_rising_)){
//        // Get the settings
//        settings.Get(&this->led_green_edge_delay_rising_);
//        settings.Get(&this->led_green_edge_delay_falling_);

//        // Send command and check if it succeeded
//        if(DLPC900_SetGreenLEDStrobeDelay( this->led_green_edge_delay_rising_.Get(),
//                                         this->led_green_edge_delay_falling_.Get()) < 0)
//            return ret.AddError(LCR6500_SETUP_LED_GREEN_EDGE_DELAYS_FAILED);

//    }

//    if(setup_default || settings.Contains(this->led_blue_edge_delay_falling_)
//                     || settings.Contains(this->led_blue_edge_delay_rising_)){
//        // Get the settings
//        settings.Get(&this->led_blue_edge_delay_rising_);
//        settings.Get(&this->led_blue_edge_delay_falling_);

//        // Send command and check if it succeeded
//        if(DLPC900_SetBlueLEDStrobeDelay( this->led_blue_edge_delay_rising_.Get(),
//                                         this->led_blue_edge_delay_falling_.Get()) < 0)
//            return ret.AddError(LCR6500_SETUP_LED_BLUE_EDGE_DELAYS_FAILED);

//    }

//    if(setup_default || settings.Contains(this->input_source_)
//                     || settings.Contains(this->parallel_port_width_)){
//        // Get the value
//        settings.Get(&this->input_source_);
//        settings.Get(&this->parallel_port_width_);
//        if(DLPC900_SetInputSource(this->input_source_.Get(),
//                                  this->parallel_port_width_.Get()) < 0)
//            return ret.AddError(LCR6500_SETUP_INPUT_SOURCE_FAILED);
//    }

//    if(setup_default || settings.Contains(this->parallel_port_clock_)){
//        // Get the value
//        settings.Get(&this->parallel_port_clock_);
//        if(DLPC900_SetPortClock(this->parallel_port_clock_.Get()) < 0)
//            return ret.AddError(LCR6500_SETUP_PARALLEL_PORT_CLOCK_FAILED);
//    }


//    if(setup_default || settings.Contains(this->parallel_data_swap_)){
//        // Get the value
//        settings.Get(&this->parallel_data_swap_);
//        if(DLPC900_SetDataChannelSwap(LCr6500::Video::DataSwap::Port::PARALLEL_INTERFACE,
//                                      this->parallel_data_swap_.Get()) < 0)
//            return ret.AddError(LCR6500_SETUP_DATA_SWAP_FAILED);
//    }


//    if(setup_default || settings.Contains(this->invert_data_)){
//        // Get the value and check that it existed
//        settings.Get(&this->invert_data_);

//        // Send command and check if it succeeded
//        if(DLPC900_SetInvertData(this->invert_data_.Get()) < 0)
//            return ret.AddError(LCR6500_SETUP_INVERT_DATA_FAILED);
//    }

//    if(setup_default || settings.Contains(this->display_mode_)){
//        bool mode_previous;

//        // Set power mode to normal
//        if(DLPC900_SetPowerMode(PowerStandbyMode::NORMAL) < 0)
//            return ret.AddError(LCR6500_SETUP_POWER_STANDBY_FAILED);

//        // Get the pattern
//        settings.Get(&this->display_mode_);

//        // Check the current mode
//        if(DLPC900_GetMode(&mode_previous)<0)
//            return ret.AddError(LCR6500_GET_OPERATING_MODE_FAILED);

//        // The current mode is different from the requested setting change it
//        if(mode_previous != this->display_mode_.Get()){
//            if(DLPC900_SetMode(this->display_mode_.Get()) < 0)
//                return ret.AddError(LCR6500_SETUP_DISPLAY_MODE_FAILED);
//        }
//    }

//    if(setup_default || settings.Contains(this->test_pattern_foreground_red)
//                     || settings.Contains(this->test_pattern_foreground_green)
//                     || settings.Contains(this->test_pattern_foreground_blue)
//                     || settings.Contains(this->test_pattern_background_red)
//                     || settings.Contains(this->test_pattern_background_green)
//                     || settings.Contains(this->test_pattern_background_blue))
//    {

//        // Get the test pattern foreground and background colors
//        settings.Get(&this->test_pattern_foreground_red);
//        settings.Get(&this->test_pattern_foreground_green);
//        settings.Get(&this->test_pattern_foreground_blue);
//        settings.Get(&this->test_pattern_background_red);
//        settings.Get(&this->test_pattern_background_green);
//        settings.Get(&this->test_pattern_background_blue);

//        // Send command to LCr6500
//        if(DLPC900_SetTPGColor(this->test_pattern_foreground_red.Get(),
//                               this->test_pattern_foreground_green.Get(),
//                               this->test_pattern_foreground_blue.Get(),
//                               this->test_pattern_background_red.Get(),
//                               this->test_pattern_background_green.Get(),
//                               this->test_pattern_background_blue.Get()) < 0)
//            return ret.AddError(LCR6500_SETUP_TEST_PATTERN_COLOR_FAILED);
//    }

//    if(settings.Contains(Parameters::VideoTestPattern())){
//        // Get the value
//        Parameters::VideoTestPattern test_pattern;

//        // Get the test pattern
//        settings.Get(&test_pattern);

//        // Set the current power mode
//        if(DLPC900_SetPowerMode(PowerStandbyMode::NORMAL) < 0)
//            return ret.AddError(LCR6500_SETUP_POWER_STANDBY_FAILED);


//        // Check the display mode
//        bool mode;
//        if(DLPC900_GetMode(&mode)<0)
//            return ret.AddError(LCR6500_GET_OPERATING_MODE_FAILED);

//        // If the display mode is NOT video switch it
//        if(mode != OperatingMode::VIDEO){
//            if(DLPC900_SetMode(OperatingMode::VIDEO) < 0)
//                return ret.AddError(LCR6500_SETUP_DISPLAY_MODE_FAILED);
//        }

//        dlp::Time::Sleep::Milliseconds(10);
//        if(DLPC900_SetInputSource(Video::InputSource::INTERNAL_TEST_PATTERNS,
//                                  Video::ParallelPortWidth::BITS_30) < 0)
//            return ret.AddError(LCR6500_SETUP_INPUT_SOURCE_FAILED);

//        dlp::Time::Sleep::Milliseconds(10);

//        if(DLPC900_SetTPGColor(0,0,0,1023,1023,1023) < 0)
//            return ret.AddError(LCR6500_SETUP_TEST_PATTERN_COLOR_FAILED);

//        dlp::Time::Sleep::Milliseconds(10);

//        if(DLPC900_SetTPGSelect(test_pattern.Get()) < 0)
//            return ret.AddError(LCR6500_SETUP_TEST_PATTERN_FAILED);

//    }



//    if(settings.Contains(Parameters::VideoFlashImage())){
//        // Get the value
//        Parameters::VideoFlashImage flash_image;
//        bool         mode;
//        unsigned int source;
//        unsigned int portWidth;

//        settings.Get(&flash_image);

//        if(DLPC900_SetPowerMode(PowerStandbyMode::NORMAL) < 0)
//            return ret.AddError(LCR6500_SETUP_POWER_STANDBY_FAILED);


//        // Check the display mode
//        if(DLPC900_GetMode(&mode)<0)
//            return ret.AddError(LCR6500_GET_OPERATING_MODE_FAILED);

//        // If the display mode is NOT video switch it
//        if(mode != OperatingMode::VIDEO){
//            if(DLPC900_SetMode(OperatingMode::VIDEO) < 0)
//                return ret.AddError(LCR6500_SETUP_DISPLAY_MODE_FAILED);
//        }

//        dlp::Time::Sleep::Milliseconds(10);
//        DLPC900_GetInputSource(&source, &portWidth);
//        if (source != Video::InputSource::FLASH_IMAGES){
//            if(DLPC900_SetInputSource(Video::InputSource::FLASH_IMAGES,
//                                      Video::ParallelPortWidth::BITS_30) < 0)
//                return ret.AddError(LCR6500_SETUP_INPUT_SOURCE_FAILED);
//        }

//        dlp::Time::Sleep::Milliseconds(10);
//        if(DLPC900_LoadImageIndex(flash_image.Get()) < 0)
//            return ret.AddError(LCR6500_SETUP_FLASH_IMAGE_FAILED);

//    }


//    if(settings.Contains(this->trigger_source_)){

//        // Get the value and check that it existed
//        settings.Get(&this->trigger_source_);

//    }

//    if(settings.Contains(this->sequence_prepared_)){

//        // Get the value and check that it existed
//        settings.Get(&this->sequence_prepared_);

//    }

//    if(settings.Contains(this->sequence_exposure_)){

//        // Get the value and check that it existed
//        settings.Get(&this->sequence_exposure_);

//    }

//    if(settings.Contains(this->sequence_period_)){

//        // Get the value and check that it existed
//        settings.Get(&this->sequence_period_);

//    }

//    if(settings.Contains(this->trigger_in_1_delay_)){

//        // Get the value and check that it existed
//        settings.Get(&this->trigger_in_1_delay_);

//        if(DLPC900_PatternDisplay(Pattern::DisplayControl::STOP) < 0)
//            return ret.AddError(LCR6500_PATTERN_DISPLAY_FAILED);

//        // Send command and check if it succeeded
//        if(DLPC900_SetTrigIn1Delay(this->trigger_in_1_delay_.Get()) == -1 )
//            return ret.AddError(LCR6500_SETUP_TRIGGER_INPUT_1_DELAY_FAILED);

//    }


//    if(   settings.Contains(this->trigger_out_1_invert_)
//       || settings.Contains(this->trigger_out_1_rising_)
//       || settings.Contains(this->trigger_out_1_falling_)){

//        // Get the value and check that it existed
//        settings.Get(&this->trigger_out_1_invert_);
//        settings.Get(&this->trigger_out_1_rising_);
//        settings.Get(&this->trigger_out_1_falling_);

//        if(DLPC900_PatternDisplay(Pattern::DisplayControl::STOP) < 0)
//            return ret.AddError(LCR6500_PATTERN_DISPLAY_FAILED);

//        // Send command and check if it succeeded
//        if(DLPC900_SetTrigOutConfig( LCR6500_TRIGGER_OUT_1,
//                                     this->trigger_out_1_invert_.Get(),
//                                     this->trigger_out_1_rising_.Get(),
//                                     this->trigger_out_1_falling_.Get()) == -1 )
//            return ret.AddError(LCR6500_SETUP_TRIGGER_OUTPUT_1_FAILED);
//    }


//    if(   settings.Contains(this->trigger_out_2_invert_)
//       || settings.Contains(this->trigger_out_2_rising_)){

//        // Get the value and check that it existed
//        settings.Get(&this->trigger_out_2_invert_);
//        settings.Get(&this->trigger_out_2_rising_);

//        if(DLPC900_PatternDisplay(Pattern::DisplayControl::STOP) < 0)
//            return ret.AddError(LCR6500_PATTERN_DISPLAY_FAILED);

//        // Send command and check if it succeeded
//        if(DLPC900_SetTrigOutConfig( LCR6500_TRIGGER_OUT_2,
//                                     this->trigger_out_2_invert_.Get(),
//                                     this->trigger_out_2_rising_.Get(),
//                                     0) == -1 )
//            return ret.AddError(LCR6500_SETUP_TRIGGER_OUTPUT_2_FAILED);
//    }

    this->is_setup_ = true;

    return ret;
}


/** @brief      Retrieves object settings
 * @param[out]  ret_parameters  Pointer to return \ref dlp::Parameters object containing setup for LightCrafter 6500
 *
 * @retval  LCR6500_NULL_POINT_ARGUMENT_PARAMETERS  The pointer is null
 * @retval  DLP_PLATFORM_NOT_SETUP                  The LightCrafter 6500 has NOT been set up
 */
ReturnCode LCr6500::GetSetup(dlp::Parameters* settings) const{
    ReturnCode ret;

//    // Check that pointer is NOT null
//    if(!settings)
//        return ret.AddError(LCR6500_NULL_POINT_ARGUMENT_PARAMETERS);

//    if(!this->isPlatformSetup())
//        return ret.AddError(DLP_PLATFORM_NOT_SETUP);

//    // Clear the parameters list
//    settings->Clear();

//    settings->Set(this->DLPC900_firmware_);
//    settings->Set(this->DLPC900_flash_parameters_);
//    settings->Set(this->pattern_sequence_firmware_);
//    settings->Set(this->use_default_);
//    settings->Set(this->power_standby_);
//    settings->Set(this->display_mode_);
//    settings->Set(this->input_source_);
//    settings->Set(this->parallel_port_width_);
//    settings->Set(this->parallel_port_clock_);
//    settings->Set(this->parallel_data_swap_);
//    settings->Set(this->invert_data_);
//    settings->Set(this->short_axis_flip_);
//    settings->Set(this->long_axis_flip_);
//    settings->Set(this->led_sequence_mode_);
//    settings->Set(this->led_invert_pwm_);
//    settings->Set(this->led_red_enable_);
//    settings->Set(this->led_red_current_);
//    settings->Set(this->led_red_edge_delay_rising_);
//    settings->Set(this->led_red_edge_delay_falling_);
//    settings->Set(this->led_green_enable_);
//    settings->Set(this->led_green_current_);
//    settings->Set(this->led_green_edge_delay_rising_);
//    settings->Set(this->led_green_edge_delay_falling_);
//    settings->Set(this->led_blue_enable_);
//    settings->Set(this->led_blue_current_);
//    settings->Set(this->led_blue_edge_delay_rising_);
//    settings->Set(this->led_blue_edge_delay_falling_);
//    settings->Set(this->trigger_in_1_delay_);
//    settings->Set(this->trigger_out_1_invert_);
//    settings->Set(this->trigger_out_2_invert_);
//    settings->Set(this->trigger_out_1_rising_);
//    settings->Set(this->trigger_out_1_falling_);
//    settings->Set(this->trigger_out_2_rising_);
//    settings->Set(this->verify_image_load_);

    return ret;
}






ReturnCode  LCr6500::UploadPrestoredCompressedImages(const unsigned char &start_index, const unsigned char &last_index){
    ReturnCode ret;


    // If an image upload is already in progress, do NOT send any commands until upload is complete!
    if(this->UploadImages_InProgress()){
        this->debug_.Msg("Cannot upload firmware because upload already in progress");
        return ret.AddError(LCR6500_FIRMWARE_UPLOAD_IN_PROGRESS);                 /** @todo update error code image upload  */
    }
    else{
        // Set the firmware uploading flag since upload has NOT started yet
        while(this->image_upload_in_progress.test_and_set()){};
    }

    // Set the mode to pattern sequence on-the-fly
    if( DLPC900_SetMode(OperatingMode::PATTERN_MODE_ON_THE_FLY) < 0)
        return ret.AddError(LCR6500_SET_PATTERN_DISPLAY_MODE_FAILED);

    // Stop the display
    if( DLPC900_PatternDisplay(Pattern::PatternStartStop::STOP) < 0 )
        return ret.AddError(LCR6500_PATTERN_DISPLAY_FAILED);

    // Reset the upload percentage
    this->image_upload_percent_complete_ = 0;

    // Check that requested number of images is below maximum
    if((last_index - start_index) > MAX_IMAGE_ENTRIES_ON_THE_FLY)
        return ret.AddError(LCR6500_IMAGE_LIST_TOO_LONG);

    // Sum the total data to upload
    this->image_upload_total_data_size_      = 0;
    this->image_upload_total_data_size_sent_ = 0;

    for(int iImageIndex = last_index; iImageIndex >= start_index; iImageIndex--){
        this->image_upload_total_data_size_ += this->compressed_images_.at(iImageIndex).sizeBitmap1;
    }

    long long total_data_size = this->image_upload_total_data_size_;
    this->debug_.Msg("Total compressed image data bytes to upload = " + dlp::Number::ToString(total_data_size));

    // Upload the images in reverse (upload last iamge first and first image last)
    for(int iImageIndex = last_index; iImageIndex >= start_index; iImageIndex--){

        this->debug_.Msg("Uploading prestored compressed image " + dlp::Number::ToString(iImageIndex));
        this->UploadCompressedImage(iImageIndex - start_index,  // This adjusts for the offset so the last image sent is image index 0
                                    this->compressed_images_.at(iImageIndex).bitmapImage1,
                                    this->compressed_images_.at(iImageIndex).sizeBitmap1);
    }

    this->image_upload_in_progress.clear();

    return ret;
}


ReturnCode LCr6500::UploadCompressedImage(const unsigned char &fw_image_index, unsigned char *compressed_image_byte_array, const int &compressed_image_data_size){
    ReturnCode ret;

    // Retrieve new data pointer and size information
    int  compressed_image_data_to_upload = compressed_image_data_size;
    bool dlpc900_master                  = true;

    // Initialize controller to receive image
    if(DLPC900_InitPatternMemLoad(dlpc900_master, fw_image_index, compressed_image_data_size) < 0)
        return ret.AddError(LCR6500_PATTERN_SEQUENCE_NOT_PREPARED);

    // Upload the image data
    int data_uploaded = 0;
    while(compressed_image_data_to_upload > 0)
    {
        data_uploaded = DLPC900_pattenMemLoad(dlpc900_master,
                                              compressed_image_byte_array + (compressed_image_data_size - compressed_image_data_to_upload),
                                              compressed_image_data_to_upload);

        // Check that data was uploaded
        if (data_uploaded < 0)
            return ret.AddError(LCR6500_FIRMWARE_UPLOAD_FAILED);

        // Adjust data pointer to send next packet of data
        compressed_image_data_to_upload -= data_uploaded;

        // Update the total data count
        this->image_upload_total_data_size_sent_ += data_uploaded;

        // Check that there is still image data to upload
        if (compressed_image_data_to_upload < 0)
            compressed_image_data_to_upload = 0;

        // Update the percent complete
        this->debug_.Msg(1, "Data left to download = " + dlp::Number::ToString(compressed_image_data_to_upload));
    }

    return ret;
}






bool LCr6500::UploadImages_InProgress(){
    if(!this->image_upload_in_progress.test_and_set()){
        // Firmware is NOT uploading, clear flag since the above
        // methods sets it
        this->image_upload_in_progress.clear();
        return false;
    }
    else{
        // Firmware upload is in progress
        return true;
    }
}

float LCr6500::UploadImages_PercentComplete(){
    return this->image_upload_percent_complete_;
}















/** @brief  Projects a continuous solid white pattern from a connected LightCrafter 6500
 *
 * @retval  LCR6500_FIRMWARE_UPLOAD_IN_PROGRESS     A firmware upload is in progress, do NOT send any commands until upload is complete!
 * @retval  LCR6500_NOT_CONNECTED                   A LightCrafter 6500 EVM has NOT enumerated on the USB
 */
ReturnCode LCr6500::ProjectSolidWhitePattern(){
    ReturnCode ret;

//    // If A firmware upload is in progress return error
//    if(this->FirmwareUploadInProgress()){
//        this->debug_.Msg("Cannot connect because firmware is uploading");
//        return ret.AddError(LCR6500_FIRMWARE_UPLOAD_IN_PROGRESS);
//    }


    // Change device to pattern sequence mode
    this->debug_.Msg("Setting device pattern sequence on-the-fly mode...");
    if(DLPC900_SetMode(OperatingMode::PATTERN_MODE_ON_THE_FLY) < 0)
        return ret.AddError(LCR6500_SET_OPERATING_MODE_FAILED);

    // Stop the sequence if something is already running
    this->debug_.Msg("Stop the pattern sequence...");
    if(DLPC900_PatternDisplay(Pattern::PatternStartStop::STOP) < 0)
          return ret.AddError(LCR6500_PATTERN_DISPLAY_FAILED);





    DLPC900_ClearPatLut();
    if(DLPC900_AddToPatLut(0,
                           this->sequence_exposure_.Get(),
                           true,
                           1,
                           dlp::LCr6500::Pattern::Led::WHITE,
                           this->pattern_wait_for_trigger_.Get(),
                           this->sequence_period_.Get() - this->sequence_exposure_.Get(),
                           true,
                           0,
                           0)<0)
            return ret.AddError(LCR6500_ADD_EXP_LUT_ENTRY_FAILED);


    if(DLPC900_SendPatLut() < 0)
        return ret.AddError(LCR6500_SEND_EXP_PATTERN_LUT_FAILED);  ///////////////////////////////////////////////////////////////////////

    // Set the pattern configurationand tell controller to repeat
    if(DLPC900_SetPatternConfig(1, 0) < 0)
        return ret.AddError(LCR6500_SET_PATTERN_DISPLAY_MODE_FAILED); ///////////////////////////////////////////

    this->UploadCompressedImage(0,
                                this->pattern_image_white_.bitmapImage1,
                                this->pattern_image_white_.sizeBitmap1);

    // Start the sequence
    if(DLPC900_PatternDisplay(Pattern::PatternStartStop::START) < 0)
        return ret.AddError(LCR6500_PATTERN_DISPLAY_FAILED);

    return ret;
}

/** @brief  Projects a continuous solid black pattern from a connected LightCrafter 6500
 *
 * @retval  LCR6500_FIRMWARE_UPLOAD_IN_PROGRESS     A firmware upload is in progress, do NOT send any commands until upload is complete!
 * @retval  LCR6500_NOT_CONNECTED                   A LightCrafter 6500 EVM has NOT enumerated on the USB
 */
ReturnCode LCr6500::ProjectSolidBlackPattern(){
    ReturnCode ret;

//    // If A firmware upload is in progress return error
//    if(this->FirmwareUploadInProgress()){
//        this->debug_.Msg("Cannot connect because firmware is uploading");
//        return ret.AddError(LCR6500_FIRMWARE_UPLOAD_IN_PROGRESS);
//    }

    // Change device to pattern sequence mode
    this->debug_.Msg("Setting device pattern sequence on-the-fly mode...");
    if(DLPC900_SetMode(OperatingMode::PATTERN_MODE_ON_THE_FLY) < 0)
        return ret.AddError(LCR6500_SET_OPERATING_MODE_FAILED);

    // Stop the sequence if something is already running
    this->debug_.Msg("Stop the pattern sequence...");
    if(DLPC900_PatternDisplay(Pattern::PatternStartStop::STOP) < 0)
          return ret.AddError(LCR6500_PATTERN_DISPLAY_FAILED);


    DLPC900_ClearPatLut();
    if(DLPC900_AddToPatLut(0,
                           this->sequence_exposure_.Get(),
                           true,
                           1,
                           dlp::LCr6500::Pattern::Led::WHITE,
                           this->pattern_wait_for_trigger_.Get(),
                           this->sequence_period_.Get() - this->sequence_exposure_.Get(),
                           true,
                           0,
                           0)<0)
            return ret.AddError(LCR6500_ADD_EXP_LUT_ENTRY_FAILED);


    if(DLPC900_SendPatLut() < 0)
        return ret.AddError(LCR6500_SEND_EXP_PATTERN_LUT_FAILED);  ///////////////////////////////////////////////////////////////////////

    // Set the pattern configurationand tell controller to repeat
    if(DLPC900_SetPatternConfig(1, 0) < 0)
        return ret.AddError(LCR6500_SET_PATTERN_DISPLAY_MODE_FAILED); ///////////////////////////////////////////

    this->UploadCompressedImage(0,
                                this->pattern_image_black_.bitmapImage1,
                                this->pattern_image_black_.sizeBitmap1);

    // Start the sequence
    if(DLPC900_PatternDisplay(Pattern::PatternStartStop::START) < 0)
        return ret.AddError(LCR6500_PATTERN_DISPLAY_FAILED);



    return ret;
}

/** @brief      Checks for valid pattern settings in \ref dlp::Pattern object
 * @param[in]   arg_pattern \ref dlp::Pattern object to verify
 *
 * @retval  PATTERN_EXPOSURE_TOO_SHORT          Pattern exposure time is set too short
 * @retval  PATTERN_EXPOSURE_TOO_LONG           Pattern exposure time is set too long
 * @retval  PATTERN_BITDEPTH_INVALID            The pattern's bit depth is an invalid value
 * @retval  PATTERN_EXPOSURE_INVALID            Pattern exposure period is an invalid value
 * @retval  FILE_DOES_NOT_EXIST                 The pattern's image file could NOT be found
 * @retval  LCR6500_IMAGE_RESOLUTION_INVALID    The image is of an invalid resolution. Please use the native LCr6500 resolution 912x1140
 * @retval  LCR6500_IMAGE_FORMAT_INVALID        The image format does NOT match the pattern settings color type
 * @retval  LCR6500_pattern_bitplane_PARAMETER_MISSING        The pattern is missing the number parameter
 * @retval  LCR6500_PATTERN_FLASH_INDEX_PARAMETER_MISSING   The pattern is missing the flash image index
 * @retval  PATTERN_DATA_TYPE_INVALID           The pattern isn't of a valid data type
 */
ReturnCode LCr6500::PatternSettingsValid(dlp::Pattern &arg_pattern){
    ReturnCode ret;

    unsigned int exposure   = arg_pattern.exposure;
    unsigned int period     = arg_pattern.period;


    // Check that exposure time is equal to period time or at meets delta requirement
    if((exposure == period) ||
       (exposure <= (period - dlp::LCr6500::Pattern::Exposure::PERIOD_DIFFERENCE_MINIMUM))){


        if(arg_pattern.bitdepth != dlp::Pattern::Bitdepth::INVALID){
            if(exposure <  dlp::LCr6500::Pattern::Exposure::MININUM(arg_pattern.bitdepth ))
                ret.AddError(PATTERN_EXPOSURE_TOO_SHORT);
            if(exposure > dlp::LCr6500::Pattern::Exposure::MAXIMUM)
                ret.AddError(PATTERN_EXPOSURE_TOO_LONG);
        }
        else{
            ret.AddError(PATTERN_BITDEPTH_INVALID);
        }
    }
    else{
        // Exposure period was invalid
        ret.AddError(PATTERN_EXPOSURE_INVALID);
    }

    // Check any errors were present
    if(!ret.hasErrors()){
        Image::Format temp_format;

        if(arg_pattern.data_type == dlp::Pattern::DataType::IMAGE_FILE){
            Image temp_image;

            // Check if file exists
            if(!dlp::File::Exists(arg_pattern.image_file))
                return ret.AddError(FILE_DOES_NOT_EXIST);

            // Load the image file
            temp_image.Load(arg_pattern.image_file);

            // Check image resolution
            if(!this->ImageResolutionCorrect(temp_image))
                return ret.AddError(LCR6500_IMAGE_RESOLUTION_INVALID);

            // Get the image data format
            temp_image.GetDataFormat(&temp_format);

            // If RGB pattern make sure image is RGB also
            if(arg_pattern.color == dlp::Pattern::Color::RGB){
                if(temp_format != Image::Format::RGB_UCHAR)
                    return ret.AddError(LCR6500_IMAGE_FORMAT_INVALID);
            }

            // Clear the image
            temp_image.Clear();
        }
        else if(arg_pattern.data_type == dlp::Pattern::DataType::IMAGE_DATA){
            // Check image resolution
            if(!this->ImageResolutionCorrect(arg_pattern.image_data)){
                return ret.AddError(LCR6500_IMAGE_RESOLUTION_INVALID);
            }

            // Get the image data format
            arg_pattern.image_data.GetDataFormat(&temp_format);

            // If RGB pattern make sure image is also RGB
            if(arg_pattern.color == dlp::Pattern::Color::RGB){
                if(temp_format != Image::Format::RGB_UCHAR)
                    return ret.AddError(LCR6500_IMAGE_FORMAT_INVALID);
            }
        }
        else if(arg_pattern.data_type == dlp::Pattern::DataType::PARAMETERS){
            // No Image resolution check required

            // Check if pattern is RGB or monochrome
            if(arg_pattern.color != dlp::Pattern::Color::RGB){
                // Patern is monochrome

                // Check for pattern number
                if(!arg_pattern.parameters.Contains(Parameters::PatternBitplane()))
                    return ret.AddError(LCR6500_PATTERN_NUMBER_PARAMETER_MISSING);

                // Check for image number
                if(!arg_pattern.parameters.Contains(Parameters::PatternImageIndex()))
                    return ret.AddError(LCR6500_PATTERN_FLASH_INDEX_PARAMETER_MISSING);
            }
            else{
                // Pattern is RGB

                // Check for red channel parameters

                    // Check for pattern number
                    if(!arg_pattern.parameters.Contains(Parameters::PatternBitplaneRed()))
                        return ret.AddError(LCR6500_PATTERN_NUMBER_PARAMETER_MISSING);

                    // Check for image number
                    if(!arg_pattern.parameters.Contains(Parameters::PatternImageIndexRed()))
                        return ret.AddError(LCR6500_PATTERN_FLASH_INDEX_PARAMETER_MISSING);

                // Check for green channel paramaters

                    // Check for pattern number
                    if(!arg_pattern.parameters.Contains(Parameters::PatternBitplaneGreen()))
                        return ret.AddError(LCR6500_PATTERN_NUMBER_PARAMETER_MISSING);

                    // Check for image number
                    if(!arg_pattern.parameters.Contains(Parameters::PatternImageIndexGreen()))
                        return ret.AddError(LCR6500_PATTERN_FLASH_INDEX_PARAMETER_MISSING);

                // Check for blue channel parameters

                    // Check for pattern number
                    if(!arg_pattern.parameters.Contains(Parameters::PatternBitplaneBlue()))
                        return ret.AddError(LCR6500_PATTERN_NUMBER_PARAMETER_MISSING);

                    // Check for image number
                    if(!arg_pattern.parameters.Contains(Parameters::PatternImageIndexBlue()))
                        return ret.AddError(LCR6500_PATTERN_FLASH_INDEX_PARAMETER_MISSING);
            }
        }
        else{
            return ret.AddError(PATTERN_DATA_TYPE_INVALID);
        }
    }
    return ret;
}

/** @brief  Creates, sends, and starts a sequence of patterns using the
 *          LightCrafter 6500 look up table format
 *  \note   This function should only be used by experienced DLPC350 programmers
 *  \note   The supplied \ref dlp::Pattern::Sequence MUST use LCr6500 parameters
 * @param[in]   arg_pattern_sequence    object of \ref dlp::Pattern::Sequence type containing sequence
 *
 * @retval  LCR6500_FIRMWARE_UPLOAD_IN_PROGRESS             A firmware upload is in progress, do NOT send any commands until upload is complete!
 * @retval  LCR6500_NOT_CONNECTED                           The LightCrafter 6500 EVM is NOT connected
 * @retval  PATTERN_SEQUENCE_EMPTY                          The pattern sequence sent contains no patterns
 * @retval  PATTERN_SEQUENCE_EXPOSURES_NOT_EQUAL            The exposure times are NOT equal for each pattern in the sequence
 * @retval  PATTERN_SEQUENCE_PERIODS_NOT_EQUAL              The periods are NOT equal for each pattern in the sequence
 * @retval  PATTERN_SEQUENCE_PATTERN_TYPES_NOT_EQUAL        The pattern types are NOT equal for each pattern in the sequence
 * @retval  PATTERN_DATA_TYPE_INVALID                       The data type of the pattern is invalid
 * @retval  PATTERN_SEQUENCE_TOO_LONG                       The pattern sequence excedes the supported number of patterns
 * @retval  LCR6500_FLASH_IMAGE_INDEX_INVALID               A flash image with the specified index is NOT valid
 * @retval  LCR6500_IMAGE_LIST_TOO_LONG                     Too many images have been created to fit in LightCrafter 6500 flash
 * @retval  LCR6500_PATTERN_SEQUENCE_BUFFERSWAP_TIME_ERROR  Buffer swap has occured prematurely
 */
ReturnCode LCr6500::ConvertSequenceToLut(const dlp::Pattern::Sequence &pattern_sequence, std::vector<LCR6500_LUT_Entry> &sequence_LUT){
    ReturnCode ret;
    unsigned int sequence_count = pattern_sequence.GetCount();

    dlp::Pattern::DataType sequence_type = dlp::Pattern::DataType::INVALID;

    dlp::Pattern      temp_pattern;

    // Check that the sequence has entries
    if(sequence_count == 0)
        return ret.AddError(PATTERN_SEQUENCE_EMPTY);

    // Check that all pattern types are equal
    if(!pattern_sequence.EqualDataTypes())
        return ret.AddError(PATTERN_SEQUENCE_PATTERN_TYPES_NOT_EQUAL);

    // Clear the supplied LUT
    sequence_LUT.clear();

    // Get the sequence type
    ret = pattern_sequence.Get(0,&temp_pattern);
    sequence_type = temp_pattern.data_type;

    // Create the new pattern sequence that the LCr6500 can use
    dlp::Pattern::Sequence   sequence_to_project;

    switch(sequence_type){
    case dlp::Pattern::DataType::INVALID:
    case dlp::Pattern::DataType::IMAGE_DATA:
    case dlp::Pattern::DataType::IMAGE_FILE:
        return ret.AddError(PATTERN_DATA_TYPE_INVALID);
    case dlp::Pattern::DataType::PARAMETERS:
        sequence_to_project.Add(pattern_sequence);
        break;
    }

//    // Stop the display
//    if(DLPC900_PatternDisplay(Pattern::PatternStartStop::STOP) < 0)
//        return ret.AddError(LCR6500_PATTERN_DISPLAY_FAILED);

    // Check pattern sequence timing and create LUT
    for(unsigned int iPat = 0; iPat < sequence_count; iPat++){

        // Check that maximum sequence has NOT been exceeded
        if( sequence_LUT.size() > LCr6500::PATTERN_LUT_SIZE )
            return ret.AddError(PATTERN_SEQUENCE_TOO_LONG);

        // Get the pattern
        temp_pattern.parameters.Clear();
        ret = sequence_to_project.Get(iPat,&temp_pattern);
        if( ret.hasErrors()) return ret;

        // Check the pattern timing
        ret = PatternSettingsValid(temp_pattern);
        if( ret.hasErrors()) return ret;


        // Increment sequence pattern count
        if(temp_pattern.color != dlp::Pattern::Color::RGB){
            // Setup the LUT entry
            LCR6500_LUT_Entry pattern_entry;

            // Copy the pattern timings
            pattern_entry.exposure_us  = temp_pattern.exposure;
            pattern_entry.dark_time_us = temp_pattern.period - temp_pattern.exposure;

            // If the setup routine saw that the trigger should be use set this to true!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            pattern_entry.wait_for_trigger = this->pattern_wait_for_trigger_.Get();

            // Determine bit depth
            pattern_entry.bitdepth = DlpPatternBitdepthToLCr6500Bitdepth(temp_pattern.bitdepth);    /////////////////////////////////////////// Does this need 1 subtracted?

            // Determine LED selects
            pattern_entry.led_select = DlpPatternColorToLCr6500Led(temp_pattern.color);

            // Set the clear DMD after exposure
            pattern_entry.clear_after_exposure = true;

            // Enable trigger out 2 for the pattern
            pattern_entry.enable_trigger_2 = true;

            // Determine pattern number
            Parameters::PatternBitplane pattern_number;
            temp_pattern.parameters.Get(&pattern_number);
            pattern_entry.pattern_bit_position = pattern_number.Get();

            // Determine pattern image
            Parameters::PatternImageIndex flash_image;
            temp_pattern.parameters.Get(&flash_image);
            pattern_entry.pattern_image_index = flash_image.Get();

            // Set the pattern index
            pattern_entry.pattern_lut_index = sequence_LUT.size();

            // Non RGB patterns only use one entry slot in LUT
            sequence_LUT.push_back(pattern_entry);
        }
        else{
            // Setup the LUT entry
            LCR6500_LUT_Entry pattern_entry_red;
            LCR6500_LUT_Entry pattern_entry_green;
            LCR6500_LUT_Entry pattern_entry_blue;

            // Copy the pattern timings

            // For the red channel use a third of the pattern exposure
            // time and set the dark time equal to its channel exposure time
            // so that the green channel displays immediately afterwards
            pattern_entry_red.exposure_us   = temp_pattern.exposure / 3;
            pattern_entry_red.dark_time_us  = 0;

            // For the green channel use a third of the pattern exposure
            // time and set the dark time equal to its channel exposure time
            // so that the blue channel displays immediately afterwards
            pattern_entry_green.exposure_us  = temp_pattern.exposure / 3;
            pattern_entry_green.dark_time_us = 0;

            // For the blue channel use a third of the pattern exposure
            // time and set the blue channels period equal such that the
            // total RGB pattern dark time will terminate at the same moment
            pattern_entry_blue.exposure_us  = temp_pattern.exposure / 3;
            pattern_entry_blue.dark_time_us = temp_pattern.period - temp_pattern.exposure;

//            // Red pattern determines the overall RGB trigger type
//            Parameters::TriggerSource pattern_trigger_red;
//            temp_pattern.parameters.Get(&pattern_trigger_red);
//            pattern_entry_red.trigger_type = pattern_trigger_red.Get();
            // If the setup routine saw that the trigger should be use set this to true!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            pattern_entry_red.wait_for_trigger = this->pattern_wait_for_trigger_.Get();

            // Green and blue patterns don't have a trigger because
            // they are a continuation of the RGB pattern
            pattern_entry_green.wait_for_trigger = false;
            pattern_entry_blue.wait_for_trigger  = false;


            // Determine bit depth. SDK only enables equal color channel bitdepths for RGB
            pattern_entry_red.bitdepth     = DlpPatternBitdepthToLCr6500Bitdepth(temp_pattern.bitdepth);
            pattern_entry_green.bitdepth   = pattern_entry_red.bitdepth;
            pattern_entry_blue.bitdepth    = pattern_entry_red.bitdepth;

            // Determine LED select
            pattern_entry_red.led_select   = dlp::LCr6500::Pattern::Led::RED;
            pattern_entry_green.led_select = dlp::LCr6500::Pattern::Led::GREEN;
            pattern_entry_blue.led_select  = dlp::LCr6500::Pattern::Led::BLUE;



            // Determine pattern number
            Parameters::PatternBitplaneRed   pattern_bitplane_red;
            Parameters::PatternBitplaneGreen pattern_bitplane_green;
            Parameters::PatternBitplaneBlue  pattern_bitplane_blue;

            temp_pattern.parameters.Get(&pattern_bitplane_red);
            pattern_entry_red.pattern_bit_position = pattern_bitplane_red.Get();

            temp_pattern.parameters.Get(&pattern_bitplane_green);
            pattern_entry_green.pattern_bit_position = pattern_bitplane_green.Get();

            temp_pattern.parameters.Get(&pattern_bitplane_blue);
            pattern_entry_blue.pattern_bit_position = pattern_bitplane_blue.Get();

            // Determine buffer swap by if the image changed or not
            Parameters::PatternImageIndexRed   flash_image_red;
            Parameters::PatternImageIndexGreen flash_image_green;
            Parameters::PatternImageIndexBlue  flash_image_blue;

            temp_pattern.parameters.Get(&flash_image_red);
            temp_pattern.parameters.Get(&flash_image_green);
            temp_pattern.parameters.Get(&flash_image_blue);

            pattern_entry_red.pattern_image_index   = flash_image_red.Get();
            pattern_entry_green.pattern_image_index = flash_image_green.Get();
            pattern_entry_blue.pattern_image_index  = flash_image_blue.Get();


            // Setup trigger out 2 to show the beginning of the RGB pattern
            pattern_entry_red.enable_trigger_2    = true;
            pattern_entry_green.enable_trigger_2  = false;
            pattern_entry_blue.enable_trigger_2   = false;

            // Set the clear DMD after exposure
            pattern_entry_red.clear_after_exposure   = true;
            pattern_entry_green.clear_after_exposure = true;
            pattern_entry_blue.clear_after_exposure  = true;

            // Set the pattern index
            pattern_entry_red.pattern_lut_index   = sequence_LUT.size();
            pattern_entry_green.pattern_lut_index = sequence_LUT.size() + 1;
            pattern_entry_blue.pattern_lut_index  = sequence_LUT.size() + 2;

            // Add each single color pattern to create the RGB pattern
            sequence_LUT.push_back(pattern_entry_red);
            sequence_LUT.push_back(pattern_entry_green);
            sequence_LUT.push_back(pattern_entry_blue);
        }
    }

    return ret;
}



/** @brief  Creates a sequence of patterns and firmware images for the LightCrafter 6500,
 *          then uploads the new firmware
 * @param[in]   arg_pattern_sequence    object of \ref dlp::Pattern::Sequence type containing sequence
 *
 * @retval  LCR6500_FIRMWARE_UPLOAD_IN_PROGRESS         A firmware upload is in progress, do NOT send any commands until upload is complete!
 * @retval  LCR6500_NOT_CONNECTED                       The LightCrafter 6500 EVM is NOT connected
 * @retval  PATTERN_SEQUENCE_EMPTY                      The supplied sequence contains no patterns
 * @retval  PATTERN_SEQUENCE_EXPOSURES_NOT_EQUAL        The exposure times are NOT equal for each pattern in the sequence
 * @retval  PATTERN_SEQUENCE_PERIODS_NOT_EQUAL          The periods are NOT equal for each pattern in the sequence
 * @retval  PATTERN_SEQUENCE_PATTERN_TYPES_NOT_EQUAL    The pattern types are NOT equal for each pattern in the sequence
 */
ReturnCode LCr6500::PreparePatternSequence(const dlp::Pattern::Sequence &pattern_sequence){
    ReturnCode   ret;
    unsigned int sequnce_count = pattern_sequence.GetCount();

    dlp::Pattern::DataType sequence_type = dlp::Pattern::DataType::INVALID;

    // Check that the sequence has entries
    if(sequnce_count == 0)
        return ret.AddError(PATTERN_SEQUENCE_EMPTY);

    // Check that all pattern types are equal
    if(!pattern_sequence.EqualDataTypes())
        return ret.AddError(PATTERN_SEQUENCE_PATTERN_TYPES_NOT_EQUAL);

    // Get the sequence type from the first pattern
    dlp::Pattern temp_pattern;
    ret = pattern_sequence.Get(0,&temp_pattern);
    sequence_type = temp_pattern.data_type;

    // Create the new pattern sequence that the LCr6500 can use
    std::vector<std::string> dlpc900_image_list;

    switch(sequence_type){
    case dlp::Pattern::DataType::INVALID:
        return ret.AddError(PATTERN_DATA_TYPE_INVALID);
    case dlp::Pattern::DataType::IMAGE_DATA:
    case dlp::Pattern::DataType::IMAGE_FILE:
    {
        // Need to make a new pattern sequence of parameters type

        // Create new firmware images and sequence
        std::string composite_image_file_base = "dlpc900_composite_image_";
        ret = this->CreateFirmwareImages( pattern_sequence,
                                          composite_image_file_base,
                                          this->pattern_sequence_,
                                          dlpc900_image_list);
        if(ret.hasErrors())
            return ret;

        // Clear the compressed images vector so it can be refilled
        for(unsigned int iImage = 0; iImage < this->compressed_images_.size(); iImage++){
            free(this->compressed_images_.at(iImage).bitmapImage1);
            //free(this->compressed_images_.at(iImage).bitmapImage2); // Uncomment for DLP9000
        }
        this->compressed_images_.clear();

        // Compress the images
        for(unsigned int iImageIndex = 0; iImageIndex < dlpc900_image_list.size(); iImageIndex++){

            this->debug_.Msg("Compressing image = " + dlp::Number::ToString(iImageIndex));

            // Compress the image for the DLPC900 using the DLP6500 DMD
            COMPRESSED_BITMAPIMAGES compressed_dlpc900_images;
            ret = this->CompressImageFile(dlpc900_image_list.at(iImageIndex), &compressed_dlpc900_images );
            if(ret.hasErrors()) return ret;

            // Add the compressed image to the vector
            this->compressed_images_.push_back(compressed_dlpc900_images);
        }

        // Set flag that firmware has been uploaded
        this->sequence_prepared_.Set(true);

        break;
    }
    case dlp::Pattern::DataType::PARAMETERS:
        this->pattern_sequence_.Clear();
        this->pattern_sequence_.Add(pattern_sequence);
        break;
    }

    this->pattern_sequence_prepared_ = true;

    return ret;
}



ReturnCode LCr6500::CompressImageFile(const std::string image_file, COMPRESSED_BITMAPIMAGES* compressed_bitmaps ){
    ReturnCode ret;

    this->debug_.Msg("Checking that image exists: " + image_file);

    // Check the image exists
    if(!dlp::File::Exists(image_file))
        return ret.AddError(FILE_DOES_NOT_EXIST);

    // Check the image resolution
    this->debug_.Msg("Checking image resolution...");
    if(!this->ImageResolutionCorrect(image_file))
        return ret.AddError(LCR6500_IMAGE_RESOLUTION_INVALID);

    // Get the size of the image
    long long image_file_size = dlp::File::GetSize(image_file);

    // Allocate memory to read the image into memory
    unsigned char *image_file_byte_array;
    image_file_byte_array = new (std::nothrow) unsigned char [image_file_size];

    // Check that the memory allocation succeeded
    if (image_file_byte_array == nullptr)
        return ret.AddError(LCR6500_IMAGE_MEMORY_ALLOCATION_FAILED);

    // Read the image into memory
    this->debug_.Msg("Reading image into memory...");
    std::ifstream image_file_stream(image_file, std::ifstream::binary);
    image_file_stream.read((char *)image_file_byte_array, image_file_size);
    image_file_stream.close();

    // Compress the image for the DLPC900 using the DLP6500 DMD
    // Settings to create the compressed image
    bool    dual_dlpc900_for_dlp9000 = false;
    int     compression_format = 0xF;

    this->debug_.Msg("Compressing image...");
    std::cout<< "Compress image " << Splash_createImage( image_file_byte_array,
                        compressed_bitmaps,
                        &compression_format,
                        dual_dlpc900_for_dlp9000) <<std::endl;


    this->debug_.Msg("Compressed original image from " + dlp::Number::ToString(image_file_size) +
                     " bytes to " + dlp::Number::ToString(compressed_bitmaps->sizeBitmap1) + " bytes");

    // Free the allocated memory
    this->debug_.Msg("Releasing memory...");
    delete[] image_file_byte_array;

    return ret;
}



/** @brief      Displays a previously prepared pattern sequence
 *  @param[in]  repeat  If true, the sequence repeats after completing
 *  @warning    Must call \ref LCr6500::PreparePatternSequence() before using this method
 *  @retval     LCR6500_PATTERN_SEQUENCE_NOT_PREPARED   The pattern sequence has NOT been prepared and sent to the LightCrafter 6500
 *  @retval     LCR6500_IN_CALIBRATION_MODE             The LightCrafter 6500 is in calibration mode and the sequence cannot be started
 */
ReturnCode LCr6500::StartPatternSequence(const unsigned int &start, const unsigned int &patterns, const bool &repeat){
    ReturnCode ret;

    // Check that LCr6500 is connected
    if(!this->isConnected()){
        // Device NOT connected
        return ret.AddError(LCR6500_NOT_CONNECTED);
    }

    // Check that sequence has been prepared
    this->debug_.Msg("Check that sequence has been prepared...");
    if(!this->pattern_sequence_prepared_ )
        return ret.AddError(LCR6500_PATTERN_SEQUENCE_NOT_PREPARED);

    // Check  that the indices are NOT out of range
    this->debug_.Msg("Check that requested patterns are within range...");
    if((start + patterns) > this->pattern_sequence_.GetCount())
        return ret.AddError(PATTERN_SEQUENCE_INDEX_OUT_OF_RANGE);

    // Create the sequence
    this->debug_.Msg("Grab the requested patterns...");
    dlp::Pattern::Sequence sequence;
    for(unsigned int iPat = start; iPat < start+patterns; iPat++){
        dlp::Pattern temp;

        // Get the pattern
        this->pattern_sequence_.Get(iPat,&temp);

        // Add it to new sequence
        sequence.Add(temp);
    }

    // Convert the dlp::Pattern::Sequence to a LCr6500 LUT
    this->debug_.Msg("Create DLPC900 pattern sequence LUT...");
    std::vector<LCR6500_LUT_Entry> dlpc900_lut;
    ret = this->ConvertSequenceToLut(sequence,dlpc900_lut);
    if( ret.hasErrors()) return ret;

    // Set the LUT and the necessary images
    this->debug_.Msg("Send DLPC900 pattern sequence LUT to device and upload images...");
    ret = this->SendLutWithImages(dlpc900_lut, repeat);
    if( ret.hasErrors()) return ret;

    // Start the sequence
    if(DLPC900_PatternDisplay(Pattern::PatternStartStop::START) < 0)
        return ret.AddError(LCR6500_PATTERN_DISPLAY_FAILED);

    return ret;
}

/** @brief      Displays a specific pattern in a previously prepared sequence
 *  @warning    Must call \ref LCr6500::PreparePatternSequence() before using this method
 * @param[in]   pattern_index   index for the pattern to be displayed
 * @param[in]   repeat  bool value, if true, repeat the display of the pattern
 *
 * @retval  LCR6500_PATTERN_SEQUENCE_NOT_PREPARED   The pattern sequence has NOT been prepared and sent to the LightCrafter 6500
 * @retval  PATTERN_SEQUENCE_INDEX_OUT_OF_RANGE     The pattern index number is NOT valid
 */
ReturnCode LCr6500::DisplayPatternInSequence(const unsigned int &pattern_index, const bool &repeat){
    ReturnCode ret;

    // Check that LCr6500 is connected
    if(!this->isConnected()){
        // Device NOT connected
        return ret.AddError(LCR6500_NOT_CONNECTED);
    }

    // Check that sequence has been prepared
    if(!this->pattern_sequence_prepared_)
        return ret.AddError(LCR6500_PATTERN_SEQUENCE_NOT_PREPARED);

    // Check that pattern is in range
    if(pattern_index >= this->pattern_sequence_.GetCount())
        return ret.AddError(PATTERN_SEQUENCE_INDEX_OUT_OF_RANGE);

    // Load only next pattern
    dlp::Pattern next_pattern;

    // Clear the parameters
    next_pattern.parameters.Clear();
    ret = this->pattern_sequence_.Get( pattern_index, &next_pattern);

    // Check for error
    if(ret.hasErrors()) return ret;

    // Load the first pattern as a sequence
    dlp::Pattern::Sequence single_pattern_sequence;
    single_pattern_sequence.Clear();
    single_pattern_sequence.Add(next_pattern);

    // Convert the dlp::Pattern::Sequence to a LCr6500 LUT
    std::vector<LCR6500_LUT_Entry> dlpc900_lut;
    ret = this->ConvertSequenceToLut(single_pattern_sequence,dlpc900_lut);
    if( ret.hasErrors()) return ret;


    //DLPC900_SetTrigIn1Config();
    //DLPC900_SetTrigIn1Delay();





    // Set the LUT and the necessary images
    ret = this->SendLutWithImages(dlpc900_lut, repeat);
    if( ret.hasErrors()) return ret;

    // Start the sequence
    if(DLPC900_PatternDisplay(Pattern::PatternStartStop::START) < 0)
        return ret.AddError(LCR6500_PATTERN_DISPLAY_FAILED);

    return ret;
}

/** @brief  Stops the current display of a pattern sequence
 *  @retval  LCR6500_NOT_CONNECTED   The LightCrafter 6500 EVM is NOT connected
 */
ReturnCode LCr6500::StopPatternSequence(){
    ReturnCode ret;

    // Check that LCr6500 is connected
    if(!this->isConnected()){
        // Device NOT connected
        return ret.AddError(LCR6500_NOT_CONNECTED);
    }

    // Stop the display
    if(DLPC900_PatternDisplay(Pattern::PatternStartStop::STOP) < 0)
        return ret.AddError(LCR6500_PATTERN_DISPLAY_FAILED);

    return ret;
}

ReturnCode LCr6500::SendLutWithImages(const std::vector<LCR6500_LUT_Entry> &dlpc900_lut, const bool &repeat ){
    ReturnCode ret;

    // Check that LCr6500 is connected
    if(!this->isConnected()){
        // Device NOT connected
        return ret.AddError(LCR6500_NOT_CONNECTED);
    }

    // Check number of patterns is NOT greater than the LUT size
    this->debug_.Msg("Check that the requested sequence will fit in the DLPC900 LUT...");
    if( dlpc900_lut.size() > LCr6500::PATTERN_LUT_SIZE )
        return ret.AddError(PATTERN_SEQUENCE_TOO_LONG);


    // Determing the first and last images used in the sequence
    this->debug_.Msg("Determine first and last images used...");
    unsigned int  dlpc900_lut_count  = dlpc900_lut.size();

    if(dlpc900_lut_count == 0)
        return ret.AddError(LCR6500_PATTERN_SEQUENCE_NOT_PREPARED);

    unsigned char image_index_first  = dlpc900_lut.at(0).pattern_image_index;
    unsigned char image_index_last   = dlpc900_lut.at(dlpc900_lut_count-1).pattern_image_index;


    // Change device to pattern sequence mode
    this->debug_.Msg("Setting device pattern sequence on-the-fly mode...");
    if(DLPC900_SetMode(OperatingMode::PATTERN_MODE_ON_THE_FLY) < 0)
        return ret.AddError(LCR6500_SET_OPERATING_MODE_FAILED);

    // Stop the sequence if something is already running
    this->debug_.Msg("Stop the pattern sequence...");
    if(DLPC900_PatternDisplay(Pattern::PatternStartStop::STOP) < 0)
          return ret.AddError(LCR6500_PATTERN_DISPLAY_FAILED);

    /////////////////////////////////////////////////////////////////////////////  SET TRIGGERS
//    if(DLPC900_SetTrigOutConfig( LCR6500_TRIGGER_OUT_1,
//                                 this->trigger_out_1_invert_.Get(),
//                                 this->trigger_out_1_rising_.Get(),
//                                 this->trigger_out_1_falling_.Get())< 0)
//        return ret.AddError(LCR6500_SET_TRIGGER_OUTPUT_CONFIG_FAILED);



//    if(DLPC900_SetTrigOutConfig( LCR6500_TRIGGER_OUT_2,
//                                 this->trigger_out_2_invert_.Get(),
//                                 this->trigger_out_2_rising_.Get(),
//                                 0)< 0)
//        return ret.AddError(LCR6500_SET_TRIGGER_OUTPUT_CONFIG_FAILED);

    // Send the add the LUT entries
    DLPC900_ClearPatLut();
    for(unsigned int iEntry = 0; iEntry < dlpc900_lut.size(); iEntry++ ){

        if(DLPC900_AddToPatLut(iEntry,
                               dlpc900_lut.at(iEntry).exposure_us,
                               dlpc900_lut.at(iEntry).clear_after_exposure,
                               dlpc900_lut.at(iEntry).bitdepth,
                               dlpc900_lut.at(iEntry).led_select,
                               dlpc900_lut.at(iEntry).wait_for_trigger,
                               dlpc900_lut.at(iEntry).dark_time_us,
                               dlpc900_lut.at(iEntry).enable_trigger_2,
                               dlpc900_lut.at(iEntry).pattern_image_index - image_index_first, // Adjust the image index
                               dlpc900_lut.at(iEntry).pattern_bit_position)<0)
            return ret.AddError(LCR6500_ADD_EXP_LUT_ENTRY_FAILED);
    }

    if(DLPC900_SendPatLut() < 0)
        return ret.AddError(LCR6500_SEND_EXP_PATTERN_LUT_FAILED);  ///////////////////////////////////////////////////////////////////////

    // Set the pattern configuration
    if(repeat){
        // Repeat the sequence
         if(DLPC900_SetPatternConfig(dlpc900_lut.size(), 0) < 0)
            return ret.AddError(LCR6500_SET_PATTERN_DISPLAY_MODE_FAILED); ///////////////////////////////////////////
    }
    else{
        // Do not repeat the sequence
        if(DLPC900_SetPatternConfig(dlpc900_lut.size(), dlpc900_lut.size()) < 0)
            return ret.AddError(LCR6500_SET_PATTERN_DISPLAY_MODE_FAILED); ///////////////////////////////////////////
    }

    // Check if the last set of images matches what was previously sent
    if( (image_index_first != this->image_previous_first_ ) ||
        (image_index_last  != this->image_previous_last_  )    ){

        // Upload the images needed for this sequence from the precomprossed image vector
        ret = this->UploadPrestoredCompressedImages(image_index_first,image_index_last);
        if(ret.hasErrors()) return ret;

        //this->image_previous_first_ = image_index_first;
        //this->image_previous_last_  = image_index_last;
    }

    return ret;
}

/** @brief      Determines which LightCrafter 6500 LED configuration to use
 *              based on the supplied pattern color setting
 * @param[in]   color  color setting in a \ref dlp::Pattern object
 *
 * @retval  LCR6500_COMMAND_FAILED  Could NOT find a valid LED for the color
 */
int LCr6500::DlpPatternColorToLCr6500Led(const dlp::Pattern::Color &color){
    switch(color){
    case dlp::Pattern::Color::RED:        return dlp::LCr6500::Pattern::Led::RED;
    case dlp::Pattern::Color::GREEN:      return dlp::LCr6500::Pattern::Led::GREEN;
    case dlp::Pattern::Color::BLUE:       return dlp::LCr6500::Pattern::Led::BLUE;
    case dlp::Pattern::Color::CYAN:       return dlp::LCr6500::Pattern::Led::CYAN;
    case dlp::Pattern::Color::YELLOW:     return dlp::LCr6500::Pattern::Led::YELLOW;
    case dlp::Pattern::Color::MAGENTA:    return dlp::LCr6500::Pattern::Led::MAGENTA;
    case dlp::Pattern::Color::WHITE:      return dlp::LCr6500::Pattern::Led::WHITE;
    case dlp::Pattern::Color::NONE:       return dlp::LCr6500::Pattern::Led::NONE;
    case dlp::Pattern::Color::BLACK:      return dlp::LCr6500::Pattern::Led::NONE;
    case dlp::Pattern::Color::RGB:        return dlp::LCr6500::Pattern::Led::NONE;
    case dlp::Pattern::Color::INVALID:    return dlp::LCr6500::Pattern::Led::NONE;
    }

    return LCR6500_COMMAND_FAILED;
}

/** @brief      Determines which LightCrafter 6500 bit depth to use based on
 *              the supplied pattern bit depth setting
 * @param[in]   depth  bit depth setting in a \ref dlp::Pattern object
 *
 * @retval  LCR6500_COMMAND_FAILED  Could NOT find a valid bit depth for the pattern
 */
int LCr6500::DlpPatternBitdepthToLCr6500Bitdepth(const dlp::Pattern::Bitdepth &depth){
    switch(depth){
    case dlp::Pattern::Bitdepth::INVALID:     return 0;
    case dlp::Pattern::Bitdepth::MONO_1BPP:   return dlp::LCr6500::Pattern::Bitdepth::MONO_1BPP;
    case dlp::Pattern::Bitdepth::MONO_2BPP:   return dlp::LCr6500::Pattern::Bitdepth::MONO_2BPP;
    case dlp::Pattern::Bitdepth::MONO_3BPP:   return dlp::LCr6500::Pattern::Bitdepth::MONO_3BPP;
    case dlp::Pattern::Bitdepth::MONO_4BPP:   return dlp::LCr6500::Pattern::Bitdepth::MONO_4BPP;
    case dlp::Pattern::Bitdepth::MONO_5BPP:   return dlp::LCr6500::Pattern::Bitdepth::MONO_5BPP;
    case dlp::Pattern::Bitdepth::MONO_6BPP:   return dlp::LCr6500::Pattern::Bitdepth::MONO_6BPP;
    case dlp::Pattern::Bitdepth::MONO_7BPP:   return dlp::LCr6500::Pattern::Bitdepth::MONO_7BPP;
    case dlp::Pattern::Bitdepth::MONO_8BPP:   return dlp::LCr6500::Pattern::Bitdepth::MONO_8BPP;
    case dlp::Pattern::Bitdepth::RGB_3BPP:    return dlp::LCr6500::Pattern::Bitdepth::MONO_1BPP;
    case dlp::Pattern::Bitdepth::RGB_6BPP:    return dlp::LCr6500::Pattern::Bitdepth::MONO_2BPP;
    case dlp::Pattern::Bitdepth::RGB_9BPP:    return dlp::LCr6500::Pattern::Bitdepth::MONO_3BPP;
    case dlp::Pattern::Bitdepth::RGB_12BPP:   return dlp::LCr6500::Pattern::Bitdepth::MONO_4BPP;
    case dlp::Pattern::Bitdepth::RGB_15BPP:   return dlp::LCr6500::Pattern::Bitdepth::MONO_5BPP;
    case dlp::Pattern::Bitdepth::RGB_18BPP:   return dlp::LCr6500::Pattern::Bitdepth::MONO_6BPP;
    case dlp::Pattern::Bitdepth::RGB_21BPP:   return dlp::LCr6500::Pattern::Bitdepth::MONO_7BPP;
    case dlp::Pattern::Bitdepth::RGB_24BPP:   return dlp::LCr6500::Pattern::Bitdepth::MONO_8BPP;
    }
    return LCR6500_COMMAND_FAILED;
}

/** @brief  Uploads firmware file to LightCrafter 6500 EVM
 * @param[in]   firmware_filename  Input file path
 *
 * @retval  LCR6500_FIRMWARE_UPLOAD_IN_PROGRESS                 A firmware upload is in progress, do NOT send any commands until upload is complete!
 * @retval  LCR6500_FIRMWARE_FILE_NOT_FOUND                     Could NOT find firmware file
 * @retval  LCR6500_FIRMWARE_FLASH_PARAMETERS_FILE_NOT_FOUND    LightCrafter 6500 flash device parameters file does NOT exist
 * @retval  LCR6500_NOT_CONNECTED                               The LightCrafter 6500 EVM is NOT connected
 * @retval  LCR6500_UNABLE_TO_ENTER_PROGRAMMING_MODE            The LightCrafter 6500 was NOT able to enter programming mode
 * @retval  LCR6500_GET_FLASH_MANUFACTURER_ID_FAILED            The flash manufacturer ID could NOT be read
 * @retval  LCR6500_GET_FLASH_DEVICE_ID_FAILED                  The flash device ID could NOT be read
 * @retval  LCR6500_FLASHDEVICE_PARAMETERS_NOT_FOUND            The flash parameters are NOT valid
 * @retval  LCR6500_COMMAND_FAILED                              The LightCrafter 6500 command failed
 * @retval  LCR6500_FIRMWARE_FLASH_ERASE_FAILED                 The firmware on the LightCrafter 6500 could NOT be erased
 * @retval  LCR6500_FIRMWARE_MEMORY_ALLOCATION_FAILED           Memory could NOT be allocated on the LightCrafter 6500
 * @retval  LCR6500_FIRMWARE_UPLOAD_FAILED                      The firmware upload failed
 * @retval  LCR6500_FIRMWARE_CHECKSUM_VERIFICATION_FAILED       The firmware checksum could NOT be verified
 * @retval  LCR6500_FIRMWARE_CHECKSUM_MISMATCH                  The uploaded firmware's checksum does NOT match the firmware on the LightCrafter 6500
 */
ReturnCode LCr6500::UploadFirmware(std::string firmware_filename){
    ReturnCode ret;

//    std::string flash_parameters_filename = this->DLPC900_flash_parameters_.Get();

//    // If A firmware upload is in progress, do NOT send any commands until upload is complete! already return error
//    if(this->FirmwareUploadInProgress()){
//        this->debug_.Msg("Cannot upload firmware because upload already in progress");
//        return ret.AddError(LCR6500_FIRMWARE_UPLOAD_IN_PROGRESS);
//    }
//    else{
//        // Set the firmware uploading flag since upload has NOT started yet
//        while(this->firmware_upload_in_progress.test_and_set()){};
//    }


//    this->pattern_sequence_prepared_     = false;

//    this->firmware_upload_restart_needed = false;

//    // Check that the firmware file exists
//    this->debug_.Msg("Checking that firmware file " + firmware_filename +" exists...");
//    if(!dlp::File::Exists(firmware_filename)){
//        this->debug_.Msg("Firmware file " + firmware_filename + " NOT found");
//        this->firmware_upload_in_progress.clear();
//        this->firmware_upload_restart_needed    = false;
//        return ret.AddError(LCR6500_FIRMWARE_FILE_NOT_FOUND);
//    }
//    this->firmwarePath = firmware_filename;




//    // Check that the flash parameters file exists
//    this->debug_.Msg("Checking that flash parameters file "+ flash_parameters_filename +" exists...");
//    if(!dlp::File::Exists(flash_parameters_filename)){
//        this->debug_.Msg("Flash parameters file " + flash_parameters_filename + " NOT found");
//        this->firmware_upload_in_progress.clear();
//        this->firmware_upload_restart_needed    = false;
//        return ret.AddError(LCR6500_FIRMWARE_FLASH_PARAMETERS_FILE_NOT_FOUND);
//    }

//    // Note the firmware upload process has begun
//    this->firmware_upload_percent_erased_   = 0;
//    this->firmware_upload_percent_complete_ = 0;

//    unsigned short      manID = 0;
//    unsigned long long  devID = 0;

//    int startSector = 0;
//    int i;
//    int BLsize = 0;
//    int lastSectorToErase;

//    unsigned char *pByteArray=NULL;
//    long long dataLen = 0;
//    long long dataLen_full = 0;

//    int bytesSent;

//    unsigned int expectedChecksum = 0;
//    unsigned int checksum         = 0;

//    bool skip_bootloader = true;

//       // Check that LCr6500 is connected
//    this->debug_.Msg("Checking that device is connected...");
//    if(!this->isConnected()){
//        // isConnected() contains debug messages
//        this->firmware_upload_in_progress.clear();
//        this->firmware_upload_restart_needed    = false;
//        return ret.AddError(LCR6500_NOT_CONNECTED);
//    }


//    // Enter programming mode
//    this->debug_.Msg("Putting device in programming mode...");
//    if(DLPC900_EnterProgrammingMode() < 0){
//        this->debug_.Msg("Device did NOT enter programming mode");
//        this->firmware_upload_in_progress.clear();
//        this->firmware_upload_restart_needed    = false;
//        return ret.AddError(LCR6500_UNABLE_TO_ENTER_PROGRAMMING_MODE);
//    }

//    // Wait for 5 seconds
//    this->firmware_upload_restart_needed = true;
//    this->debug_.Msg("Waiting for 5 seconds...");
//    dlp::Time::Sleep::Milliseconds(5000);

//    // Disconnect the projector
//    this->debug_.Msg("Disconnecting device...");
//    this->Disconnect();
//    this->debug_.Msg("Device disconnected");
//    this->debug_.Msg("Waiting for 5 seconds...");
//    dlp::Time::Sleep::Milliseconds(5000);

//    // Reconnect the projector
//    this->debug_.Msg("Connecting to device...");
//    unsigned int device;
//    this->GetDevice(&device);
//    this->Connect(device);
//    if(!this->isConnected()){
//        // isConnected() contains debug messages
//        this->firmware_upload_in_progress.clear();
//        this->firmware_upload_restart_needed    = false;
//        return ret.AddError(LCR6500_NOT_CONNECTED);
//    }
//    this->debug_.Msg("Connected");
//    this->debug_.Msg("Waiting for 5 seconds...");
//    dlp::Time::Sleep::Milliseconds(5000);
//    this->firmware_upload_restart_needed = false;

//    // Enter programming mode
//    this->debug_.Msg("Putting device in programming mode...");
//    if(DLPC900_EnterProgrammingMode() < 0){
//        this->debug_.Msg("Device did NOT enter programming mode");
//        this->firmware_upload_in_progress.clear();
//        this->firmware_upload_restart_needed    = false;
//        return ret.AddError(LCR6500_UNABLE_TO_ENTER_PROGRAMMING_MODE);
//    }

//    // Get the connected LCr6500 flashdevice information
//    this->debug_.Msg("Retrieving flash manufacturer and device IDs from connected device...");
//    if((DLPC900_GetFlashManID(&manID) < 0)){
//        this->debug_.Msg("Retrieving flash manufacturer ID FAILED");
//        this->firmware_upload_in_progress.clear();
//        this->firmware_upload_restart_needed    = false;
//        return ret.AddError(LCR6500_GET_FLASH_MANUFACTURER_ID_FAILED);
//    }

//    if((DLPC900_GetFlashDevID(&devID) < 0)){
//        this->debug_.Msg("Retrieving flash device ID FAILED");
//        this->firmware_upload_in_progress.clear();
//        this->firmware_upload_restart_needed    = false;
//        return ret.AddError(LCR6500_GET_FLASH_DEVICE_ID_FAILED);
//    }

//    // Save flash device information to this LCr6500 object
//    devID &= 0xFFFF;
//    this->myFlashDevice.Mfg_ID = manID;
//    this->myFlashDevice.Dev_ID = devID;

//    std::ifstream   flash_param_file(flash_parameters_filename);
//    std::string     flash_param_file_line;

//    // Read in file line by line
//    this->debug_.Msg("Searching for flash device parameters in " + flash_parameters_filename + "...");
//    bool flashdevice_found = false;
//    while(std::getline(flash_param_file,flash_param_file_line) &&
//          flashdevice_found == false)
//    {
//        this->debug_.Msg("...");
//        flashdevice_found = this->ProcessFlashParamsLine(flash_param_file_line);
//    }
//    flash_param_file.close();

//    // If the flash device was NOT found in the file return error
//    if(flashdevice_found == false){
//        this->debug_.Msg("Flash device parameters NOT found");
//        this->firmware_upload_in_progress.clear();
//        this->firmware_upload_restart_needed    = false;
//        return ret.AddError(LCR6500_FLASHDEVICE_PARAMETERS_NOT_FOUND);
//    }

//    this->debug_.Msg("Flash device parameters found");
//    this->debug_.Msg("Waiting 1 second...");
//    dlp::Time::Sleep::Milliseconds(1000);

//    if(skip_bootloader)
//    {
//        BLsize = 128 * 1024;
//        dataLen -= BLsize;
//    }

//    // Get the flash device memory address sectors needed for programming
//    this->debug_.Msg("Determining amount of memory needed for firmware file...");
//    startSector = GetSectorNum(BLsize);
//    lastSectorToErase = GetSectorNum(dlp::File::GetSize(firmware_filename));

//    //If perfectly aligned with last sector start addr, no need to erase last sector.
//    if(dlp::File::GetSize(firmware_filename) == this->myFlashDevice.SectorArr[lastSectorToErase]){
//        lastSectorToErase -= 1;
//    }

//    // Set the flashdevice type on connected LCr6500
//    DLPC900_SetFlashType(this->myFlashDevice.Type);

//    // Erase the flash sectors on connected LCr6500
//    this->debug_.Msg("Erasing flash sectors " + Number::ToString(startSector) + " to " + Number::ToString(lastSectorToErase) + "...");
//    for(int iSector=startSector; iSector <= lastSectorToErase; iSector++)
//    {
//        // Set the flash sector to be erased and erase it
//        DLPC900_SetFlashAddr(this->myFlashDevice.SectorArr[iSector]);
//        if(DLPC900_FlashSectorErase() < 0){
//            this->debug_.Msg("Flash sector " + Number::ToString(iSector) + " FAILED to erase");
//            this->firmware_upload_in_progress.clear();
//            this->firmware_upload_restart_needed    = false;
//            return ret.AddError(LCR6500_FIRMWARE_FLASH_ERASE_FAILED);
//        }

//        // Wait for the erase command to complete
//        DLPC900_WaitForFlashReady();
//        this->firmware_upload_percent_erased_ = iSector*100/lastSectorToErase;
//        this->debug_.Msg("Flash erase " + Number::ToString(this->GetFirmwareFlashEraseComplete()) + "% complete");
//    }
//    this->firmware_upload_percent_erased_ = 100;
//    this->debug_.Msg("Erasing flash sectors complete");

//    // Allocate memory to load the firmware image
//    this->debug_.Msg("Allocating memory for firmware image...");
//    dataLen = dlp::File::GetSize(firmware_filename);
//    pByteArray = new (std::nothrow) unsigned char [dataLen];
//    if (pByteArray == nullptr){
//        this->debug_.Msg("Allocating memory for firmware image FAILED");
//        this->firmware_upload_in_progress.clear();
//        this->firmware_upload_restart_needed    = false;
//        return ret.AddError(LCR6500_FIRMWARE_MEMORY_ALLOCATION_FAILED);
//    }

//    // Read the firmware iamge into memory
//    this->debug_.Msg("Loading firmware image " + firmware_filename + " into memory...");
//    std::ifstream firmware(firmware_filename, std::ifstream::binary);
//    firmware.read((char *)pByteArray, dataLen);
//    firmware.close();

//    // Upload the firmware into the EVM
//    DLPC900_SetFlashAddr(BLsize);
//    dataLen -= BLsize;
//    DLPC900_SetUploadSize(dataLen);
//    dataLen_full = dataLen;
//    this->debug_.Msg("Starting to upload firmware to device...");
//    while(dataLen > 0)
//    {
//        bytesSent = DLPC900_UploadData(pByteArray+BLsize+dataLen_full-dataLen, dataLen);

//        if(bytesSent < 0)
//        {
//            delete[] pByteArray;
//            this->debug_.Msg("Firmware upload FAILED");
//            this->firmware_upload_in_progress.clear();
//            this->firmware_upload_restart_needed    = false;
//            return ret.AddError(LCR6500_FIRMWARE_UPLOAD_FAILED);
//        }

//        for(i=0; i<bytesSent; i++)
//        {
//            expectedChecksum += pByteArray[BLsize+dataLen_full-dataLen+i];
//        }

//        dataLen -= bytesSent;

//        if(this->firmware_upload_percent_complete_ != (((dataLen_full-dataLen)*100)/dataLen_full))
//        {
//            this->firmware_upload_percent_complete_ = (((dataLen_full-dataLen)*100)/dataLen_full);
//            this->debug_.Msg("Uploading firmware image " + Number::ToString(this->GetFirmwareUploadPercentComplete()) + "% complete");
//        }
//    }
//    this->debug_.Msg("Verifying checksum...");
//    if(DLPC900_CalculateFlashChecksum() < 0){
//        this->debug_.Msg("Checksum verification FAILED");
//        this->firmware_upload_in_progress.clear();
//        this->firmware_upload_restart_needed    = false;
//        return ret.AddError(LCR6500_FIRMWARE_CHECKSUM_VERIFICATION_FAILED);
//    }
//    this->debug_.Msg("Device calculating checksum...");
//    DLPC900_WaitForFlashReady();
//    if(DLPC900_GetFlashChecksum(&checksum) < 0){
//        this->debug_.Msg("Checksum verification FAILED");
//        this->firmware_upload_in_progress.clear();
//        this->firmware_upload_restart_needed    = false;
//        return ret.AddError(LCR6500_FIRMWARE_CHECKSUM_VERIFICATION_FAILED);
//    }
//    else if(checksum != expectedChecksum)
//    {
//        this->debug_.Msg("Checksum mismatch");
//        this->debug_.Msg("Expected = " + Number::ToString(expectedChecksum));
//        this->debug_.Msg("Received = " + Number::ToString(checksum));
//        this->firmware_upload_in_progress.clear();
//        this->firmware_upload_restart_needed    = false;
//        return ret.AddError(LCR6500_FIRMWARE_CHECKSUM_MISMATCH);
//    }
//    else
//    {
//        this->debug_.Msg("Exiting programming mode...");
//        DLPC900_ExitProgrammingMode(); //Exit programming mode; Start application.
//    }

//    delete[] pByteArray;

//    this->debug_.Msg("Device rebooting...");
//    this->debug_.Msg("Waiting 5 seconds...");
//    this->firmware_upload_restart_needed = true;
//    dlp::Time::Sleep::Milliseconds(5000);

//    // Disconnect the projector
//    this->debug_.Msg("Disconnecting device...");
//    this->Disconnect();
//    this->debug_.Msg("Waiting 5 seconds...");
//    dlp::Time::Sleep::Milliseconds(5000);

//    // Reconnect the projector
//    this->debug_.Msg("Connecting to device...");
//    this->GetDevice(&device);
//    this->Connect(device);
//    if(!this->isConnected()){
//        // isConnected() contains debug messages
//        this->firmware_upload_in_progress.clear();
//        this->firmware_upload_restart_needed    = false;
//        return ret.AddError(LCR6500_NOT_CONNECTED);
//    }
//    this->debug_.Msg("Connected");
//    this->debug_.Msg("Firmware upload complete");
//    this->firmware_upload_in_progress.clear();
//    this->firmware_upload_restart_needed    = false;
    return ret;
}

/** @brief  Returns true if a firmware upload is in progress */
bool LCr6500::FirmwareUploadInProgress(){
//    if(!this->firmware_upload_in_progress.test_and_set()){
//        // Firmware is NOT uploading, clear flag since the above
//        // methods sets it
//        this->firmware_upload_in_progress.clear();
//        return false;
//    }
//    else{
//        // Firmware upload is in progress
        return true;
//    }
}

/** @brief  Returns the firmware upload completion in percent */
long long LCr6500::GetFirmwareUploadPercentComplete(){
    return this->firmware_upload_percent_complete_;
}

/** @brief  Returns the firmware flash erase completion in percent */
long long LCr6500::GetFirmwareFlashEraseComplete(){
    return this->firmware_upload_percent_erased_;
}

/** @brief  Gets flash memory device parameters from flash parameter file.
 * Returns false if parameters are NOT found or flash device has too few sectors
 * @param[in]   line    input line for read in from file
 */
bool LCr6500::ProcessFlashParamsLine(const std::string &line)
{
//    unsigned int MfgID;
//    unsigned int DevID;

//    std::vector<std::string> separated_parameters;

//    std::string param_line = line;

//    // Check that the line is NOT empty
//    if(param_line.empty()) return false;

//    // Remove leading and trailing whitespace
//    param_line = dlp::String::Trim(line);

//    // Check that the line does NOT begin with /
//    if(param_line.front() == '/') return false;

//    // Separate the CSV values
//    separated_parameters = dlp::String::SeparateDelimited(param_line,',');

//    // Check that there are at least 9 flash device parameters
//    // Mfg    MfgID    Device    DevID    Mb    Alg    Size    #sec    Sector_Addresses
//    if(separated_parameters.size() <= 9) return false;

//    // Read in the HEX manufacturer ID and device ID
//    MfgID = dlp::String::HEXtoNumber_unsigned<unsigned int>(separated_parameters.at(1));
//    DevID = dlp::String::HEXtoNumber_unsigned<unsigned int>(separated_parameters.at(3));

//    // Check is these values match the device currently connected
//    if((MfgID == this->myFlashDevice.Mfg_ID) && (DevID == this->myFlashDevice.Dev_ID))
//    {
//        this->myFlashDevice.Mfg = separated_parameters.at(0);
//        this->myFlashDevice.Dev = separated_parameters.at(2);
//        this->myFlashDevice.Size_MBit  = dlp::String::ToNumber<unsigned int>(separated_parameters.at(4));
//        this->myFlashDevice.Type       = dlp::String::ToNumber<unsigned char>(separated_parameters.at(5));
//        this->myFlashDevice.numSectors = dlp::String::ToNumber<unsigned int>(separated_parameters.at(7));

//        // Check that flash device has number of sectors
//        if(separated_parameters.size() < (8 + this->myFlashDevice.numSectors)) return false;

//        for(unsigned int iSector = 0; iSector < this->myFlashDevice.numSectors; iSector++)
//        {
//            this->myFlashDevice.SectorArr[iSector] = dlp::String::HEXtoNumber_unsigned<unsigned int>(separated_parameters.at(8 + iSector));
//        }
//        return true;
//    }
    return false;
}

/** @brief  Gets flash sector number for a memory address
 * @param[in]   addr    Memory address location for which the sector number is required
 */
int LCr6500::GetSectorNum(unsigned int Addr)
{
    unsigned int i = 0;
//    for(i=0; i < this->myFlashDevice.numSectors; i++)
//    {
//        if(this->myFlashDevice.SectorArr[i] > Addr)
//            break;
//    }
    return i-1;
}

/** @brief      Creates a firmware file for LightCrafter 6500 EVM
 * @param[in]   new_firmware_filename  file name desired for the created firmware file
 * @param[in]   image_filenames a vector of strings with all images to be included in firmware file
 *
 * @retval  LCR6500_FIRMWARE_UPLOAD_IN_PROGRESS         A firmware upload is in progress, do NOT send any commands until upload is complete!
 * @retval  LCR6500_DLPC900_FIRMWARE_FILE_NOT_FOUND     Could NOT find blank DLPRR350 firmware file
 * @retval  LCR6500_FIRMWARE_FILE_NAME_INVALID          The desired firmware file name is NOT a valid file name
 * @retval  LCR6500_FIRMWARE_MEMORY_ALLOCATION_FAILED   Could NOT allocate memory for the firmware file
 * @retval  LCR6500_FIRMWARE_FILE_INVALID               The firmware created is invalid
 * @retval  LCR6500_IMAGE_FILE_FORMAT_INVALID           The file formats of the images in the input vector are invalid
 */
ReturnCode LCr6500::CreateFirmware(const std::string &new_firmware_filename, const std::vector<std::string> &image_filenames){
    ReturnCode ret;

//    int i = 0;
//    int frwm_ret = 0;
//    int count = 0;

//    long long dataLen;
//    unsigned char *pByteArray;

//    unsigned char *newFrmwImage;
//    unsigned int  newFrmwSize;

//    unsigned char compression;
//    unsigned int  compSize;


//    // If A firmware upload is in progress return error
//    if(this->FirmwareUploadInProgress()){
//        this->debug_.Msg("Cannot connect because firmware is uploading");
//        return ret.AddError(LCR6500_FIRMWARE_UPLOAD_IN_PROGRESS);
//    }


//    // Check that original DLPC350 firmware file exists
//    if(!dlp::File::Exists(this->DLPC900_firmware_.Get()))
//        return ret.AddError(LCR6500_DLPC900_FIRMWARE_FILE_NOT_FOUND);

//    // Check that new firmware file name is NOT empty
//    if(new_firmware_filename.empty())
//        return ret.AddError(LCR6500_FIRMWARE_FILE_NAME_INVALID);

//    // Load the original DLPC350 firmware file into the firmware api

//    dataLen = dlp::File::GetSize(this->DLPC900_firmware_.Get());
//    pByteArray = new (std::nothrow) unsigned char [dataLen];
//    if (pByteArray == nullptr)
//        return ret.AddError(LCR6500_FIRMWARE_MEMORY_ALLOCATION_FAILED);


//    // Read the firmware into memory
//    std::ifstream firmware_orig(this->DLPC900_firmware_.Get(), std::ifstream::binary);
//    firmware_orig.read((char *)pByteArray, dataLen);
//    firmware_orig.close();


//    // Read the firmware into firmware api
//    frwm_ret = Frmw_CopyAndVerifyImage(pByteArray, dataLen);
//    if (frwm_ret)
//    {
//        switch(frwm_ret)
//        {
//        case ERROR_FRMW_FLASH_TABLE_SIGN_MISMATCH:
//            this->debug_.Msg( "ERROR: Flash Table Signature doesn't match! Bad Firmware Image!");
//            delete[] pByteArray;
//            return ret.AddError(LCR6500_FIRMWARE_FILE_INVALID);
//        case ERROR_NO_MEM_FOR_MALLOC:
//            this->debug_.Msg( "Fatal Error! System Run out of memory");
//            delete[] pByteArray;
//            return ret.AddError(LCR6500_FIRMWARE_MEMORY_ALLOCATION_FAILED);
//        default:
//            break;
//        }
//    }

//    delete[] pByteArray;

//    // Check the firmware version
//    if ((Frmw_GetVersionNumber() & 0xFFFFFF) < RELEASE_FW_VERSION)
//        this->debug_.Msg( "WARNING: Old version of Firmware detected. Download the latest release from http://www.ti.com/tool/dlpr350.");


//    firmwarePath = new_firmware_filename;

//    // Initialize the firmware image buffer
//    count = (int)image_filenames.size();
//    Frmw_SPLASH_InitBuffer(count);


//    // Create a log file to document firmware build process
//    std::fstream log_file_out;
//    log_file_out.open("Frmw-build.log", std::fstream::out);


//    // Start to build firmware
//    log_file_out << "Building Images from specified BMPs\n\n";
//    for(i = 0; (i < MAX_SPLASH_IMAGES) && (i < count); i++)
//    {
//        // Check that the image file is the correct resolution
//        // Also checks it the image doesn't exist
//        if( !this->ImageResolutionCorrect(image_filenames.at(i)) ){
//            this->debug_.Msg("Did NOT add image " + dlp::Number::ToString(image_filenames.at(i)) + " to DLPC350 firmware");
//            continue;
//        }

//        //this->debug_.Msg( "Allocating memory for LightCrafter 6500 firmware image...");
//        dataLen = dlp::File::GetSize(image_filenames.at(i));
//        pByteArray = new (std::nothrow) unsigned char [dataLen];
//        if (pByteArray == nullptr)
//            return ret.AddError(LCR6500_IMAGE_MEMORY_ALLOCATION_FAILED);

//        // Read the image into memory
//        std::ifstream firmware(image_filenames.at(i), std::ifstream::binary);
//        firmware.read((char *)pByteArray, dataLen);
//        firmware.close();

//        // Log the uncompressed image size
//        log_file_out << image_filenames.at(i) << "\n";
//        log_file_out << "\t" << "Uncompressed Size = " << dataLen << " Compression type : ";

//        // Determine if the filename specifies the compression type
//        if (image_filenames.at(i).find("_nocomp.bmp") != std::string::npos){
//            compression = SPLASH_UNCOMPRESSED;
//        }
//        else if (image_filenames.at(i).find("_rle.bmp") != std::string::npos){
//            compression = SPLASH_RLE_COMPRESSION;
//        }
//        else if (image_filenames.at(i).find("_4line.bmp") != std::string::npos){
//            compression = SPLASH_4LINE_COMPRESSION;
//        }
//        else{
//            compression = SPLASH_NOCOMP_SPECIFIED;
//        }

//        // Add the image to the firmware
//        frwm_ret = Frmw_SPLASH_AddSplash(pByteArray, &compression, &compSize);

//        // Check if there was an error
//        if (frwm_ret < 0)
//        {
//            switch(frwm_ret)
//            {
//            case ERROR_NOT_BMP_FILE:
//            case ERROR_NOT_24bit_BMP_FILE:
//                this->debug_.Msg( "Error building firmware - " + image_filenames.at(i) + " NOT in BMP format");
//                return ret.AddError(LCR6500_IMAGE_FILE_FORMAT_INVALID);
//            case ERROR_NO_MEM_FOR_MALLOC:
//            case ERROR_NO_SPACE_IN_FRMW:
//                this->debug_.Msg( "Could not add " + image_filenames.at(i) + " - Insufficient memory");
//                return ret.AddError(LCR6500_FIRMWARE_NOT_ENOUGH_MEMORY);
//            default:
//                this->debug_.Msg( "Error building firmware with " + image_filenames.at(i) + " - Insufficient memory");
//                return ret.AddError(LCR6500_FIRMWARE_MEMORY_ALLOCATION_FAILED);
//            }
//        }

//        this->debug_.Msg("Added image " + dlp::Number::ToString(image_filenames.at(i)) + " to DLPC350 firmware");

//        // Log the compression applied to image
//        switch(compression)
//        {
//            case SPLASH_UNCOMPRESSED:
//                log_file_out << "Uncompressed";
//                break;
//            case SPLASH_RLE_COMPRESSION:
//                log_file_out << "RLE Compression";
//                break;
//            case SPLASH_4LINE_COMPRESSION:
//                log_file_out << "4 Line Compression";
//                break;
//            default:
//                break;
//        }

//        // Log the compressed size of the image
//        log_file_out << " Compressed Size = " << compSize << "\n\n";

//        delete[] pByteArray;
//    }

//    // Close the log file
//    log_file_out.close();

//    // Get the new flash image data
//    Frmw_Get_NewFlashImage(&newFrmwImage, &newFrmwSize);

//    // Save the new flash image data to a file
//    std::ofstream firmware(new_firmware_filename, std::ofstream::binary);
//    firmware.write((char *)newFrmwImage, newFrmwSize);
//    firmware.close();

    return ret;
}


/** @brief  Creates images to be included in LightCrafter 6500 firmware from a sequence of patterns
 * @param[in]   arg_pattern_sequence    \ref dlp::Pattern::Sequence type to make firmware images from
 * @param[in]   arg_image_filename_base Base image filename desired, index is appended to final images
 * @param[out]  ret_pattern_sequence    Pointer to return new \ref dlp::Pattern::Sequence
 * @param[out]  ret_image_filename_list vector of strings containing file names of the created firmware images
 *
 * @retval  DLP_PLATFORM_NOT_SETUP                  The LightCrafter 6500 object has NOT been properly set up
 * @retval  LCR6500_FIRMWARE_IMAGE_BASENAME_EMPTY   The desired firmware file name base is invalid
 * @retval  PATTERN_SEQUENCE_EMPTY                  The sequence to create the images from is empty
 * @retval  PATTERN_SEQUENCE_PATTERN_TYPES_NOT_EQUAL    The types for the patterns in the sequence do NOT match
 * @retval  PATTERN_DATA_TYPE_INVALID               The data type of the pattern in the sequence is NOT valid
 * @retval  FILE_DOES_NOT_EXIST                     Image file pointed to by pattern does NOT exist
 */
ReturnCode LCr6500::CreateFirmwareImages(const dlp::Pattern::Sequence   &arg_pattern_sequence,
                                         const std::string              &arg_image_filename_base,
                                         dlp::Pattern::Sequence         &ret_pattern_sequence,
                                         std::vector<std::string>       &ret_image_filename_list ){
    ReturnCode      ret;
    std::string     filename_temp;
    dlp::Pattern    temp_pattern;

    unsigned int dmd_rows = 0;
    unsigned int dmd_cols = 0;

    // Check that DLP_Platform is setup
    if(!this->isPlatformSetup())
        return ret.AddError(DLP_PLATFORM_NOT_SETUP);

    // Get DMD columns and rows
    this->GetColumns(&dmd_cols);
    this->GetRows(&dmd_rows);

    this->debug_.Msg("DMD resolution " + dlp::Number::ToString(dmd_cols) + " x " + dlp::Number::ToString(dmd_rows));


    // Check that image filename base is NOT empty
    this->debug_.Msg("Check that the image filename base is NOT empty...");
    if(arg_image_filename_base.empty())
        return ret.AddError(LCR6500_FIRMWARE_IMAGE_BASENAME_EMPTY);
    this->debug_.Msg("Image file basename is " + arg_image_filename_base);


    // Check that sequence is NOT empty
    this->debug_.Msg("Sequence contains " + dlp::Number::ToString(arg_pattern_sequence.GetCount()) + " patterns");
    if(arg_pattern_sequence.GetCount() <= 0 )
        return ret.AddError(PATTERN_SEQUENCE_EMPTY);


    // Check that sequence patterns all have same type
    if(!arg_pattern_sequence.EqualDataTypes())
        return ret.AddError(PATTERN_SEQUENCE_PATTERN_TYPES_NOT_EQUAL);
    this->debug_.Msg("Sequence has equal pattern types");


    // Get the first pattern
    arg_pattern_sequence.Get(0,&temp_pattern);

    // Check that sequence DOES have image data or files
    if((temp_pattern.data_type != dlp::Pattern::DataType::IMAGE_DATA) &&
       (temp_pattern.data_type != dlp::Pattern::DataType::IMAGE_FILE))
        return ret.AddError(PATTERN_DATA_TYPE_INVALID);
    this->debug_.Msg("Pattern Type correct");

    // If the pattern does not have exposures or periods set
    // use the default values entered during Setup()
    bool         override_timing    = false;
    unsigned int sequence_exposure  = this->sequence_exposure_.Get();
    unsigned int sequence_period    = this->sequence_period_.Get();

    if(temp_pattern.exposure == 0 || temp_pattern.period == 0){
        override_timing = true;
        std::cout << "Pattern timings should be overridden!" << std::endl;
    }

    // Clean the return sequence
    this->debug_.Msg("Clearing return sequence and image file list");
    ret_pattern_sequence.Clear();
    ret_image_filename_list.clear();


    // Create an empty composite image
    Image dlpc900_composite_image;
    dlpc900_composite_image.Create(dmd_cols,dmd_rows,Image::Format::MONO_INT);
    dlpc900_composite_image.FillImage( (int)0);

    // Reset pattern image index and bitplane
    unsigned char   bitplane_position   = 0;
    unsigned int    image_index         = 0;
    unsigned char   pattern_bitdepth    = 0;
    unsigned char   pattern_number      = 0;

    // Pixel variables
    PixelRGB        pattern_pixel_rgb;
    unsigned char   pattern_pixel       = 0;
    unsigned char   pixel_bitdepth_mask  = 0;
    int             temp_pixel          = 0;
    int             fw_pixel            = 0;


    // Create the new sequence and images
    for(unsigned int iPat = 0; iPat < arg_pattern_sequence.GetCount(); iPat++){
        dlp::Pattern grab_pattern;
        dlp::Image   temp_pattern_image;

        temp_pattern_image.Clear();

        // Get the current pattern from sequence
        this->debug_.Msg("Retrieving pattern " + dlp::Number::ToString(iPat));
        arg_pattern_sequence.Get(iPat,&grab_pattern);

        // Import the image data
        if(grab_pattern.data_type == dlp::Pattern::DataType::IMAGE_FILE){
            this->debug_.Msg(1,"Pattern Type is image file");

            // Check that file exists
            if(!dlp::File::Exists(grab_pattern.image_file))
                return ret.AddError(FILE_DOES_NOT_EXIST);

            this->debug_.Msg(1,"Pattern image file exists");

            // Load the image
            temp_pattern_image.Load(grab_pattern.image_file);
        }
        else if(grab_pattern.data_type == dlp::Pattern::DataType::IMAGE_DATA){
            this->debug_.Msg(1,"Pattern Type is image data");

            // Perform shallow copy of the image data
            temp_pattern_image = grab_pattern.image_data;
        }

        // Copy all pattern settings
        this->debug_.Msg(1,"Copy pattern information");
        temp_pattern.id        = grab_pattern.id;
        temp_pattern.bitdepth  = grab_pattern.bitdepth;
        temp_pattern.color     = grab_pattern.color;
        temp_pattern.data_type = dlp::Pattern::DataType::PARAMETERS;
        temp_pattern.image_data.Clear();
        temp_pattern.parameters.Clear();

        // If no times were included use the Setup() values
        if(!override_timing){
            temp_pattern.exposure  = grab_pattern.exposure;
            temp_pattern.period    = grab_pattern.period;
        }
        else{
            temp_pattern.exposure  = sequence_exposure;
            temp_pattern.period    = sequence_period;
        }

        // Check the image resolution
        if(!this->ImageResolutionCorrect(temp_pattern_image))
            return ret.AddError(LCR6500_IMAGE_RESOLUTION_INVALID);


        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Set the trigger source specificed during Setup()
        //temp_pattern.parameters.Set(this->trigger_source_);

        // Determine which bitplane to save pattern image to
        pattern_bitdepth = (unsigned char)DlpPatternBitdepthToLCr6500Bitdepth(temp_pattern.bitdepth);
        this->debug_.Msg(1,"Pattern bpp = " + dlp::Number::ToString(pattern_bitdepth));

        // Check if there are enough bitplanes left in the current image to store the new pattern
        if( (bitplane_position + pattern_bitdepth) > 24){
            // There is NOT enough space to add the pattern bitplanes to the composite image

            // Create new image filename
            filename_temp = arg_image_filename_base + dlp::Number::ToString(image_index) + ".bmp";

            // Save the current image            
            this->debug_.Msg("Saving new composite image: " + filename_temp);
            ret = this->SavePatternIntImageAsRGBfile(dlpc900_composite_image, filename_temp);
            if(ret.hasErrors()) return ret;

            // Add the new image to return image list vector
            ret_image_filename_list.push_back(filename_temp);

            // Clear the image
            dlpc900_composite_image.Clear();

            // Reallocate the image memory
            dlpc900_composite_image.Create(dmd_cols,dmd_rows,Image::Format::MONO_INT);
            dlpc900_composite_image.FillImage( (int) 0 );

            // Increment the image index and clear the image bitplane position
            image_index++;
            bitplane_position = 0;
        }

        // Determine if pattern is monochrome or RGB
        if(temp_pattern.color != dlp::Pattern::Color::RGB){

            // Pattern is monochrome

            // If the stored image is RGB convert it to monochrome
            temp_pattern_image.ConvertToMonochrome();

            // Create mask according to pattern bitdepth
            pixel_bitdepth_mask = (1 << pattern_bitdepth) - 1;

            // Add the pattern image to the image
            for( unsigned int yRow = 0; yRow < dmd_rows; yRow++){
                for( unsigned int xCol = 0; xCol < dmd_cols; xCol++){

                    // Get the monochrome pixel from the pattern image
                    temp_pattern_image.Unsafe_GetPixel(xCol,yRow,&pattern_pixel);

                    // Mask the pixels according to the pattern bitdepth
                    if(pattern_pixel > pixel_bitdepth_mask){
                        pattern_pixel = pixel_bitdepth_mask;
                    }
                    else{
                        pattern_pixel = pattern_pixel & pixel_bitdepth_mask;
                    }

                    // Convert the pattern image pixel to the FW image pixel
                    temp_pixel = (int) pattern_pixel;

                    // Shift the data to the correct starting bitplane
                    temp_pixel = temp_pixel <<  bitplane_position;

                    // Get the current fw pixel
                    dlpc900_composite_image.Unsafe_GetPixel( xCol, yRow, &fw_pixel);

                    // Update the fw pixel
                    fw_pixel = temp_pixel + fw_pixel;

                    // Set the uint pixel
                    dlpc900_composite_image.Unsafe_SetPixel( xCol, yRow, fw_pixel);
                }
            }

            this->debug_.Msg(1,"Setting pattern parameters");

            // Add the flash image index and pattern number to the pattern parameters
            temp_pattern.parameters.Set(Parameters::PatternImageIndex(image_index));
            temp_pattern.parameters.Set(Parameters::PatternBitplane(bitplane_position));

            // Add the pattern image data to the composite image
            this->debug_.Msg(1,"Pattern number    = " + dlp::Number::ToString(pattern_number));
            this->debug_.Msg(1,"Bitplane position = " + dlp::Number::ToString(bitplane_position));
            this->debug_.Msg(1,"Image index       = " + dlp::Number::ToString(image_index));

            // Increment the image bit position so this pattern image is NOT overwritten
            bitplane_position = bitplane_position + pattern_bitdepth;
        }
        else{
            // Pattern is RGB

            // Pattern image bitplane position and bitdepth have already been checked...

            // Check that image is RGB
            dlp::Image::Format temp_image_format;
            temp_pattern_image.GetDataFormat(&temp_image_format);
            if(temp_image_format != dlp::Image::Format::RGB_UCHAR)
                return ret.AddError(LCR6500_IMAGE_FORMAT_INVALID);

            // Add the red channel

                // Create mask according to pattern bitdepth
                pixel_bitdepth_mask = (1 << pattern_bitdepth) - 1;

                // Add the pattern image to the composite image
                for( unsigned int yRow = 0; yRow < dmd_rows; yRow++){
                    for( unsigned int xCol = 0; xCol < dmd_cols; xCol++){
                        // Get the monochrome pixel from the pattern image
                        temp_pattern_image.Unsafe_GetPixel(xCol,yRow,&pattern_pixel_rgb);

                        // Mask the pixels according to the pattern bitdepth
                        if(pattern_pixel_rgb.r > pixel_bitdepth_mask){
                            pattern_pixel_rgb.r = pixel_bitdepth_mask;
                        }
                        else{
                            pattern_pixel_rgb.r = pattern_pixel_rgb.r & pixel_bitdepth_mask;
                        }

                        // Convert the pattern image pixel to the FW image pixel
                        temp_pixel = (int) pattern_pixel_rgb.r;

                        // Shift the data to the correct starting bitplane
                        temp_pixel = temp_pixel <<  bitplane_position;

                        // Get the current fw pixel
                        dlpc900_composite_image.Unsafe_GetPixel( xCol, yRow, &fw_pixel);

                        // Update the fw pixel
                        fw_pixel = temp_pixel + fw_pixel;

                        // Set the uint pixel
                        dlpc900_composite_image.Unsafe_SetPixel( xCol, yRow, fw_pixel);
                    }
                }

                this->debug_.Msg(1,"Setting pattern parameters");

                // Add the flash image index and pattern number to the pattern parameters
                temp_pattern.parameters.Set(Parameters::PatternImageIndexRed(image_index));
                temp_pattern.parameters.Set(Parameters::PatternBitplaneRed(bitplane_position));

                // Add the pattern image data to the composite image
                this->debug_.Msg(1,"RGB Pattern number    = " + dlp::Number::ToString(pattern_number));
                this->debug_.Msg(1,"RGB Red Bitplane position = " + dlp::Number::ToString(bitplane_position));
                this->debug_.Msg(1,"RGB Red Image index       = " + dlp::Number::ToString(image_index));

                // Increment the image bit position so this pattern image is NOT overwritten
                bitplane_position = bitplane_position + pattern_bitdepth;

            // Check if the green channel will fit on the same composite image
                if( (bitplane_position + pattern_bitdepth) > 24){
                    // There is NOT enough space to add the pattern bitplanes to the composite image

                    // Create new image filename
                    filename_temp = arg_image_filename_base + dlp::Number::ToString(image_index) + ".bmp";

                    // Save the current image
                    this->debug_.Msg("Saving new composite image: " + filename_temp);
                    ret = this->SavePatternIntImageAsRGBfile(dlpc900_composite_image, filename_temp);
                    if(ret.hasErrors()) return ret;

                    // Add the new image to return image list vector
                    ret_image_filename_list.push_back(filename_temp);

                    // Clear the image
                    dlpc900_composite_image.Clear();

                    // Reallocate the image memory
                    dlpc900_composite_image.Create(dmd_cols,dmd_rows,Image::Format::MONO_INT);
                    dlpc900_composite_image.FillImage( (int) 0 );

                    // Increment the image index and clear the image bitplane position
                    image_index++;
                    bitplane_position = 0;
                }

            // Add the green channel to the composite image

                // Add the pattern image to the temp image if sequence has NOT been prepared
                for( unsigned int yRow = 0; yRow < dmd_rows; yRow++){
                    for( unsigned int xCol = 0; xCol < dmd_cols; xCol++){
                        // Get the monochrome pixel from the pattern image
                        temp_pattern_image.Unsafe_GetPixel(xCol,yRow,&pattern_pixel_rgb);

                        // Mask the pixels according to the pattern bitdepth
                        if(pattern_pixel_rgb.g > pixel_bitdepth_mask){
                            pattern_pixel_rgb.g = pixel_bitdepth_mask;
                        }
                        else{
                            pattern_pixel_rgb.g = pattern_pixel_rgb.g & pixel_bitdepth_mask;
                        }

                        // Convert the pattern image pixel to the FW image pixel
                        temp_pixel = (int) pattern_pixel_rgb.g;

                        // Shift the data to the correct starting bitplane
                        temp_pixel = temp_pixel <<  bitplane_position;

                        // Get the current fw pixel
                        dlpc900_composite_image.Unsafe_GetPixel( xCol, yRow, &fw_pixel);

                        // Update the fw pixel
                        fw_pixel = temp_pixel + fw_pixel;

                        // Set the uint pixel
                        dlpc900_composite_image.Unsafe_SetPixel( xCol, yRow, fw_pixel);
                    }
                }

                this->debug_.Msg(1,"Setting pattern parameters");

                // Add the flash image index and pattern number to the pattern parameters
                temp_pattern.parameters.Set(Parameters::PatternImageIndexGreen(image_index));
                temp_pattern.parameters.Set(Parameters::PatternBitplaneGreen(bitplane_position));

                // Add the pattern image data to the composite image
                this->debug_.Msg(1,"RGB Green Bitplane position = " + dlp::Number::ToString(bitplane_position));
                this->debug_.Msg(1,"RGB Green Image index       = " + dlp::Number::ToString(image_index));

                // Increment the image bit position so this pattern image is NOT overwritten
                bitplane_position = bitplane_position + pattern_bitdepth;

            // Check if the blue channel will fit on the same composite image
                if( (bitplane_position + pattern_bitdepth) > 24){
                    // There is NOT enough space to add the pattern bitplanes to the composite image

                    // Create new image filename
                    filename_temp = arg_image_filename_base + dlp::Number::ToString(image_index) + ".bmp";

                    // Save the current image
                    this->debug_.Msg("Saving new composite image: " + filename_temp);
                    ret = this->SavePatternIntImageAsRGBfile(dlpc900_composite_image, filename_temp);
                    if(ret.hasErrors()) return ret;

                    // Add the new image to return image list vector
                    ret_image_filename_list.push_back(filename_temp);

                    // Clear the image
                    dlpc900_composite_image.Clear();

                    // Reallocate the image memory
                    dlpc900_composite_image.Create(dmd_cols,dmd_rows,Image::Format::MONO_INT);
                    dlpc900_composite_image.FillImage( (int) 0 );

                    // Increment the image index and clear the image bitplane position
                    image_index++;
                    bitplane_position = 0;
                }

            // Add the blue channel to composite image

                // Add the pattern image to the composite image
                for( unsigned int yRow = 0; yRow < dmd_rows; yRow++){
                    for( unsigned int xCol = 0; xCol < dmd_cols; xCol++){
                        // Get the monochrome pixel from the pattern image
                        temp_pattern_image.Unsafe_GetPixel(xCol,yRow,&pattern_pixel_rgb);

                        // Mask the pixels according to the pattern bitdepth
                        if(pattern_pixel_rgb.b > pixel_bitdepth_mask){
                            pattern_pixel_rgb.b = pixel_bitdepth_mask;
                        }
                        else{
                            pattern_pixel_rgb.b = pattern_pixel_rgb.b & pixel_bitdepth_mask;
                        }

                        // Convert the pattern image pixel to the FW image pixel
                        temp_pixel = (int) pattern_pixel_rgb.b;

                        // Shift the data to the correct starting bitplane
                        temp_pixel = temp_pixel <<  bitplane_position;

                        // Get the current fw pixel
                        dlpc900_composite_image.Unsafe_GetPixel( xCol, yRow, &fw_pixel);

                        // Update the fw pixel
                        fw_pixel = temp_pixel + fw_pixel;

                        // Set the uint pixel
                        dlpc900_composite_image.Unsafe_SetPixel( xCol, yRow, fw_pixel);
                    }
                }

                this->debug_.Msg(1,"Setting pattern parameters");

                // Add the flash image index and pattern number to the pattern parameters
                temp_pattern.parameters.Set(Parameters::PatternImageIndexBlue(image_index));
                temp_pattern.parameters.Set(Parameters::PatternBitplaneBlue(bitplane_position));

                this->debug_.Msg(1,"RGB Blue Bitplane position = " + dlp::Number::ToString(bitplane_position));
                this->debug_.Msg(1,"RGB Blue Image index       = " + dlp::Number::ToString(image_index));

                // Increment the image bit position so this pattern image is NOT overwritten
                bitplane_position = bitplane_position + pattern_bitdepth;
        }

        this->debug_.Msg("Adding pattern to sequence");
        ret_pattern_sequence.Add(temp_pattern);
    }

    // Save the last composite image
        // Create new image filename
        filename_temp = arg_image_filename_base + dlp::Number::ToString(image_index) + ".bmp";

        // Save the current image
        ret = this->SavePatternIntImageAsRGBfile(dlpc900_composite_image, filename_temp);
        if(ret.hasErrors()) return ret;

        // Add the new image to return image list vector
        ret_image_filename_list.push_back(filename_temp);

        // Clear the image
        dlpc900_composite_image.Clear();

    // Copy over any parameters
    ret_pattern_sequence.parameters = arg_pattern_sequence.parameters;

    return ret;
}

/** @brief      Creates images to be included in LightCrafter 6500 firmware from an INT image
 * @param[in]   image_int       object of \ref dlp::Image type containing pattern image
 * @param[in]   filename        file name to save image object
 *
 * @retval  DLP_PLATFORM_NOT_SETUP  The LightCrafter 6500 object has NOT been properly set up
 */
ReturnCode LCr6500::SavePatternIntImageAsRGBfile(Image &image_int, const std::string &filename){
    ReturnCode ret;

    Image image_rgb;
    PixelRGB  new_pixel;

    int             pixel_int;
    unsigned int    pixel;

    unsigned int    dmd_rows;
    unsigned int    dmd_cols;

    // Check that DLP_Platform has been setup
    if(!this->isPlatformSetup())
        return ret.AddError(DLP_PLATFORM_NOT_SETUP);

    // Check that the input image is the correct resoultiong
    if(!this->ImageResolutionCorrect(image_int))
        return ret.AddError(LCR6500_IMAGE_RESOLUTION_INVALID);

    // Get DMD size
    this->GetColumns(&dmd_cols);
    this->GetRows(&dmd_rows);

    // Create the RGB image in memory
    ret = image_rgb.Create(dmd_cols,dmd_rows,Image::Format::RGB_UCHAR);
    if(ret.hasErrors()) return ret;

    // Fill the new RGB image with zeros
    new_pixel.r = 0;
    new_pixel.g = 0;
    new_pixel.b = 0;
    ret = image_rgb.FillImage(new_pixel);
    if(ret.hasErrors()) return ret;

    // Create the RGB image pixel by pixel
    for(unsigned int yRow = 0; yRow < dmd_rows; yRow++){
        for(unsigned int xCol = 0; xCol < dmd_cols; xCol++){

            // Get the int pixel
            image_int.Unsafe_GetPixel(xCol,yRow, &pixel_int);

            // Create the RGB pixel
            pixel = (unsigned int) pixel_int;
            new_pixel.r = (pixel >>  8) & 255;   // Pattern Image bitplanes  8 - 15
            new_pixel.g = (pixel >>  0) & 255;   // Pattern Image bitplanes  0 -  7
            new_pixel.b = (pixel >> 16) & 255;   // Pattern Image bitplanes 16 - 23

            // Set the pixel
            image_rgb.Unsafe_SetPixel(xCol,yRow,new_pixel);
        }
    }

    // Save the image to a file
    ret = image_rgb.Save(filename);

    return ret;
}

}
