/** @file   geometry.hpp
 *  @brief  Contains definition for the DLP SDK geometry classes
 *  @copyright  2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#include <common/debug.hpp>
#include <common/returncode.hpp>
#include <common/image/image.hpp>
#include <common/capture/capture.hpp>
#include <common/pattern/pattern.hpp>
#include <common/point_cloud/point_cloud.hpp>
#include <common/disparity_map.hpp>
#include <common/module.hpp>
#include <calibration/calibration.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>


#include <vector>
#include <string>

#define GEOMETRY_NO_ORIGIN_SET                              "GEOMETRY_NO_ORIGIN_SET"
#define GEOMETRY_CALIBRATION_NOT_COMPLETE                   "GEOMETRY_CALIBRATION_NOT_COMPLETE"
#define GEOMETRY_NULL_POINTER                               "GEOMETRY_NULL_POINTER"
#define GEOMETRY_ORIGIN_RAY_OUT_OF_RANGE                    "GEOMETRY_ORIGIN_RAY_OUT_OF_RANGE"
#define GEOMETRY_ORIGIN_PLANE_OUT_OF_RANGE                  "GEOMETRY_ORIGIN_PLANE_OUT_OF_RANGE"
#define GEOMETRY_VIEWPORT_ID_OUT_OF_RANGE                   "GEOMETRY_VIEWPORT_ID_OUT_OF_RANGE"
#define GEOMETRY_VIEWPORT_RAY_OUT_OF_RANGE                  "GEOMETRY_VIEWPORT_RAY_OUT_OF_RANGE"
#define GEOMETRY_DISPARITY_MAP_RESOLUTION_INVALID           "GEOMETRY_DISPARITY_MAP_RESOLUTION_INVALID"
#define GEOMETRY_DISPARITY_MAPS_SAME_ORIENTATION            "GEOMETRY_DISPARITY_MAPS_SAME_ORIENTATION"
#define GEOMETRY_DISPARITY_MAPS_MISMATCHED                  "GEOMETRY_DISPARITY_MAPS_MISMATCHED"
#define GEOMETRY_DISPARITY_MAP_ORIENTATION_INVALID          "GEOMETRY_DISPARITY_MAP_ORIENTATION_INVALID"
#define GEOMETRY_DISPARITY_MAP_ROW_ORIENTATION_INVALID      "GEOMETRY_DISPARITY_MAP_ROW_ORIENTATION_INVALID"
#define GEOMETRY_DISPARITY_MAP_COLUMN_ORIENTATION_INVALID   "GEOMETRY_DISPARITY_MAP_COLUMN_ORIENTATION_INVALID"
#define GEOMETRY_SETTINGS_EMPTY                             "GEOMETRY_SETTINGS_EMPTY"
#define GEOMETRY_POINT_CLOUD_EMPTY                          "GEOMETRY_POINT_CLOUD_EMPTY"
#define GEOMETRY_PLANE_ORIENTATION_INVALID                  "GEOMETRY_PLANE_ORIENTATION_INVALID"

#define GEOMETRY_TAN_2  -2.18503986326152

/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{

/** @class      Geometry
 *  @defgroup   Geometry
 *  @brief      Calculates real world location of points using data from
 *              disparity maps and calibration data
 *
 *  The geometry class calculates points in 3D space (real world) using the
 *  disparity map and calibration data. The geometry class also allows setting the
 *  origin to the projector or the camera.
 *
 */
class Geometry: public dlp::Module{
public:

    enum class PositiveDirectionX{
        LEFT,
        RIGHT,
        INVALID
    };

    enum class PositiveDirectionY{
        UP,
        DOWN,
        INVALID
    };

    class Parameters{
    public:
        DLP_NEW_PARAMETERS_ENTRY(ScaleXYZ,              "GEOMETRY_PARAMETERS_SCALE_XYZ", double, 1.0);

        DLP_NEW_PARAMETERS_ENTRY(FilterRaysEnable,      "GEOMETRY_PARAMETERS_FILTER_RAYS_ENABLE",   bool, true);
        DLP_NEW_PARAMETERS_ENTRY(FilterRaysError,       "GEOMETRY_PARAMETERS_FILTER_RAYS_ERROR",   float,  0.2);

        DLP_NEW_PARAMETERS_ENTRY(PointDistanceMax,      "GEOMETRY_PARAMETERS_POINT_DISTANCE_MAX", double,  0.0);
        DLP_NEW_PARAMETERS_ENTRY(PointDistanceMin,      "GEOMETRY_PARAMETERS_POINT_DISTANCE_MIN", double,  0.0);

        DLP_NEW_PARAMETERS_ENTRY(PositiveDirectionX,    "GEOMETRY_PARAMETERS_POSITIVE_DIRECTION_X", dlp::Geometry::PositiveDirectionX,dlp::Geometry::PositiveDirectionX::RIGHT);
        DLP_NEW_PARAMETERS_ENTRY(PositiveDirectionY,    "GEOMETRY_PARAMETERS_POSITIVE_DIRECTION_Y", dlp::Geometry::PositiveDirectionY,dlp::Geometry::PositiveDirectionY::UP);

        DLP_NEW_PARAMETERS_ENTRY(SmoothDisparity,       "GEOMETRY_PARAMETERS_SMOOTH_DISPARITY_ENABLE",   bool, true);
        DLP_NEW_PARAMETERS_ENTRY(OverSampleColumns,     "GEOMETRY_PARAMETERS_OVERSAMPLE_COLUMNS", unsigned int, 1);
        DLP_NEW_PARAMETERS_ENTRY(OverSampleRows,        "GEOMETRY_PARAMETERS_OVERSAMPLE_ROWS", unsigned int, 1);
        DLP_NEW_PARAMETERS_ENTRY(OverSamplePlanesDiamondAngle1,        "GEOMETRY_PARAMETERS_OVERSAMPLE_PLANES_DIAMOND_ANGLE_1", unsigned int, 1);
        DLP_NEW_PARAMETERS_ENTRY(OverSamplePlanesDiamondAngle2,        "GEOMETRY_PARAMETERS_OVERSAMPLE_PLANES_DIAMOND_ANGLE_2", unsigned int, 1);

        DLP_NEW_PARAMETERS_ENTRY(GenerateOriginPlanesVertical,          "GEOMETRY_PARAMETERS_GENERATE_ORIGIN_PLANES_VERTICAL",           bool, true);
        DLP_NEW_PARAMETERS_ENTRY(GenerateOriginPlanesHorizontal,        "GEOMETRY_PARAMETERS_GENERATE_ORIGIN_PLANES_HORIZONTAL",         bool, true);
        DLP_NEW_PARAMETERS_ENTRY(GenerateOriginPlanesDiamondAngle1,     "GEOMETRY_PARAMETERS_GENERATE_ORIGIN_PLANES_DIAMOND_ANGLE_1",    bool, true);
        DLP_NEW_PARAMETERS_ENTRY(GenerateOriginPlanesDiamondAngle2,     "GEOMETRY_PARAMETERS_GENERATE_ORIGIN_PLANES_DIAMOND_ANGLE_2",    bool, true);


    };

    // A*X + B*X + C*X = D
    // A = w.x
    // B = w.y
    // C = w.z
    struct PlaneEquation{
        cv::Point3d w;
        double GetA(){return w.x;}
        double GetB(){return w.y;}
        double GetC(){return w.z;}
        double d;
    };

    /** @class ViewPoint
     *  @brief Contains the camera or projector XYZ position in space and its optical rays and planes
     * */
    class ViewPoint{
    public:
        // XYZ position of Object (Single cv::Point3_ object
        cv::Point3d center;

        // Object Rays (List of cv::Point3_ objects)
        cv::Mat ray;

        // Object planes (List of cv::Point3_ objects)
        std::vector<PlaneEquation> plane_columns;
        std::vector<PlaneEquation> plane_rows;
        std::vector<PlaneEquation> plane_diamond_angle_1;
        std::vector<PlaneEquation> plane_diamond_angle_2;
    };

    Geometry();
    ~Geometry();

    ReturnCode Setup(const dlp::Parameters &settings);
    ReturnCode GetSetup(dlp::Parameters *settings) const;

    void Clear();

    ReturnCode SetOriginView(const dlp::Calibration::Data &origin_calib);

    ReturnCode AddView(const dlp::Calibration::Data &viewport_calib,
                                      unsigned int *ret_viewport_id);

    ReturnCode GetNumberOfViews(unsigned int *viewports);

    // Origin_x is the decoded value from the vertical patterns
    // Origin_y is the decoded value from the horizontal patterns
    // Viewport x & y are the camera pixel location
    ReturnCode Find3dLineLineIntersection(const unsigned int &origin_x, const unsigned int &origin_y,
                                            const unsigned int &viewport_id,
                                            const unsigned int &viewport_x, const unsigned int &viewport_y,
                                            Point *ret_xyz);
    void Unsafe_Find3dLineLineIntersection( const unsigned int &origin_x, const unsigned int &origin_y,
                                            const unsigned int &viewport_id,
                                            const unsigned int &viewport_x, const unsigned int &viewport_y,
                                            Point *ret_xyz);


    ReturnCode Find3dPlaneLineIntersection( const unsigned int &origin_plane,
                                            dlp::Pattern::Orientation orientation,
                                            const unsigned int &viewport_id,
                                            const unsigned int &viewport_x, const unsigned int &viewport_y,
                                            Point *ret_xyz);

    void Unsafe_Find3dPlaneLineIntersection(const unsigned int &origin_plane,
                                            dlp::Pattern::Orientation orientation,
                                            const unsigned int &viewport_id,
                                            const unsigned int &viewport_x, const unsigned int &viewport_y,
                                            Point *ret_xyz);



    ReturnCode GeneratePointCloud(const unsigned int  &viewport_id,
                                    dlp::DisparityMap   &disparity_1,
                                    dlp::DisparityMap   &disparity_2,
                                    dlp::Point::Cloud   *ret_cloud,
                                    dlp::Image          *ret_distancemap);


    ReturnCode GeneratePointCloud(  const unsigned int  &viewport_id,
                                    dlp::DisparityMap   &disparity,
                                    dlp::Point::Cloud   *ret_cloud,
                                    dlp::Image          *ret_distancemap);

    static ReturnCode ConvertDistanceMapToColor(const dlp::Image &distance_map, dlp::Image *color_depth);


    static ReturnCode CalculateFlatness(const dlp::Point::Cloud &cloud, double *flatness);

    bool ConvertAngledToXY(const int &angled_positive, const int &angled_negative,
                                 int *x, int *y);


private:
    Parameters::GenerateOriginPlanesVertical        generate_planes_vertical_;
    Parameters::GenerateOriginPlanesHorizontal      generate_planes_horizontal_;
    Parameters::GenerateOriginPlanesDiamondAngle1   generate_planes_diamond_angle_1_;
    Parameters::GenerateOriginPlanesDiamondAngle2   generate_planes_diamond_angle_2_;

    Parameters::FilterRaysEnable    filter_rays_enable_;
    Parameters::FilterRaysError     filter_rays_max_error_;

    Parameters::PointDistanceMax    max_distance_;
    Parameters::PointDistanceMin    min_distance_;

    Parameters::OverSampleColumns   oversample_columns_;
    Parameters::OverSampleRows      oversample_rows_;
    Parameters::OverSamplePlanesDiamondAngle1  oversample_angled_positive_;
    Parameters::OverSamplePlanesDiamondAngle2  oversample_angled_negative_;
    Parameters::SmoothDisparity     smooth_disparity_;


    Parameters::ScaleXYZ scale_xyz_;
    Parameters::PositiveDirectionX positive_direction_x_;
    Parameters::PositiveDirectionY positive_direction_y_;


    double scale_;
    double dir_x_;
    double dir_y_;
    double dir_z_;


    bool                                    origin_set_;
    ViewPoint                               origin_;
    dlp::Calibration::Data                  origin_calibration_;
    std::vector<dlp::Geometry::ViewPoint>   viewport_;

    static bool GenerateOpticalPoints(const unsigned int &columns,
                                        const unsigned int &rows,
                                        const float &compress_x,
                                        const float &compress_y,
                                        const float &shift_rows,
                                        const float &shift_columns,
                                      const unsigned int &over_sample_columns,
                                      const unsigned int &over_sample_rows,
                                        cv::Mat &intrinsics,
                                        cv::Mat &distortion,
                                        cv::Mat &undistorted_points,
                                        unsigned long long &ray_count,
                                        unsigned long long &total_columns,
                                        unsigned long long &total_rows);

    static PlaneEquation FitPlane(const cv::Mat &points);

};

namespace Number{
template <> std::string ToString<dlp::Geometry::PositiveDirectionX>( dlp::Geometry::PositiveDirectionX direction );
template <> std::string ToString<dlp::Geometry::PositiveDirectionY>( dlp::Geometry::PositiveDirectionY direction );
}

namespace String{
template <> dlp::Geometry::PositiveDirectionX ToNumber( const std::string &text, unsigned int base );
template <> dlp::Geometry::PositiveDirectionY ToNumber( const std::string &text, unsigned int base );
}

}
