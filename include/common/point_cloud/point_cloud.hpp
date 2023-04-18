/** @file   point_cloud.hpp
 *  @brief  Contains definitions for the DLP SDK point cloud class
 *  @copyright  2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#ifndef DLP_SDK_POINTCLOUD_HPP
#define DLP_SDK_POINTCLOUD_HPP

#include <common/debug.hpp>
#include <common/other.hpp>
#include <common/image/image.hpp>
#include <common/returncode.hpp>

#include <string>
#include <vector>
#include <atomic>

#define POINT_CLOUD_EMPTY                   "POINT_CLOUD_EMPTY"
#define POINT_CLOUD_INDEX_OUT_OF_RANGE      "POINT_CLOUD_INDEX_OUT_OF_RANGE"
#define POINT_CLOUD_FILE_SAVE_FAILED        "POINT_CLOUD_FILE_SAVE_FAILED"
#define POINT_CLOUD_NULL_POINTER_ARGUMENT   "POINT_CLOUD_NULL_POINTER_ARGUMENT"
#define POINT_CLOUD_FILENAME_EMPTY          "POINT_CLOUD_FILENAME_EMPTY"
#define POINT_CLOUD_GLFW_INIT_FAILED        "POINT_CLOUD_GLFW_INIT_FAILED"
#define POINT_CLOUD_GLFW_WINDOW_FAILED      "POINT_CLOUD_GLFW_FAILED"
#define POINT_CLOUD_FILE_DOES_NOT_EXIST     "POINT_CLOUD_FILE_DOES_NOT_EXIST"
#define POINT_CLOUD_FILE_OPEN_FAILED        "POINT_CLOUD_FILE_OPEN_FAILED"
#define POINT_CLOUD_FILE_MISSING_DIMENSION  "POINT_CLOUD_FILE_MISSING_DIMENSION"

/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{

/** @class  Point
 *  @brief  Stores a point in real space in x, y, z format
 */
class Point{
public:

    /** @class  Cloud
     *  @brief  Stores a collection of points and methods to clear the cloud and
     *          add individual points.
     *  @example point_cloud_viewer.cpp
     */
    class Cloud{
    public:

        Cloud();
        ~Cloud();

        void Clear();
        void Add(Point new_point);

        unsigned long long GetCount() const;

        ReturnCode Get(unsigned long long index, Point *ret_point)const;
        ReturnCode Remove(unsigned long long index);

        ReturnCode SaveXYZ(const std::string &filename, const unsigned char &delimiter = ' ')const;
        ReturnCode LoadXYZ(const std::string &filename, const unsigned char &delimiter = ' ');



        /** @class  Window
         *  @brief   Displays a point cloud using GLFW interface
         */
        class Window{
        public:
            Window();
            ~Window();

            ReturnCode Open(const std::string &title, const unsigned int &width = 0, const unsigned int &height = 0); // Sized empty window
            ReturnCode Update(const dlp::Point::Cloud &cloud);

            bool isOpen();
            void Close();
        private:
            void Loop(std::string title, bool fullscreen, unsigned int width, unsigned int height);
            std::atomic_bool is_open_;
            std::atomic_flag lock_data_access_;// = ATOMIC_FLAG_INIT;
            std::atomic_flag close_window_;// = ATOMIC_FLAG_INIT;
            std::vector<dlp::Point>    points_xyz_;
            std::vector<dlp::Point>    points_xyz_original_;
            std::vector<dlp::PixelRGB> points_rgb_;
            double x_min;
            double x_max;
            double y_min;
            double y_max;
            double z_min;
            double z_max;
            double x_ave;
            double y_ave;
            double z_ave;
        };

    private:
        std::vector<dlp::Point> points_;
    };

    typedef double PointType;
    Point();
    Point(const PointType &x_in, const Point::PointType &y_in, const PointType &z_in);
    Point(const PointType &x_in, const Point::PointType &y_in, const PointType &z_in, const PointType &distance_in);
    PointType x;
    PointType y;
    PointType z;
    PointType distance;
};


}


#endif // DLP_SDK_POINTCLOUD_HPP


