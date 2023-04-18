/** @file       camera_config_pg_flycap2_c.cpp
 *  @brief      Demonstrates how to configure the PointGrey FlyCap2 Camera Module, also to save and load settings
 *  @copyright  2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#include <dlp_sdk.hpp>

/* Read user's input for camera configuration */
void ReadCameraSettings(dlp::Parameters *camera_settings){

    unsigned int tmpUIntVar;
    float tmpFloatVar;

    //Clear the Camera Settings first
    camera_settings->Clear();

    /***********************************************/
    /* Camera Capture Settings                     */
    /***********************************************/

    //Read the camera capture buffer size
    dlp::CmdLine::Get(tmpUIntVar,"Please enter camera capture buffer size: ");
    camera_settings->Set(dlp::Camera::Parameters::FrameBufferSize(tmpUIntVar));

    //Read Pixel Format
    dlp::CmdLine::Get(tmpUIntVar,"Enter FlyCap2 Camera Pixel Format, [0] - RAW8, [1]- MONO8, [2] - RGB8 [Suggested - MONO8 or RAW8]: ");
    while(tmpUIntVar >= 3)
    {
        // Ignore base variable
        if (tmpUIntVar == 0){
            camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::PixelFormat(dlp::PG_FlyCap2_C::PixelFormat::RAW8));
        }
        else if (tmpUIntVar == 1){
            camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::PixelFormat(dlp::PG_FlyCap2_C::PixelFormat::MONO8));
        }
        else {

            camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::PixelFormat(dlp::PG_FlyCap2_C::PixelFormat::RGB8));

            //Read Auto-White Balance enable option
            dlp::CmdLine::Get(tmpUIntVar, "Enable FlyCap2 Camera Auto WhiteBalance [1] - Enable, [0] - Disable [Suggested - Disable]:");
            if(tmpUIntVar) {
                camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::WhiteBalanceEnable(true));
            }
            else
            {
                camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::WhiteBalanceEnable(false));
            }

            //Read White Balance - Red Channel setting
            dlp::CmdLine::Get(tmpFloatVar, "Enable FlyCap2 Camera White Balance Red Channel [Suggested - 0.0]:");
            camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::WhiteBalanceRed(tmpFloatVar));

            //Read White Balance - Blue Channel setting
            dlp::CmdLine::Get(tmpFloatVar, "Enable FlyCap2 Camera White Balance Blue Channel [Suggested - 0.0]:");
            camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::WhiteBalanceBlue(tmpFloatVar));

            //Read Hue - Adjustment settings
            dlp::CmdLine::Get(tmpFloatVar, "Enable FlyCap2 Camera Hue settings [Suggested - 0.0]:");
            camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Hue(tmpFloatVar));

            //Read Saturation - Adjustment settings
            dlp::CmdLine::Get(tmpFloatVar, "Enable FlyCap2 Camera Saturation settings [Suggested - 0.0]:");
            camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Saturation(tmpFloatVar));
        }
    }


    //Read Shutter Exposure time (in ms)
    dlp::CmdLine::Get(tmpFloatVar,"Enter FlyCap2 Camera Shutter time (in ms): ");
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::ShutterTime(tmpFloatVar));

    //Read Frame rate (in Hz)
    dlp::CmdLine::Get(tmpFloatVar, "Enter FlyCap2 Camera Frame rate (in Hz): ");
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::FrameRate(tmpFloatVar));

    //Read Auto-Exposure enable option
    dlp::CmdLine::Get(tmpUIntVar, "Enable FlyCap2 Camera Auto Exposure [1] - Enable, [0] - Disable [Suggested - Disable]:");
    if(tmpUIntVar) {
        camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::AutoExposure(true));

        //Read Exposure time
        dlp::CmdLine::Get(tmpFloatVar, "Enter FlyCap2 Camera Exposure [0.0 - 1.0] :");
        camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Exposure(tmpFloatVar));

    }
    else
    {
        camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::AutoExposure(false));
    }


    /***********************************************/
    /* Camera Capture Image Settings               */
    /***********************************************/

    //Brightness Adjustment
    dlp::CmdLine::Get(tmpFloatVar,"Enter FlyCap2 Camera brightness adjustment [suggested - 0.0]): ");
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Brightness(tmpFloatVar));

    //Read Camera Sharpeness setting, TBD Range 0.0 - 1.0?
    dlp::CmdLine::Get(tmpFloatVar, "Enter FlyCap2 Camera Sharpness settings [Suggested - 1000.0]: ");
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Sharpness(tmpFloatVar));

    //Read Camera Gain setting, TBD Range 0.0 - 1.0?
    dlp::CmdLine::Get(tmpFloatVar, "Enter FlyCap2 Camera Gain settings [Suggested - 0.0]: ");
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Gain(tmpFloatVar));

    //Read Gamma - Adjustment settings
    dlp::CmdLine::Get(tmpFloatVar, "Enable FlyCap2 Camera Gamma settings [Suggested - 1.0]:");
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Gamma(tmpFloatVar));

    /***********************************************/
    /* Camera trigger input siganl configuration   */
    /***********************************************/

    //Read Camera Trigger Settings
    dlp::CmdLine::Get(tmpUIntVar, "Enable FlyCap2 Camera Enable Camera HW Trigger [1]: YES, [0]: NO: ");
    if(tmpUIntVar){
        camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerEnable(true));

        //Read Camera Trigger Polarity - Adjustment settings
        dlp::CmdLine::Get(tmpUIntVar, "Enable FlyCap2 Camera Trigger Polarity -  [0]: Failling Edge [1]: Raising Edge:");
        camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerPolarity(tmpUIntVar));

        //Read Camera Trigger Source - GPIO Pin
        dlp::CmdLine::Get(tmpUIntVar, "Enable FlyCap2 Camera Trigger Source -  [0]: GPIO Pin#0 [1]: GPIO #1 [Suggested - 0]:");
        camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerSource(tmpUIntVar));

        //Read Camera Trigger Mode
        dlp::CmdLine::Get(tmpUIntVar, "Enable FlyCap2 Camera Trigger Mode -  [0]: Trig_Mode_0, [1]: Trig_Mode_1, [15]:Trig_Mode_15 [Suggested - Trig_Mode_0]:");
        camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerMode(tmpUIntVar));

        //Read Camera Trigger Parameter
        dlp::CmdLine::Get(tmpUIntVar, "Enable FlyCap2 Camera Trigger Parameter [Suggested - 0]:");
        camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerParameter(tmpUIntVar));

        //Read Camera Trigger Delay
        dlp::CmdLine::Get(tmpFloatVar, "Enable FlyCap2 Camera Trigger Delay (in ms):");
        camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerDelay(tmpFloatVar));
    }
    else
    {
        camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerEnable(false));
    }


    /***********************************************/
    /* Camera trigger output signal configuration */
    /***********************************************/

    //Read Strobe Source GPIO O/P Pin of Camera? Just one trigger O/P we can skip this one
    dlp::CmdLine::Get(tmpUIntVar, "Enter FlyCap2 Camera Strobe Source: ");
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::StrobeSource(tmpUIntVar));

    //Read Strobe Enable option
    dlp::CmdLine::Get(tmpUIntVar, "Enable FlyCap2 Camera Strobe output [1]: YES, [0]: NO: ");
    if(tmpUIntVar){
        camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::StrobeEnable(true));

        //Read Strobe Polarity
        dlp::CmdLine::Get(tmpUIntVar, "Enable FlyCap2 Camera Strobe Polairty [1] - Active HIGH, [0] - Active LOW");
        camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::StrobePolarity(tmpUIntVar));

        //Read Strobe Delay
        dlp::CmdLine::Get(tmpFloatVar,"Enter FlyCap2 Camera Strobe Delay (in ms) ");
        camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::StrobeDelay(tmpFloatVar));

        //Read Strobe duration
        dlp::CmdLine::Get(tmpFloatVar, "Enter FlyCap2 Camera Strobe Duration (in ms):");
        camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::StrobeDuration(tmpFloatVar));

    }
    else
    {
        camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::StrobeEnable(false));
    }

    return;
}

void FillDefaultSettingsTriggerInputMode(dlp::Parameters *camera_settings){

    //Clear the Camera Settings first
    camera_settings->Clear();

    /***********************************************/
    /* Camera Capture Settings                     */
    /***********************************************/

    //Set the camera capture buffer size = 10
    camera_settings->Set(dlp::Camera::Parameters::FrameBufferSize(10));

    //Set Pixel Format to 8-Bit MONO
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::PixelFormat(dlp::PG_FlyCap2_C::PixelFormat::MONO8));

    //Set Shutter Exposure time (in ms)
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::ShutterTime(0));

    //Set Frame rate = 60Hz (in Hz)
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::FrameRate(60.0));

    //Disable Auto-Exposure enable option
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::AutoExposure(false));

    //Set Exposure time 16.67ms
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Exposure(16.67));

    /***********************************************/
    /* Camera Capture Image Settings               */
    /***********************************************/

    //Brightness = 0.0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Brightness(0.0));

    //Sharpeness = 0.0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Sharpness(1000.0));

    //Gain  = 0.0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Gain(0.0));

    //Disable Auto-White Balance
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::WhiteBalanceEnable(false));

    //White Balance - Red Channel setting = 0.0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::WhiteBalanceRed(0.0));

    //White Balance - Blue Channel setting = 0.0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::WhiteBalanceBlue(0.0));

    //Hue = 0.0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Hue(0.0));

    //Saturation = 0.0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Saturation(0.0));

    //Gamma = 1.0 (Linear Gamma)
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Gamma(1.0));

    /***********************************************/
    /* Camera trigger input siganl configuration   */
    /***********************************************/

    //Enable Camera Trigger Mode
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerEnable(true));

    //Set Trigger Polarity - Active high
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerPolarity(1));

    //Set Trigger input source = 0 (GPIO#0)
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerSource(0));

    //Camera Trigger Mode = Trigger_Mode_0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerMode(0));

    //Camera Trigger Parameter = 0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerParameter(0));

    //Camera Trigger Delay = 0.0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerDelay(0.0));

    /***********************************************/
    /* Camera trigger output signal configuration */
    /***********************************************/

    //Disable Strobe Enable option
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::StrobeEnable(false));

    //Default trigger output GPIO #1
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::StrobeSource(1));

    //Set Strobe Polarity - Active HIGH / Raising Edge
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::StrobePolarity(1));

    //Set Strobe Delay
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::StrobeDelay(0.0));

    //Set Strobe duration
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::StrobeDuration(0.0));

    return;
}

void FillDefaultSettingsStrobeOutputMode(dlp::Parameters *camera_settings){

    //Clear the Camera Settings first
    camera_settings->Clear();

    /***********************************************/
    /* Camera Capture Settings                     */
    /***********************************************/

    //Set the camera capture buffer size = 10
    camera_settings->Set(dlp::Camera::Parameters::FrameBufferSize(10));

    //Set Pixel Format to 8-Bit MONO
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::PixelFormat(dlp::PG_FlyCap2_C::PixelFormat::MONO8));

    //Set Shutter Exposure time (in ms)
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::ShutterTime(0));

    //Set Frame rate = 60Hz (in Hz)
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::FrameRate(60.0));

    //Disable Auto-Exposure enable option
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::AutoExposure(false));

    //Set Exposure time 16.67ms
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Exposure(16.67));

    /***********************************************/
    /* Camera Capture Image Settings               */
    /***********************************************/

    //Brightness = 0.0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Brightness(0.0));

    //Sharpeness = 0.0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Sharpness(1000.0));

    //Gain  = 0.0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Gain(0.0));

    //Disable Auto-White Balance
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::WhiteBalanceEnable(false));

    //White Balance - Red Channel setting = 0.0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::WhiteBalanceRed(0.0));

    //White Balance - Blue Channel setting = 0.0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::WhiteBalanceBlue(0.0));

    //Hue = 0.0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Hue(0.0));

    //Saturation = 0.0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Saturation(0.0));

    //Gamma = 1.0 (Linear Gamma)
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::Gamma(1.0));

    /***********************************************/
    /* Camera trigger input siganl configuration   */
    /***********************************************/

    //Enable Camera Trigger Mode
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerEnable(false));

    //Set Trigger Polarity - Active high
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerPolarity(1));

    //Set Trigger input source = 0 (GPIO#0)
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerSource(0));

    //Camera Trigger Mode = Trigger_Mode_0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerMode(0));

    //Camera Trigger Parameter = 0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerParameter(0));

    //Camera Trigger Delay = 0.0
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::TriggerDelay(0.0));

    /***********************************************/
    /* Camera trigger output signal configuration */
    /***********************************************/

    //Disable Strobe Enable option
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::StrobeEnable(true));

    //Default trigger output GPIO #1
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::StrobeSource(1));

    //Set Strobe Polarity - Active HIGH / Raising Edge
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::StrobePolarity(1));

    //Set Strobe Delay
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::StrobeDelay(0.0));

    //Set Strobe duration
    camera_settings->Set(dlp::PG_FlyCap2_C::Parameters::StrobeDuration(0.0));

    return;
}


/* Configure/Setup camera with User's applied settings */
void ConfigureCamera(dlp::Camera *camera, std::string *camera_id, dlp::Parameters *camera_settings) {

    dlp::ReturnCode ret;

    // Connect and setup camera
    dlp::CmdLine::Print();
    dlp::CmdLine::Print("Connecting and configuring camera ",camera_id);
    ret = dlp::Camera::ConnectSetup(*camera,*camera_id,*camera_settings,true);
    if(ret.hasErrors()){
        // Error messages displayed in ConnectSetup function above
        dlp::CmdLine::PressEnterToContinue("Press ENTER to exit...");
        return;
    }

    // Retrieve and print the actual camera settings
    dlp::Parameters temp;
    camera->GetSetup(&temp);
    dlp::CmdLine::Print();
    dlp::CmdLine::Print("Actual camera settings: ");
    dlp::CmdLine::Print(temp.ToString());
}

/* Livew Preview of applied camera settings */
void LivePreviewOfCameraWithAppliedConfig(dlp::Camera *camera) {

    dlp::ReturnCode ret;

    // Start the camera
    dlp::CmdLine::Print();
    dlp::CmdLine::Print("Starting the camera...");
    ret = camera->Start();
    // Check for error
    if(ret.hasErrors()){
        dlp::CmdLine::Print(ret.ToString());
        dlp::CmdLine::PressEnterToContinue("Press ENTER to exit...");
        return;
    }

    // Open a window to view the latest camera frame
    std::atomic_bool live_view_on(true);
    std::string live_view_title = "Camera  Live View - Hit ESC to close";

    //TBD - Not work on Linux and Mac better use CameraLiveView make it public
    dlp::Camera::StartLiveView(*camera,live_view_title,live_view_on);

    dlp::CmdLine::Print("Live camera view started...");
    dlp::CmdLine::PressEnterToContinue("\nPress ENTER to stop capture...");
    camera->Stop();

    // Open a window to view each frame in the camera buffer
    std::atomic_bool buffered_view_on(true);
    std::string buffered_view_title = "Camera Buffered View - Hit ESC to close";
    unsigned int image_playback_delay = 16;
    dlp::CmdLine::Get(image_playback_delay,"\nPlease enter delay in milliseconds between images: ");
    dlp::Camera::StartBufferedView(*camera,buffered_view_title,buffered_view_on,image_playback_delay);

    dlp::CmdLine::Print("Buffered camera view started...");
    dlp::CmdLine::PressEnterToContinue("\nPress ENTER to restart capture and close buffered view...");
    buffered_view_on = false;
    camera->Start();

    dlp::CmdLine::PressEnterToContinue("\nPress ENTER here to close the live view...");

    // Kill the live view thread with the atomic bool flag
    if(live_view_on){
        dlp::CmdLine::Print("Closing camera live view window...");
        live_view_on = false;
    }
    else{
        dlp::CmdLine::Print("Camera live view window already closed...");
    }

    // Stop the camera
    dlp::CmdLine::Print("Stopping camera capture...");
    camera->Stop();

    // Disconnect the camera
    dlp::CmdLine::Print("Disconnecting camera...");
    camera->Disconnect();
}


/* Save the camera configuration in the .txt file */
void SaveCameraConfiguration(std::string *param_filename, dlp::Parameters *camera_settings, bool *isSaveSuccess) {

    // Load calibration settings
    dlp::CmdLine::Print("Saving camera calibration settings...");
    if(camera_settings->Save(*param_filename).hasErrors()){

        // Camera calibration settings file did NOT load
        dlp::CmdLine::Print("Could NOT save camera configuration settings!");
        *isSaveSuccess = false;
        return;
    }
    else
    {
        *isSaveSuccess = true;
    }
}

/* Load the camera configuration in the .txt file */
void LoadCameraConfiguration(std::string *param_filename, dlp::Parameters *camera_settings, bool *isLoadSuccess) {

    // Load calibration settings
    dlp::CmdLine::Print("Loading camera calibration settings...");
    if(camera_settings->Load(*param_filename).hasErrors()){

        // Camera calibration settings file did NOT load
        dlp::CmdLine::Print("Could NOT load camera calibration settings!");
        *isLoadSuccess = false;
        return;
    }
    else
    {
        *isLoadSuccess = true;
    }
}


/* Sample code demonstrate how to configure camera module */
int main(){

    dlp::PG_FlyCap2_C camera;
    std::string       camera_id;
    std::string       fileName;
    dlp::Parameters   camera_settings;

    dlp::CmdLine::Print();
    dlp::CmdLine::Print();
    dlp::CmdLine::Print("[BEGIN]***Sample code demonstrating PG FlyCap2 camera configuration***");
    dlp::CmdLine::Print();
    dlp::CmdLine::Print();

    //Read the camera ID
    dlp::CmdLine::Get(camera_id,"Please enter camera ID: ");

    //Example:1 -

    dlp::CmdLine::Print();
    dlp::CmdLine::Print("Example-1: Read each settings from user input then apply");
    dlp::CmdLine::Print();

    //Create FlyCap2 Camera settings
    ReadCameraSettings(&camera_settings);

    // Print the desired camera settings
    dlp::CmdLine::Print();
    dlp::CmdLine::Print("Using the following settings to configure camera: ");
    dlp::CmdLine::Print(camera_settings.ToString());

    //Apply the settings to Camera
    ConfigureCamera(&camera,&camera_id,&camera_settings);

    //Test Applied Settings with Live Preview
    LivePreviewOfCameraWithAppliedConfig(&camera);

    //Example:2 -

    dlp::CmdLine::Print();
    dlp::CmdLine::Print("Example-2: Create Default Settings for External Trigger Input Mode");
    dlp::CmdLine::Print("To test this out you should connect an external trigger signal source");
    dlp::CmdLine::Print("For example: Signal Generator or a DLP LightCrafter Kit with trigger O/P signal configured");
    dlp::CmdLine::Print();

    //Read the FlyCap2 Camera settings
    FillDefaultSettingsTriggerInputMode(&camera_settings);

    //Modify specific settings on top of this: For example, Trigger Delay set to 10.0us
    camera_settings.Set(dlp::PG_FlyCap2_C::Parameters::TriggerDelay(10.0));

    // Print the desired camera settings
    dlp::CmdLine::Print();
    dlp::CmdLine::Print("Using the following settings to configure camera: ");
    dlp::CmdLine::Print(camera_settings.ToString());

    //Apply the settings to Camera
    ConfigureCamera(&camera,&camera_id,&camera_settings);

    //Test Applied Settings with Live Preview
    LivePreviewOfCameraWithAppliedConfig(&camera);

    //Example:3 -

    dlp::CmdLine::Print();
    dlp::CmdLine::Print("Example-3: Create Default Settings for Triggering DLP LightCrafter HW");
    dlp::CmdLine::Print("To test this out you should connect trigger/strobe signal from Camera to the DLP LightCrafter Input Trigger");
    dlp::CmdLine::Print();

    //Create FlyCap2 Camera settings
    FillDefaultSettingsStrobeOutputMode(&camera_settings);

    //Modify specific settings as per your requirement; example: strobe duration to 10ms
    camera_settings.Set(dlp::PG_FlyCap2_C::Parameters::StrobeDuration(10.0));

    // Print the desired camera settings
    dlp::CmdLine::Print();
    dlp::CmdLine::Print("Using the following settings to configure camera: ");
    dlp::CmdLine::Print(camera_settings.ToString());

    //Apply the settings to Camera
    ConfigureCamera(&camera,&camera_id,&camera_settings);

    //Test Applied Settings with Live Preview
    LivePreviewOfCameraWithAppliedConfig(&camera);

    //Example: 4 -

    bool result;
    dlp::CmdLine::Print();
    dlp::CmdLine::Print("Example-4: Demonstrate Save Configuration Settings into a txt file;");
    dlp::CmdLine::Print("           Load Camera Configuration Settings then apply to the camera");
    dlp::CmdLine::Print();

    //Create FlyCap2 Camera settings
    ReadCameraSettings(&camera_settings);

    //Save Settings in a txt file
    dlp::CmdLine::Get(fileName,"Please enter file name <file_name>.txt: ");


    SaveCameraConfiguration(&fileName,&camera_settings,&result);
    if(!result) {
        dlp::CmdLine::Print("Couldn't save the file");
    }

    //Load the settings from the text file
    dlp::CmdLine::Print();
    dlp::CmdLine::Print("Loading configuration from file...");
    LoadCameraConfiguration(&fileName,&camera_settings,&result);
    if(!result) {
        dlp::CmdLine::Print("Couldn't load the configuration settings from the file");
    }


    if(result)
    {
        // Print the desired camera settings
        dlp::CmdLine::Print();
        dlp::CmdLine::Print("Using the following settings to configure camera: ");
        dlp::CmdLine::Print(camera_settings.ToString());

        dlp::CmdLine::Print("Applying Settings to the camera");

        //Apply the settings to Camera
        ConfigureCamera(&camera,&camera_id,&camera_settings);

        //Test Applied Settings with Live Preview
        LivePreviewOfCameraWithAppliedConfig(&camera);
    }

    dlp::CmdLine::Print();
    dlp::CmdLine::Print();
    dlp::CmdLine::Print("[END]***Sample code demonstrating camera configuration***");
    dlp::CmdLine::Print();
    dlp::CmdLine::Print();

    return 0;
}

