/** @file   image_window.cpp
 *  @brief  Contains methods to display \ref dlp::Image objects in a window
 *  @copyright 2014 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#include <string>
#include <vector>
#include <iostream>

#include <common/returncode.hpp>
#include <common/debug.hpp>
#include <common/other.hpp>
#include <common/image/image.hpp>

#include <opencv2/opencv.hpp>

/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{

std::vector<std::string> Image::Window::open_windows_;

/** @brief  Constructs dlp::Image window which is displayed after calling \ref Open() */
Image::Window::Window(){
    this->open_ = false;
    this->name_ = "";
}

/** @brief Deconstructs object */
Image::Window::~Window(){
    // Close the window
    this->Close();
}

/** @brief Returns true if window is open */
bool Image::Window::isOpen(){
    return this->open_;
}

/** @brief      Opens an OpenCV window to display \ref dlp::Image objects
 *  @param[in]  name                        Name of window that is displayed on top of the opened window
 *  @retval     IMAGE_WINDOW_NAME_TAKEN     A window has already been opened with the supplied name. NO new window is opened.
 */
ReturnCode Image::Window::Open(const std::string &name){
    ReturnCode ret;

    // Check that the window name hasn't already been opened
    for(unsigned int iWindow = 0; iWindow < Window::open_windows_.size(); iWindow++){
        // If the window name has already been used do NOT open it
        if(name.compare(Window::open_windows_.at(iWindow)) == 0)
            return ret.AddError(IMAGE_WINDOW_NAME_TAKEN);
    }

    // Add name to list
    Window::open_windows_.push_back(name);

    // Save the window information
    this->name_  = name;
    this->open_  = true;
    this->id_    = Window::open_windows_.size() - 1;

    // Open the window
    cv::namedWindow(name, CV_WINDOW_NORMAL);
    cv::waitKey(1);

    return ret;
}

/** @brief      Opens an OpenCV window to display the supplied \ref dlp::Image object
 *  @param[in]  name                        Name of window that is displayed on top of the opened window
 *  @param[in]  image                       \ref dlp::Image object to display when the window opens
 *  @retval     IMAGE_EMPTY                 Supplied \ref dlp::Image is empty. NO new window is opened
 *  @retval     IMAGE_WINDOW_NAME_TAKEN     A window has already been opened with the supplied name. NO new window is opened.
 */
ReturnCode Image::Window::Open(const std::string &name, const dlp::Image &image){
    ReturnCode ret;

    // Check that image has data
    if(image.isEmpty())
        return ret.AddError(IMAGE_EMPTY);

    // Check that the window name hasn't already been opened
    for(unsigned int iWindow = 0; iWindow < Window::open_windows_.size(); iWindow++){
        // If the window name has already been used do NOT open it
        if(name.compare(Window::open_windows_.at(iWindow)) == 0)
            return ret.AddError(IMAGE_WINDOW_NAME_TAKEN);
    }


    // Add name to list
    Window::open_windows_.push_back(name);

    // Save the window information
    this->name_  = name;
    this->open_  = true;
    this->id_    = Window::open_windows_.size() - 1;

    // Open the window
    cv::namedWindow(name, CV_WINDOW_NORMAL);
    cv::imshow(this->name_,image.data_);
    cv::waitKey(1);

    return ret;
}


/** @brief      Opens an OpenCV window to display the supplied \ref dlp::Image object at the specified resolution
 *  @param[in]  name                        Name of window that is displayed on top of the opened window
 *  @param[in]  image                       \ref dlp::Image object to display when the window opens
 *  @param[in]  width                       Desired width of displayed image in pixels
 *  @param[in]  height                      Desired height of displayed image in pixels
 *  @retval     IMAGE_EMPTY                 Supplied \ref dlp::Image is empty. NO new window is opened
 *  @retval     IMAGE_WINDOW_NAME_TAKEN     A window has already been opened with the supplied name. NO new window is opened.
 */
ReturnCode  Image::Window::Open(const std::string &name, const dlp::Image &image, const unsigned int &width, const unsigned int &height ){
    ReturnCode ret;
    unsigned int new_width  = width;
    unsigned int new_height = height;

    // Check that image has data
    if(image.isEmpty())
        return ret.AddError(IMAGE_EMPTY);

    // Check that the window name hasn't already been opened
    for(unsigned int iWindow = 0; iWindow < Window::open_windows_.size(); iWindow++){
        // If the window name has already been used do NOT open it
        if(name.compare(Window::open_windows_.at(iWindow)) == 0)
            return ret.AddError(IMAGE_WINDOW_NAME_TAKEN);
    }

    if(new_width  == 0) new_width  = image.data_.cols;
    if(new_height == 0) new_height = image.data_.rows;

    // Add name to list
    Window::open_windows_.push_back(name);

    // Save the window information
    this->name_  = name;
    this->open_  = true;
    this->id_    = Window::open_windows_.size() - 1;

    // Resize the image
    cv::Mat  image_resized;
    cv::Size new_size(new_width,new_height);
    cv::resize(image.data_,image_resized,new_size);

    // Open the window
    cv::namedWindow(name, CV_WINDOW_NORMAL );
    cv::imshow(this->name_,image_resized);
    cv::waitKey(1);

    return ret;
}

/** @brief  Updates the OpenCV window to display the supplied \ref dlp::Image object at the specified resolution
 *  @param[in]  image                   \ref dlp::Image object to display in the window
 *  @param[in]  width                       Desired width of displayed image in pixels
 *  @param[in]  height                      Desired height of displayed image in pixels
 *  @retval     IMAGE_EMPTY             Supplied \ref dlp::Image is empty. The window is NOT updated.
 *  @retval     IMAGE_WINDOW_NOT_OPEN   The window has NOT been opened yet. NO new window is opened.
 */
ReturnCode Image::Window::Update(const dlp::Image &image, const unsigned int &width, const unsigned int &height){
    ReturnCode ret;
    unsigned int new_width  = width;
    unsigned int new_height = height;

    // Check that window is open
    if(!this->isOpen())
        return ret.AddError(IMAGE_WINDOW_NOT_OPEN);

    // Check that the image is NOT empty
    if(image.isEmpty())
        return ret.AddError(IMAGE_EMPTY);

    if(new_width  == 0) new_width  = image.data_.cols;
    if(new_height == 0) new_height = image.data_.rows;

    // Resize the image
    cv::Mat  image_resized;
    cv::Size new_size(new_width,new_height);
    cv::resize(image.data_,image_resized,new_size);

    // Open the window
    cv::imshow(this->name_,image_resized);
    //cv::waitKey(1);

    return ret;
}

/** @brief  Updates the OpenCV window to display the supplied \ref dlp::Image object
 *  @param[in]  image                   \ref dlp::Image object to display in the window
 *  @retval     IMAGE_EMPTY             Supplied \ref dlp::Image is empty. The window is NOT updated.
 *  @retval     IMAGE_WINDOW_NOT_OPEN   The window has NOT been opened yet. NO new window is opened.
 */
ReturnCode Image::Window::Update(const dlp::Image &image){
    ReturnCode ret;

    // Check that window is open
    if(!this->isOpen())
        return ret.AddError(IMAGE_WINDOW_NOT_OPEN);

    // Check that the image is NOT empty
    if(image.isEmpty())
        return ret.AddError(IMAGE_EMPTY);

    // Update the window
    cv::imshow(this->name_,image.data_);
    //cv::waitKey(1);

    return ret;
}

/** @brief  Pauses execution until a key has been pressed while focused on the window
 *  @param[in]  delay_millisecs         Number of milliseconds for the program to pause. The value 0 is a special case that pauses the program indefinitely until a key has been pressed.
 *  @param[out] key_return              Pointer to return the ASCII value of what key was pressed
 *  @retval     IMAGE_WINDOW_NULL_POINTER_KEY_RETURN    Input argument is NULL
 *  @retval     IMAGE_WINDOW_NOT_OPEN                   The window has NOT been opened yet. NO new window is opened.
 */
ReturnCode Image::Window::WaitForKey(const unsigned int &delay_millisecs, unsigned int *key_return){
    ReturnCode ret;

    // Check that window is open
    if(!this->isOpen())
        return ret.AddError(IMAGE_WINDOW_NOT_OPEN);

    // Check pointer
    if(!key_return)
        return ret.AddError(IMAGE_WINDOW_NULL_POINTER_KEY_RETURN);

    // Wait for the key
    (*key_return) = cv::waitKey(delay_millisecs);

    return ret;
}

/** @brief      Pauses execution until a key has been pressed while focused on the window
 *  @param[in]  delay_millisecs         Number of milliseconds for the program to pause. The value 0 is a special case that pauses the program indefinitely until a key has been pressed.
 *  @retval     IMAGE_WINDOW_NOT_OPEN   The window has NOT been opened yet. NO new window is opened.
 */
ReturnCode Image::Window::WaitForKey(const unsigned int &delay_millisecs){
    ReturnCode ret;

    // Check that window is open
    if(!this->isOpen())
        return ret.AddError(IMAGE_WINDOW_NOT_OPEN);

    // Wait for the key
    cv::waitKey(delay_millisecs);

    return ret;
}

/** @brief  Closes the window if opened */
void Image::Window::Close(){
    // If the window is open close it
    if(this->open_){
        cv::destroyWindow(this->name_);
        Window::open_windows_.erase(Window::open_windows_.begin()+this->id_);
        this->open_ = false;
    }
}

}
