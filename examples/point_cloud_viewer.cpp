/** @file       point_cloud_viewer.cpp
 *  @brief      Demonstrates how to load and display dlp::Point::Cloud objects
 *  @copyright  2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#include <dlp_sdk.hpp>
#include <string>

int main(){

    // Get the filename of the point cloud from the user
    std::string filename;
    dlp::CmdLine::Get(filename,"Enter point cloud XYZ file name: ");

    // Load the point cloud file
    dlp::ReturnCode     ret;
    dlp::Point::Cloud   cloud;
    dlp::CmdLine::Print("Loading the point cloud file...");
    ret = cloud.LoadXYZ(filename);

    if(ret.hasErrors()){
        dlp::CmdLine::Print("Could not open point cloud file!");
        dlp::CmdLine::Print(ret.ToString());
        dlp::CmdLine::PressEnterToContinue("Please press ENTER to exit...");
        return 0;
    }

    // Display the instructions to use the point cloud viewer
    dlp::CmdLine::Print();
    dlp::CmdLine::Print("Point Cloud Viewer Operation:");
    dlp::CmdLine::Print("i/I = Zoom in");
    dlp::CmdLine::Print("o/O = Zoom out");
    dlp::CmdLine::Print("s/S = Save point cloud xyz file");
    dlp::CmdLine::Print("a/A = Auto-rotate the point cloud");
    dlp::CmdLine::Print("c/C = Turn point cloud color on/off");
    dlp::CmdLine::Print("\nPress ESC key to close the viewer");
    dlp::CmdLine::Print();
    dlp::CmdLine::PressEnterToContinue("Press ENTER to open the viewer...");

    // Open the point cloud viewer window
    dlp::Point::Cloud::Window viewer;
    ret = viewer.Open("Point Cloud Viewer - " + filename);

    if(ret.hasErrors()){
        dlp::CmdLine::Print("Could not open point cloud viewer!");
        dlp::CmdLine::Print(ret.ToString());
        dlp::CmdLine::PressEnterToContinue("Please press ENTER to exit...");
        return 0;
    }

    // Display the point cloud in the viewer
    dlp::CmdLine::Print("Updating the point cloud viewer content...");
    ret = viewer.Update(cloud);

    if(ret.hasErrors()){
        dlp::CmdLine::Print("Could not display point cloud!");
        dlp::CmdLine::Print(ret.ToString());
        dlp::CmdLine::PressEnterToContinue("Please press ENTER to exit...");
        return 0;
    }

    // Wait for viewer to close
    dlp::CmdLine::Print("Waiting for point cloud viewer to close...");
    while(viewer.isOpen()){}

    // Close and release memory
    viewer.Close();
    cloud.Clear();

    return 0;
}

