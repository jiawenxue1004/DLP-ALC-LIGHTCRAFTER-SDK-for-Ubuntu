#include <dlp_sdk.hpp>

int main(int argc, char *argv[]){
    dlp::ReturnCode ret;
    dlp::OpenCV_Cam camera;
    dlp::Parameters camera_settings;
    std::string     camera_id;

    dlp::CmdLine::Get(camera_id,"Please enter camera ID: ");

    // Example camera settings
    camera_settings.Clear();
    camera_settings.Set(dlp::Camera::Parameters::FrameBufferSize(100));
	
	// The following parameters are available through the OpenCV camera
	// module. However, not all cameras support or allow these to be set.
    // camera_settings.Set(dlp::OpenCV_Cam::Parameters::FrameRate(60));
    // camera_settings.Set(dlp::OpenCV_Cam::Parameters::Height(720));
    // camera_settings.Set(dlp::OpenCV_Cam::Parameters::Width(1280));
    // camera_settings.Set(dlp::OpenCV_Cam::Parameters::Brightness(50));
    // camera_settings.Set(dlp::OpenCV_Cam::Parameters::Contrast(25));
    // camera_settings.Set(dlp::OpenCV_Cam::Parameters::Saturation(12));
    // camera_settings.Set(dlp::OpenCV_Cam::Parameters::Hue(10));
    // camera_settings.Set(dlp::OpenCV_Cam::Parameters::Gain(10));
    // camera_settings.Set(dlp::OpenCV_Cam::Parameters::Exposure(10));

    // Print the desired camera settings
    dlp::CmdLine::Print();
    dlp::CmdLine::Print("Using the following settings to configure camera: ");
    dlp::CmdLine::Print(camera_settings.ToString());

    // Connect and setup camera
    dlp::CmdLine::Print();
    dlp::CmdLine::Print("Connecting and configuring camera ",camera_id);
    ret = dlp::Camera::ConnectSetup(camera,camera_id,camera_settings,true);

    if(ret.hasErrors()){
		// Error messages displayed in ConnectSetup function above
        dlp::CmdLine::PressEnterToContinue("Press ENTER to exit...");
        return 0;
    }

    // Retrieve and print the actual camera settings
    dlp::Parameters temp;
    camera.GetSetup(&temp);
    dlp::CmdLine::Print();
    dlp::CmdLine::Print("Actual camera settings: ");
    dlp::CmdLine::Print(temp.ToString());

    // Start the camera
    dlp::CmdLine::Print();
    dlp::CmdLine::Print("Starting the camera...");
    ret = camera.Start();

    // Check for error
    if(ret.hasErrors()){
        dlp::CmdLine::Print(ret.ToString());
        dlp::CmdLine::PressEnterToContinue("Press ENTER to exit...");
        return 0;
    }

    // Open a window to view the latest camera frame
    std::atomic_bool live_view_on(true);
    std::string live_view_title = "Camera  Live View - Hit ESC to close";

    dlp::Camera::StartLiveView(camera,live_view_title,live_view_on);

    dlp::CmdLine::Print("Live camera view started...");
    dlp::CmdLine::PressEnterToContinue("\nPress ENTER to stop capture...");
    camera.Stop();

    // Open a window to view each frame in the camera buffer
    std::atomic_bool buffered_view_on(true);
    std::string buffered_view_title = "Camera Buffered View - Hit ESC to close";
    unsigned int image_playback_delay = 16;
    dlp::CmdLine::Get(image_playback_delay,"\nPlease enter delay in milliseconds between images: ");
    dlp::Camera::StartBufferedView(camera,buffered_view_title,buffered_view_on,image_playback_delay);

    dlp::CmdLine::Print("Buffered camera view started...");
    dlp::CmdLine::PressEnterToContinue("\nPress ENTER to restart capture and close buffered view...");
    buffered_view_on = false;
    camera.Start();

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
    camera.Stop();

    // Disconnect the camera
    dlp::CmdLine::Print("Disconnecting camera...");
    camera.Disconnect();

    return 0;
}

