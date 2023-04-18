#include <iostream>
#include <string>
#include <functional>
#include <thread>
#include <dlp_sdk.hpp>
#include <opencv2/opencv.hpp>

void showError(std::string err)
{
    std::cerr<<"get error: "<<err.c_str()<<std::endl;
}

int main(void)
{
    //dlp params
    unsigned char status_hw_;
    unsigned char status_sys_;
    unsigned char status_main_;
    unsigned int  sequence_validation     = 0;

    //initialize usb connection
    if(DLPC350_USB_Init())
    {
        showError("Could not init usb!");
        return -1;
    }
    if(DLPC350_USB_Open())
    {
        showError("Could not connect!");
        return -1;
    }
    if(!DLPC350_USB_IsConnected())
    {
        showError("not connected.");
        return -1;
    }

    // Stop the display
    DLPC350_PatternDisplay(dlp::LCr4500::Pattern::DisplayControl::STOP);
    unsigned int patMode = 0;
    DLPC350_GetPatternDisplay(&patMode);
    if(patMode!=0)
    {
        DLPC350_PatternDisplay(dlp::LCr4500::Pattern::DisplayControl::STOP);
        while(patMode!=0)
        {
            dlp::Time::Sleep::Milliseconds(50); 
            DLPC350_GetPatternDisplay(&patMode);
        }
    }

    dlp::Time::Sleep::Milliseconds(10);
    if(DLPC350_GetStatus(&status_hw_,&status_sys_,&status_main_)<0)
    {
        showError("LCR4500_GET_STATUS_FAILED - Pattern display stop");
        return -1;
    }

    //set source as external video
    if(DLPC350_SetInputSource(0,1)<0)
    {
        showError("set up input source failed!");
        return -1;
    }

    // Setup the LUT entry
    //please note that the color projection order in parallel interface (e.g. HDMI) is grean, red, blue
    dlp::LCR4500_LUT_Entry pattern_entry_red;
    dlp::LCR4500_LUT_Entry pattern_entry_green;
    dlp::LCR4500_LUT_Entry pattern_entry_blue;

    //exposure and period for RGB are fixed! See Programmar's guide Page 64
    pattern_entry_red.exposure   = 4500;
    pattern_entry_red.period     = 4500;

    pattern_entry_green.exposure = 8500;
    pattern_entry_green.period   = 8500;

    pattern_entry_blue.exposure  = 2500;
    pattern_entry_blue.period    = 2500;

    pattern_entry_red.trigger_type   = dlp::LCr4500::Pattern::TriggerSource::NONE;
    pattern_entry_green.trigger_type = dlp::LCr4500::Pattern::TriggerSource::EXTERNAL_POSITIVE;
    pattern_entry_blue.trigger_type  = dlp::LCr4500::Pattern::TriggerSource::NONE;

    pattern_entry_red.bit_depth     = 7;
    pattern_entry_green.bit_depth   = 8;
    pattern_entry_blue.bit_depth    = 6;

    pattern_entry_red.LED_select   = dlp::LCr4500::Pattern::Led::RED;
    pattern_entry_green.LED_select = dlp::LCr4500::Pattern::Led::GREEN;
    pattern_entry_blue.LED_select  = dlp::LCr4500::Pattern::Led::BLUE;

    // Determine pattern number
    pattern_entry_red.pattern_number   = 1;
    pattern_entry_green.pattern_number = 0;
    pattern_entry_blue.pattern_number  = 2;

    // Different image so buffer swap is needed
    pattern_entry_red.buffer_swap   = true;
    pattern_entry_green.buffer_swap = true;
    pattern_entry_blue.buffer_swap  = true;

    // Determine share previous trigger out (exposure sharing)
    pattern_entry_red.trigger_out_share_prev    = false;
    pattern_entry_green.trigger_out_share_prev  = false;
    pattern_entry_blue.trigger_out_share_prev   = false;

    // Insert black fill should be used if period and exposure are NOT equal
    pattern_entry_red.insert_black      = true;
    pattern_entry_green.insert_black    = true;
    pattern_entry_blue.insert_black     = true;

    // Determine invert pattern
    pattern_entry_red.invert_pattern    = true;
    pattern_entry_green.invert_pattern  = pattern_entry_red.invert_pattern;
    pattern_entry_blue.invert_pattern   = pattern_entry_red.invert_pattern;


    // Stop the sequence if something is already running
    if(DLPC350_PatternDisplay(dlp::LCr4500::Pattern::DisplayControl::STOP) < 0)
    {
        showError(LCR4500_PATTERN_DISPLAY_FAILED);
        return -1;
    }

    // Set power mode to normal
    if(DLPC350_SetPowerMode(dlp::LCr4500::PowerStandbyMode::NORMAL) < 0 )
    {
        showError(LCR4500_SET_POWER_MODE_FAILED);
        return -1;
    }

    // Change device to pattern sequence mode
    // Check the current mode
    bool mode_previous;
    if(DLPC350_GetMode(&mode_previous)<0)
    {
        showError(LCR4500_GET_OPERATING_MODE_FAILED);
        return -1;
    }

    // The current mode is different from the requested setting change it
    if(mode_previous != dlp::LCr4500::OperatingMode::PATTERN_SEQUENCE){
        if(DLPC350_SetMode(dlp::LCr4500::OperatingMode::PATTERN_SEQUENCE) < 0)
        {
            showError(LCR4500_SET_OPERATING_MODE_FAILED);
            return -1;
        }
    }

    dlp::Time::Sleep::Milliseconds(10);
    if(DLPC350_GetStatus(&status_hw_,&status_sys_,&status_main_)<0)
    {
        showError("LCR4500_GET_STATUS_FAILED - get mode or set mode");
        return -1;
    }

    // Clear the LUT
    DLPC350_ClearExpLut();  // Hardcoded return value

    // Send the add the LUT entries
    // Green channel
    if(DLPC350_AddToExpLut(pattern_entry_green.trigger_type,
                               pattern_entry_green.pattern_number,
                               pattern_entry_green.bit_depth,
                               pattern_entry_green.LED_select,
                               pattern_entry_green.invert_pattern,
                               pattern_entry_green.insert_black,
                               pattern_entry_green.buffer_swap,
                               pattern_entry_green.trigger_out_share_prev,
                               pattern_entry_green.exposure,
                               pattern_entry_green.period)<0)
    {
        showError(LCR4500_ADD_EXP_LUT_ENTRY_FAILED);
        return -1;
    }

    // Red channel
    if(DLPC350_AddToExpLut(pattern_entry_red.trigger_type,
                               pattern_entry_red.pattern_number,
                               pattern_entry_red.bit_depth,
                               pattern_entry_red.LED_select,
                               pattern_entry_red.invert_pattern,
                               pattern_entry_red.insert_black,
                               pattern_entry_red.buffer_swap,
                               pattern_entry_red.trigger_out_share_prev,
                               pattern_entry_red.exposure,
                               pattern_entry_red.period)<0)
    {
        showError(LCR4500_ADD_EXP_LUT_ENTRY_FAILED);
        return -1;
    }

    // Blue channel
    if(DLPC350_AddToExpLut(pattern_entry_blue.trigger_type,
                               pattern_entry_blue.pattern_number,
                               pattern_entry_blue.bit_depth,
                               pattern_entry_blue.LED_select,
                               pattern_entry_blue.invert_pattern,
                               pattern_entry_blue.insert_black,
                               pattern_entry_blue.buffer_swap,
                               pattern_entry_blue.trigger_out_share_prev,
                               pattern_entry_blue.exposure,
                               pattern_entry_blue.period)<0)
    {
        showError(LCR4500_ADD_EXP_LUT_ENTRY_FAILED);
        return -1;
    }

    if(DLPC350_SetPatternDisplayMode(dlp::LCr4500::Pattern::Source::VIDEO_PORT)<0)
    {
        showError(LCR4500_SET_PATTERN_DISPLAY_MODE_FAILED);
        return -1;
    }

    // Set the trigger mode
    std::cout<<"Set pattern trigger mode..."<<std::endl;
    if(DLPC350_SetPatternTriggerMode(dlp::LCr4500::Pattern::TriggerMode::MODE_4_EXP_VSYNC)<0)
    {
        showError(LCR4500_SET_PATTERN_TRIGGER_MODE_FAILED);
        return -1;
    }

    // ***video streaming mode does nto require send the image LUT

    // Send the pattern LUT
    std::cout<<"Sending extended pattern lookup table..."<<std::endl;
    if(DLPC350_SendVarExpPatLut()<0)
    {
        showError(LCR4500_SEND_EXP_PATTERN_LUT_FAILED);
        return -1;
    }

    // Setup the pattern sequence
    if(DLPC350_SetVarExpPatternConfig(3,
                                      3,
                                      3,
                                      true)<0)
    {
        showError(LCR4500_SET_VAR_EXP_PATTERN_CONFIG_FAILED);
        return -1;
    }

    // Validate the sequence
    if(DLPC350_StartPatLutValidate() < 0)
    {
        showError(LCR4500_PATTERN_SEQUENCE_VALIDATION_FAILED);
        return -1;
    }

    dlp::Time::Sleep::Milliseconds(100);
    bool dlpc350_ready = false;
    while(!dlpc350_ready){
        dlp::Time::Sleep::Milliseconds(10);
        if(DLPC350_CheckPatLutValidate(&dlpc350_ready,&sequence_validation) < 0)
        {
            showError(LCR4500_PATTERN_SEQUENCE_VALIDATION_FAILED);
            return -1;
        }
    }

    // Display validation data if there was an error
    if( sequence_validation != 0 ){
        std::cout<<"Sequence validation FAILED!"<<std::endl;
        showError(LCR4500_SEQUENCE_VALIDATION_FAILED);

        if((sequence_validation & BIT0) == BIT0){
            std::cout<<"- Exposure or frame period OUT OF RANGE"<<std::endl;
            showError(LCR4500_SEQUENCE_VALIDATION_EXP_OR_PERIOD_OOO);
        }

        if((sequence_validation & BIT1) == BIT1){
            std::cout<<"- Pattern number in lookup table INVALID"<<std::endl;
            showError(LCR4500_SEQUENCE_VALIDATION_PATTERN_NUMBER_INVALID);
        }

        if((sequence_validation & BIT2) == BIT2){
            std::cout<<"- Continued output trigger OVERLAPS black vector"<<std::endl;
            showError(LCR4500_SEQUENCE_VALIDATION_OVERLAP_BLACK);
        }

        if((sequence_validation & BIT3) == BIT3){
            std::cout<<"- Black vector MISSING when exposure less than frame period"<<std::endl;
            showError(LCR4500_SEQUENCE_VALIDATION_BLACK_MISSING);
        }

        if((sequence_validation & BIT4) == BIT4){
            std::cout<<"Difference between exposure and frame period less than 230us"<<std::endl;
            showError(LCR4500_SEQUENCE_VALIDATION_EXP_PERIOD_DELTA_INVALID);
        }
        return -1;
    }


    dlp::Time::Sleep::Milliseconds(10);
    std::cout<<"Start pattern sequence..."<<std::endl;
    if( DLPC350_PatternDisplay(dlp::LCr4500::Pattern::DisplayControl::START) < 0)
    {
        showError(LCR4500_PATTERN_SEQUENCE_START_FAILED);
        return -1;
    }

    dlp::Time::Sleep::Milliseconds(10);
    if(DLPC350_GetStatus(&status_hw_,&status_sys_,&status_main_)<0)
    {
        showError("LCR4500_GET_STATUS_FAILED - pattern display start");
        return -1;
    }

    cv::Mat testimg = cv::imread("/home/jiawenxue/projects/image/album2.jpg");
    cv::resize(testimg,testimg,cv::Size(912,1140));
    cv::namedWindow("display",cv::WINDOW_AUTOSIZE);
    cv::imshow("display",testimg);
    cv::moveWindow("display",2560,0);
    cv::waitKey(1000);

    //add camera trigger test here

    //disconnect projector
    if(DLPC350_PatternDisplay(dlp::LCr4500::Pattern::DisplayControl::STOP) < 0)
    {
        showError(LCR4500_PATTERN_DISPLAY_FAILED);
        return -1;
    }

    DLPC350_USB_Close();
    DLPC350_USB_Exit();

    return 1;
}


