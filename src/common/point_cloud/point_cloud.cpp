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

/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{

/** @brief  Constructs point at 0,0,0 */
Point::Point(){
    this->x = 0.0;
    this->y = 0.0;
    this->z = 0.0;
    this->distance = 0.0;
}

/** @brief  Constructs point at supplied x, y, z location */
Point::Point(const Point::PointType &x_in, const PointType &y_in, const Point::PointType &z_in){
    this->x = x_in;
    this->y = y_in;
    this->z = z_in;
    this->distance = 0.0;
}


Point::Point(const PointType &x_in, const Point::PointType &y_in, const PointType &z_in, const PointType &distance_in){
    this->x = x_in;
    this->y = y_in;
    this->z = z_in;
    this->distance = distance_in;
}


/** @brief  Constructs empty point cloud*/
Point::Cloud::Cloud(){    
    this->Clear();
}

/** @brief Deallocates memory */
Point::Cloud::~Cloud(){
    this->Clear();
}

/** @brief Deallocates memory */
void Point::Cloud::Clear(){
    this->points_.clear();
}

/** @brief  Adds a point to a point cloud*/
void Point::Cloud::Add(Point new_point){
    this->points_.push_back(new_point);
}

/** @brief  Returns the number of points in a point cloud*/
unsigned long long Point::Cloud::GetCount() const{
    return this->points_.size();
}

/** @brief  Returns a point in a point cloud
 *  @param[in]  index       Point index in the point cloud
 *  @param[out] ret_point   Pointer to return \ref dlp::Point
 *  @retval POINT_CLOUD_EMPTY                   Point cloud has NO data
 *  @retval POINT_CLOUD_INDEX_OUT_OF_RANGE      Requested point does NOT exist
 *  @retval POINT_CLOUD_NULL_POINTER_ARGUMENT   Return argument NULL
 */
ReturnCode Point::Cloud::Get(unsigned long long index, Point *ret_point) const{
    ReturnCode ret;

    if(this->GetCount() == 0)
        return ret.AddError(POINT_CLOUD_EMPTY);

    if(index >= this->GetCount())
        return ret.AddError(POINT_CLOUD_INDEX_OUT_OF_RANGE);

    if(!ret_point)
        return ret.AddError(POINT_CLOUD_NULL_POINTER_ARGUMENT);

    (*ret_point) = this->points_.at(index);

    return ret;
}

/** @brief     Removes a point in a point cloud
 *  @param[in] index    Point index in the point cloud
 *  @retval POINT_CLOUD_EMPTY                   Point cloud has NO data
 *  @retval POINT_CLOUD_INDEX_OUT_OF_RANGE      Requested point does NOT exist
 */
ReturnCode Point::Cloud::Remove(unsigned long long index){
    ReturnCode ret;

    if(this->GetCount() == 0)
        return ret.AddError(POINT_CLOUD_EMPTY);

    if(index >= this->GetCount())
        return ret.AddError(POINT_CLOUD_INDEX_OUT_OF_RANGE);

    this->points_.erase(this->points_.begin() + index);

    return ret;
}

/** @brief  Saves point cloud data to a file and separates x, y,
 *          and z with the supplied delimiter
 *  @param[in] filename     Output file name
 *  @param[in] delimiter    Character to separate x, y, and z
 *  @retval POINT_CLOUD_EMPTY               Point cloud has NO data
 *  @retval POINT_CLOUD_FILE_SAVE_FAILED    Could NOT save file
 */
ReturnCode Point::Cloud::SaveXYZ(const std::string &filename, const unsigned char &delimiter)const{
    ReturnCode ret;
    std::ofstream myfile;

    // Check that filename is not empty
    if(filename.empty())
        return ret.AddError(POINT_CLOUD_FILENAME_EMPTY);

    // Open file
    myfile.open(filename);

    // Check that file opened
    if(!myfile.is_open())
        return ret.AddError(POINT_CLOUD_FILE_SAVE_FAILED);

    for(unsigned long long i = 0; i < this->points_.size();i++){
        dlp::Point point = this->points_.at(i);
        myfile << point.x << delimiter << point.y << delimiter << point.z  << "\n";
    }

    //Close file
    myfile.close();

    return ret;
}


/** @brief  Saves point cloud data to a file and separates x, y,
 *          and z with the supplied delimiter
 *  @param[in] filename     Input file name
 *  @param[in] delimiter    Character to separate x, y, and z values
 *  @retval POINT_CLOUD_EMPTY                   Point cloud has NO data
 *  @retval POINT_CLOUD_FILE_DOES_NOT_EXIST     Supplied file does not exist
 *  @retval POINT_CLOUD_FILE_OPEN_FAILED        Could not open the file for reading
 */
ReturnCode Point::Cloud::LoadXYZ(const std::string &filename, const unsigned char &delimiter){
    ReturnCode ret;

    // Check that file exists
    if(!dlp::File::Exists(filename))
        return ret.AddError(POINT_CLOUD_FILE_DOES_NOT_EXIST);


    // Open the file to read
    std::string     line;
    std::string     param_name;
    std::string     param_value;
    std::ifstream   point_cloud_file(filename.c_str());

    // Check that the file opened
    if(!point_cloud_file.is_open())
        return ret.AddError(POINT_CLOUD_FILE_OPEN_FAILED);

    // Clear the point cloud
    this->Clear();

    // Read a line while an error hasn't been found in the previous line
    while ( getline (point_cloud_file,line) )
    {
        dlp::Point new_point;
        std::vector<std::string> line_entries;

        // Seperate the delimited values
        line_entries = dlp::String::SeparateDelimited(line,delimiter);

        // Check that there are at least three entries
        if(line_entries.size() < 3)
            return ret.AddError(POINT_CLOUD_FILE_MISSING_DIMENSION);

        // Assume that the first column is X, second Y, and third Z
        new_point.x = dlp::String::ToNumber<dlp::Point::PointType>(line_entries.at(0));
        new_point.y = dlp::String::ToNumber<dlp::Point::PointType>(line_entries.at(1));
        new_point.z = dlp::String::ToNumber<dlp::Point::PointType>(line_entries.at(2));

        // Add this new point to the point cloud
        this->Add(new_point);
    }

    // Close the file
    point_cloud_file.close();

    return ret;
}

}





