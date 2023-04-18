/** @file   point_cloud.cpp
 *  @brief  Contains methods for accessing, modifying, and storing point
 *          data and point clouds.
 *  @copyright 2014 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#include <GLFW/glfw3.h>

#include <common/other.hpp>
#include <common/debug.hpp>
#include <common/returncode.hpp>
#include <common/point_cloud/point_cloud.hpp>   // Adds dlp::PointCloud

#include <string>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <thread>
#include <functional>

/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{

void SavePointCloud( const dlp::Point::Cloud &cloud, const std::string &filename, const unsigned char &delimiter ){
    cloud.SaveXYZ(filename,delimiter);
}


static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);

    // Set scancode and mods so no warnings appear during compilation
    scancode++;
    mods++;
}

void Point::Cloud::Window::Loop(std::string title, bool fullscreen, unsigned int width, unsigned int height){
    GLFWwindow *glfw_window;

    if (!dlp::GLFW_Library::Init()){
        this->is_open_ = false;
        std::cout << "GLFW Init failed..." << std::endl;
        return;
    }

    if(fullscreen){
        // Get primary screen information
        GLFWmonitor*        primary_window      = glfwGetPrimaryMonitor();
        const GLFWvidmode*  primary_window_mode = glfwGetVideoMode( primary_window );

        // Create the window
        glfw_window = glfwCreateWindow(primary_window_mode->width, primary_window_mode->height, title.c_str(), primary_window, NULL);
    }
    else{
        glfw_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    }

    // Check that the window opened successfully
    if (!glfw_window){
        dlp::GLFW_Library::Terminate();
        this->is_open_ = false;
        std::cout << "GLFW Window failed..." << std::endl;
        return;
    }

    // Mark that window opened
    this->is_open_ = true;


    // Create the OpenGL contect and assign the key callback function
    glfwMakeContextCurrent(glfw_window);
    glfwSetKeyCallback(glfw_window, key_callback);

    float window_ratio  = 0;
    int   window_width  = 0;
    int   window_height = 0;

    double  mouseX = 0;
    double  mouseY = 0;
    double  cameraAngleY = 180;
    double  cameraAngleX = 20;
    double  cameraAngleZ = 0;
    double  xpos = 0;
    double  ypos = 0;
    double  cameraMoveY = 0;
    double  cameraMoveX = 0;
    double  scale = 4.0;

    bool close_window = false;
    bool display_color = true;
    bool auto_rotate = true;
    float rotate_sign = 1.0;

    // Create the OpenGL current
    glfwMakeContextCurrent(glfw_window);
    glfwSetKeyCallback(glfw_window, key_callback);

    glEnable(GL_DEPTH_TEST);

    while(!glfwWindowShouldClose(glfw_window) || close_window){

        // Lock the atomic flag while updating the OpenGL context
        while(this->lock_data_access_.test_and_set()) {}

        // Check if the window should close
        if(this->close_window_.test_and_set()) glfwSetWindowShouldClose(glfw_window,GL_TRUE);
        this->close_window_.clear();

        // Get the screen buffer size so that the point cloud and adjust
        // the OpenGL viewpoints so the every is displayed properly
        glfwGetFramebufferSize(glfw_window, &window_width, &window_height);

        // Calculate the aspect ratio
        window_ratio = float(window_width) / float(window_height);

        // Size and clear the OpenGL context
        glViewport(0, 0, window_width, window_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-window_ratio, window_ratio,
                -1.f,1.f,
                 1000*(this->z_min-this->z_ave)/ this->z_max,
                 1000*(this->z_max-this->z_ave)/ this->z_max);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Get the mouse position in the screen
        glfwGetCursorPos(glfw_window,&xpos,&ypos);

        // If the left button is held down, adjust the rotation of the cloud
        if(glfwGetMouseButton(glfw_window,GLFW_MOUSE_BUTTON_LEFT) &&
                ((xpos != mouseX) || (ypos != mouseY)))
        {
            auto_rotate = false;
            cameraAngleY -= (xpos - mouseX)*0.5;//*(1/scale);
            cameraAngleX -= (ypos - mouseY)*0.5;//*(1/scale);

            if(cameraAngleX > 360) cameraAngleX = 0;
            if(cameraAngleY > 360) cameraAngleY = 0;
            if(cameraAngleX < 0) cameraAngleX = 360;
            if(cameraAngleY < 0) cameraAngleY = 360;
        }   // If the right mouse button is held down, adjust the position of camera
        else if(glfwGetMouseButton(glfw_window,GLFW_MOUSE_BUTTON_RIGHT) &&
                ((xpos != mouseX) || (ypos != mouseY))) {
            auto_rotate = false;
            cameraMoveX += (xpos - mouseX)*(1/scale)/window_width*2;
            cameraMoveY -= (ypos - mouseY)*(1/scale)/window_height*2;
        }   // If the middle mouse button is held down, adjust the zoom of camera
        else if(glfwGetMouseButton(glfw_window,GLFW_MOUSE_BUTTON_MIDDLE) &&
                ((xpos != mouseX) || (ypos != mouseY))) {
            auto_rotate = false;
            scale -= (ypos - mouseY)*0.125;
            if(scale < 0) scale = 0;
        }
        else if(glfwGetKey(glfw_window,GLFW_KEY_R)){
            cameraAngleY = 180;
            cameraAngleX = 20;
            cameraAngleZ = 0;
            scale = 4.0;
        }
        else if(glfwGetKey(glfw_window,GLFW_KEY_C)){
            display_color = !display_color;
            dlp::Time::Sleep::Milliseconds(250);
        }
        else if(glfwGetKey(glfw_window,GLFW_KEY_A)){
            auto_rotate = !auto_rotate;
            dlp::Time::Sleep::Milliseconds(250);
        }
        else if(glfwGetKey(glfw_window,GLFW_KEY_S)){
            // Copy the point cloud data
            dlp::Point::Cloud temp;
            temp.points_ = this->points_xyz_original_; // Saves the original non-modified point cloud

            // Start the thread to save the file
            dlp::Time::Chronograph timer;
            std::string file_time = dlp::Number::ToString(timer.Reset());
            std::thread save_cloud(SavePointCloud,temp,"output/scan_data/" + file_time + "_point_cloud_viewer.xyz", ' ');
            save_cloud.detach();

            // Add delay to debounce key press
            dlp::Time::Sleep::Milliseconds(250);
        }
        else if(glfwGetKey(glfw_window,GLFW_KEY_O)){

            auto_rotate = false;
            scale -= 0.1;
            if(scale < 0) scale = 0;
        }
        else if(glfwGetKey(glfw_window,GLFW_KEY_I)){

            auto_rotate = false;
            scale += 0.1;
            if(scale < 0) scale = 0;
        }

        if(auto_rotate){
            cameraAngleY -= 0.125 * rotate_sign;
            dlp::Time::Sleep::Milliseconds(2);

            if(cameraAngleY > (180+90)){
                rotate_sign = 1.0;
            }
            else if(cameraAngleY < (180-90)){
                rotate_sign = -1.0;
            }

            if(cameraAngleY > 360) cameraAngleY = 0;
            if(cameraAngleY < 0) cameraAngleY = 360;
        }

        // Check if the escape button has been clicked
        if(glfwGetKey(glfw_window,GLFW_KEY_ESCAPE)) close_window = true;//glfwSetWindowShouldClose(glfw_window, GL_TRUE);

        // Update the previous mouse position variables
        mouseX = xpos;
        mouseY = ypos;

        // Scale, rotate, and positions the point cloud and camera
        glScalef(scale,scale,scale);
        glTranslatef( cameraMoveX, cameraMoveY, 0.0);
        glRotatef( cameraAngleX, 1.f, 0.f, 0.f);
        glRotatef( cameraAngleY, 0.f, 1.f, 0.f);
        glRotatef( cameraAngleZ, 0.f, 0.f, 1.f);
        glPointSize(1.5);

        // Add all points from the cloud to the OpenGL context
        glBegin(GL_POINTS);
        for(unsigned long long iPoint = 0; iPoint < this->points_xyz_.size(); iPoint++){

            if(display_color){
                glColor3f(  float(this->points_rgb_.at(iPoint).r)/255,
                            float(this->points_rgb_.at(iPoint).g)/255,
                            float(this->points_rgb_.at(iPoint).b)/255);
            }
            else{
                glColor3f(1.0,1.0,1.0);
            }

            glVertex3f( this->points_xyz_.at(iPoint).x,
                        this->points_xyz_.at(iPoint).y,
                        this->points_xyz_.at(iPoint).z);
        }
        glEnd();

        // Update the window display
        glfwSwapBuffers(glfw_window);
        glfwPollEvents();

        // Clear the atomic flag so the data can be updated
        this->lock_data_access_.clear();
    }
    glfwDestroyWindow(glfw_window);
    this->is_open_ = false;

    // Clear the atomic flag so the data can be updated
    this->lock_data_access_.clear();

    dlp::GLFW_Library::Terminate();
}

Point::Cloud::Window::Window(){
    this->is_open_ = false;
	this->close_window_.clear();
	this->lock_data_access_.clear();
    this->points_xyz_.clear();
    this->points_rgb_.clear();
}

Point::Cloud::Window::~Window(){
    this->Close();
}

ReturnCode Point::Cloud::Window::Open(const std::string &title, const unsigned int &width, const unsigned int &height){ // Sized empty window
    ReturnCode ret;

    if(!this->isOpen()){
        this->is_open_ = true;

        // If width or height equal 0 open fullscreen
        if((width == 0) || (height == 0)){
            std::thread window_loop(&Point::Cloud::Window::Loop, this, title, true, 0 , 0);
            window_loop.detach();
        }
        else{
            // Open the window to a specific size
            std::thread window_loop(&Point::Cloud::Window::Loop, this, title, false, width, height);
            window_loop.detach();
        }

        // Wait 50ms and check that window opened
        dlp::Time::Sleep::Milliseconds(50);

        if(!this->is_open_) ret.AddError(POINT_CLOUD_GLFW_WINDOW_FAILED);
    }

    return ret;
}

ReturnCode Point::Cloud::Window::Update(const dlp::Point::Cloud &cloud){
    ReturnCode ret;

    // Check that window is open
    if(!this->isOpen())
        return ret.AddError(POINT_CLOUD_GLFW_WINDOW_FAILED);

    // Check that the point cloud has points
    if(cloud.GetCount() == 0)
        return ret.AddError(POINT_CLOUD_EMPTY);

    // Set the flag to copy the data
    while(this->lock_data_access_.test_and_set()) {}

    // Clear the old data
    this->points_xyz_.clear();
    this->points_rgb_.clear();

    // Copy the data
    this->points_xyz_original_ = cloud.points_;
    this->points_xyz_          = cloud.points_;

    // Grab the first point to start the min max calculations
    this->x_min = this->points_xyz_.at(0).x;
    this->x_max = this->points_xyz_.at(0).x;
    this->y_min = this->points_xyz_.at(0).y;
    this->y_max = this->points_xyz_.at(0).y;
    this->z_min = this->points_xyz_.at(0).z;
    this->z_max = this->points_xyz_.at(0).z;
    this->x_ave = 0;
    this->y_ave = 0;
    this->z_ave = 0;

    // Find the minimum, maximum, and average
    for(unsigned long long iPoint = 0; iPoint < this->points_xyz_.size(); iPoint++){
        if(this->points_xyz_.at(iPoint).x < this->x_min) this->x_min = this->points_xyz_.at(iPoint).x;
        if(this->points_xyz_.at(iPoint).x > this->x_max) this->x_max = this->points_xyz_.at(iPoint).x;
        if(this->points_xyz_.at(iPoint).y < this->y_min) this->y_min = this->points_xyz_.at(iPoint).y;
        if(this->points_xyz_.at(iPoint).y > this->y_max) this->y_max = this->points_xyz_.at(iPoint).y;
        if(this->points_xyz_.at(iPoint).z < this->z_min) this->z_min = this->points_xyz_.at(iPoint).z;
        if(this->points_xyz_.at(iPoint).z > this->z_max) this->z_max = this->points_xyz_.at(iPoint).z;
        this->x_ave += this->points_xyz_.at(iPoint).x;
        this->y_ave += this->points_xyz_.at(iPoint).y;
        this->z_ave += this->points_xyz_.at(iPoint).z;
    }
    this->x_ave = this->x_ave / this->points_xyz_.size();
    this->y_ave = this->y_ave / this->points_xyz_.size();
    this->z_ave = this->z_ave / this->points_xyz_.size();

    // Create the point colors based on z-depth
    for(unsigned long long iPoint = 0; iPoint < this->points_xyz_.size(); iPoint++){
        float depth_temp = ((this->z_max - this->points_xyz_.at(iPoint).z) / (this->z_max - this->z_min));
        float red   = 1.0;
        float green = 1.0;
        float blue  = 1.0;

        if (depth_temp < (0.0 + 0.25 * 1.0)) {
            red = 0;
            green = 4 * (depth_temp - 0.0) / 1.0;
        } else if (depth_temp < (0.0 + 0.5 * 1.0)) {
            red = 0;
            blue = 1 + 4 * (0.0 + 0.25 * 1.0 - depth_temp) / 1.0;
        } else if (depth_temp < (0.0 + 0.75 * 1.0)) {
            red = 4 * (depth_temp - 0.0 - 0.5 * 1.0) / 1.0;
            blue = 0;
        } else {
            green = 1 + 4 * (0.0 + 0.75 * 1.0 - depth_temp) / 1.0;
            blue = 0;
        }
        this->points_rgb_.push_back(dlp::PixelRGB((unsigned char)(255*red),
                                                  (unsigned char)(255*green),
                                                  (unsigned char)(255*blue)));

        // Center and scale the cloud points
        this->points_xyz_.at(iPoint).x = (this->points_xyz_.at(iPoint).x - this->x_ave)/ this->z_max;
        this->points_xyz_.at(iPoint).y = (this->points_xyz_.at(iPoint).y - this->y_ave)/ this->z_max;
        this->points_xyz_.at(iPoint).z = (this->points_xyz_.at(iPoint).z - this->z_ave)/ this->z_max;
    }

    this->lock_data_access_.clear();

    return ret;
}

bool Point::Cloud::Window::isOpen(){
    return this->is_open_;
}

void Point::Cloud::Window::Close(){
    if(this->isOpen()){
        // Tell window to close
        while(this->close_window_.test_and_set()){}

        // Wait for window loop to finish processing
        while(this->lock_data_access_.test_and_set()){}

        // Clear all window data
        this->is_open_ = false;
        this->points_xyz_.clear();
        this->points_rgb_.clear();
        this->lock_data_access_.clear();
    }
}




}





