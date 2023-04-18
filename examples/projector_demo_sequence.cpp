#include <iostream>
#include <string>
#include <functional>
#include <thread>
#include <dlp_sdk.hpp>
#include <opencv2/opencv.hpp>

//most of the code is adopted from https://github.com/alexandre-bernier/coro_eyes_sdk

void print_dlp_errors(dlp::ReturnCode err)
{
    unsigned int i;
    if(err.hasErrors()) {
        for(i=0; i<err.GetErrorCount(); i++) {
            std::cerr << "Error: " << err.GetErrors().at(i) << std::endl;
        }
    }

    if(err.hasWarnings()) {
        for(i=0; i<err.GetWarningCount(); i++) {
            std::cout << "Warning: " << err.GetWarnings().at(i) << std::endl;
        }
    }
}

void print_firmware_upload_progress(dlp::LCr4500 *projector)
{
    // Write first message
    std::cout << "Uploading: 0%" << std::flush;

    // Give time for the firmware upload to start
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    int progress = 0;
    do {
        // Sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Print progress
        std::cout << "\rUploading: " << projector->GetFirmwareUploadPercentComplete() << "% " << std::flush;
        switch(progress++) {
        case 0:
            std::cout << "|" << std::flush;
            break;
        case 1:
            std::cout << "/" << std::flush;
            break;
        case 2:
            std::cout << "—" << std::flush;
            break;
        case 3:
            std::cout << "\\" << std::flush;
            progress = 0;
            break;
        }
    } while(projector->FirmwareUploadInProgress());

    // Upload complete
    std::cout << "\rUpload done." << std::endl << std::flush;
}

dlp::Pattern::Sequence convert_cv_patterns_to_dlp(const std::vector<cv::Mat>& images)
{
    dlp::Pattern::Sequence dlp_pattern_sequence;

    // Variable holding a single structured light pattern
    dlp::Pattern dlp_pattern;
    dlp_pattern.color = dlp::Pattern::Color::WHITE;
    dlp_pattern.data_type = dlp::Pattern::DataType::IMAGE_DATA;
    dlp_pattern.bitdepth = dlp::Pattern::Bitdepth::MONO_1BPP;
    dlp_pattern.exposure = 34000;
    dlp_pattern.period = 34000;

    // Create the DLP pattern sequence
    for(unsigned int i=0; i<images.size(); i++) {

        dlp_pattern.image_data.Clear();
        dlp_pattern.image_data.Create(images[i]);
        dlp_pattern_sequence.Add(dlp_pattern);
    }

    return dlp_pattern_sequence;
}

int main(void)
{

    dlp::ReturnCode ret;    // Return variable of all DLP's methods
    dlp::LCr4500 projector; // Instance of the projector (DLP)
    dlp::Parameters param;  // DLP class to hold the projector settings (DLP)
    std::string proj_param_file = "/home/jiawenxue/projects/DLP-ALC-SDK-ubuntu/examples/dlp_configures/projector_settings.txt";   // Path to the projector settings file (DLP)

    std::cout << "Connecting..." << std::endl;
    ret = projector.Connect("");
    print_dlp_errors(ret);
    if(ret.hasErrors()) {
        std::cout << "Stopping application..." << std::endl;
        return -1;
    }

    std::cout << "Loading parameters..." << std::endl;
    ret = param.Load(proj_param_file);
    print_dlp_errors(ret);
    if(ret.hasErrors()) {
        std::cout << "Stopping application..." << std::endl;
        return -1;
    }

    std::cout << "Setting up projector..." << std::endl;
    ret = projector.Setup(param);
    print_dlp_errors(ret);
    if(ret.hasErrors()) {
        std::cout << "Stopping application..." << std::endl;
        return -1;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "Projecting white..." << std::endl;
    print_dlp_errors(projector.ProjectSolidWhitePattern());
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "Projecting black..." << std::endl;
    print_dlp_errors(projector.ProjectSolidBlackPattern());
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "Stopping projection..." << std::endl;
    print_dlp_errors(projector.StopPatternSequence());

    std::cout << "Generating patterns..." << std::endl;
    unsigned int proj_height;
    projector.GetRows(&proj_height);
    unsigned int proj_width;
    projector.GetColumns(&proj_width);


    /**
     * @section param Decide if we upload generated patterns to projector
     * Set parameter telling if we need to upload the patterns to the projector.
     * Only needs to be done once, or whenever the patterns are changed.
     * @snippet projector_example.cpp Upload parameter
     */
    // [Upload parameter]

    bool upload_patterns = true;
    dlp::Parameters upload_patterns_param;
    upload_patterns_param.Set(dlp::DLP_Platform::Parameters::SequencePrepared(!upload_patterns));
    projector.Setup(upload_patterns_param);
    if(upload_patterns)
        std::cout << "Patterns will be uploaded to the projector..." << std::endl;

    /**
     * @section firmware_progress Print firmware upload progress
     * Print firmware upload completion in a seperate thread since the upload takes full control.
     * Can't put the upload in a seperate thread, because the images generated get corrupted if
     * 'PreparePatternSequence' isn't in the main thread.
     * @snippet projector_example.cpp Print upload progress
     */
    // [Print upload progress]

    if(upload_patterns) {
        std::thread print_progress_thread(&print_firmware_upload_progress, &projector);
        print_progress_thread.detach();
    }

    /**
     * @section prepare Prepare patterns for the projector (DLP)
     * They will be sent to the projector if the dlp::DLP_Platform::Parameters::SequencePrepared parameter is set to false.
     * @snippet projector_example.cpp Prepare patterns
     */
    // [Prepare patterns]

    std::cout << "Preparing patterns..." << std::endl;

    std::vector<cv::Mat> sl_images;
    char str[1024];
    for(int idx=0; idx<12; idx++)
    {
        sprintf(str,"/home/jiawenxue/projects/DLP-ALC-SDK-ubuntu/images/debug/GRAYCODE_ALBEDO_VERTICAL_600x400_%d.bmp",idx);
        cv::Mat curimg = cv::imread(str,0);
        cv::resize(curimg,curimg,cv::Size(proj_width,proj_height));
        sl_images.push_back(curimg);
    }
        
    dlp::Pattern::Sequence dlp_pattern_sequence = convert_cv_patterns_to_dlp(sl_images);

    ret = projector.PreparePatternSequence(dlp_pattern_sequence);
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));    // Wait to allow the print_progress_thread to finish properly

    /**
     * @section project_patterns Start patterns projection
     * @snippet projector_example.cpp Project patterns
     */
    // [Project patterns]

    std::cout << "Projecting patterns..." << std::endl;
    ret = projector.StartPatternSequence(0, sl_images.size(), true);
    if(ret.hasErrors())
    {
        dlp::CmdLine::Print("Sequence failed..."+ret.ToString());
    }

    /**
     * @section wait Wait for projection to complete
     * @snippet projector_example.cpp Wait for projection
     */
    // [Wait for projection]

    dlp::DLP_Platform::Parameters::SequencePeriod sequence_period;
    param.Get(&sequence_period);
    unsigned int sequence_duration = sequence_period.Get() * sl_images.size();
    std::this_thread::sleep_for(std::chrono::microseconds((unsigned int)((float)sequence_duration*1.2)));

    /**
     * @section stop2 Stop projection (turn off the lamp)
     * @snippet projector_example.cpp Stop projection 2
     */
    // [Stop projection 2]

    std::cout << "Stopping projection..." << std::endl;
    print_dlp_errors(projector.ProjectSolidWhitePattern());
    print_dlp_errors(projector.ProjectSolidBlackPattern());
    print_dlp_errors(projector.StopPatternSequence());

    /**
     * @section disconnect Disconnect from the projector
     * @snippet projector_example.cpp Disconnect
     */
    // [Disconnect]

    std::cout << "Disconnecting..." << std::endl;
    print_dlp_errors(projector.Disconnect());

    return 0;
}
