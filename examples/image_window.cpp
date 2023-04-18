#include <common/debug.hpp>
#include <common/returncode.hpp>
#include <common/other.hpp>
#include <common/image/image.hpp>

#include <iostream>


int main()
{
    // Create ErrorCode object
    dlp::ReturnCode ret;

    // Create Image object
    dlp::Image example_image;
    dlp::Image::Window image_window;

    // Create string with image filename
    std::string image_file = "DLP_Logo.bmp";

    // Load the image file into the image object
    std::cout << "Loading image..." << image_file << std::endl;
    ret = example_image.Load(image_file);
    if(ret.hasErrors()){
        std::cout << "Image load FAILED: "<< ret.ToString() << std::endl;
        std::cout << "Could not find file: " << image_file << std::endl;
        getchar();
        return 0;
    }

    std::cout << "Opening an OpenCV window" << std::endl;
    ret = image_window.Open("Image Window Example");
    if(ret.hasErrors()){
        std::cout << "Could NOT open image window: "<< ret.ToString() << std::endl;
        getchar();
        return 0;
    }

    std::cout << "Updating the window to display the example image" << std::endl;
    ret = image_window.Update(example_image);
    if(ret.hasErrors()){
        std::cout << "Could NOT update image window: "<< ret.ToString() << std::endl;
        getchar();
        return 0;
    }

    // Wait for a key to be pressed
    std::cout << "Waiting for any key to be pressed while the window is selected to close it..." << std::endl;
    unsigned int return_key = 0;
    ret = image_window.WaitForKey(0, &return_key); // 0 signifies waiting until a key press

    std::cout << "ASCII Code of key pressed = " << return_key << " = " << (char)return_key << std::endl;

    // Close the window
    std::cout << "Closing the window.." << std::endl;
    image_window.Close();

    return(0);
}
