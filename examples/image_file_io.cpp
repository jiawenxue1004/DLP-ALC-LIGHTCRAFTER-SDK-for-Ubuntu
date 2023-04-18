#include <dlp_sdk.hpp>
#include <iostream>

int main()
{
    // Create ErrorCode object
    dlp::ReturnCode ret;

    // Create Image object
    dlp::Image example_image;

    // Create string with image filename
    std::string image_file = "DLP_Logo.bmp";
    
    // Load the image file into the image object
    std::cout << "Loading image file..." << image_file << std::endl;

    ret = example_image.Load(image_file);

    if(ret.hasErrors()){
        std::cout << "Image load FAILED: "<< ret.ToString() << std::endl;
        std::cout << "Could not find file: " << image_file << std::endl;
        getchar();
        return 0;
    }

    std::cout << "Saving the loaded image as image_copy.bmp" << std::endl;
    ret = example_image.Save("image_copy.bmp");
    if(ret.hasErrors()){
        std::cout << "Image save FAILED: "<< ret.ToString() << std::endl;
        getchar();
        return 0;
    }

    return(0);
}

