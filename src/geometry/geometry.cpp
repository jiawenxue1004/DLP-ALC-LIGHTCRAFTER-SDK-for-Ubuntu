/** @file   geometry.cpp
 *  @brief  Contains methods for setting geometry origins and finding intersections
 *  of lines in 3D space to determine point locations from disparity map and calibration data.
 *  @copyright 2014 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#include <common/debug.hpp>
#include <common/returncode.hpp>
#include <common/image/image.hpp>
#include <common/capture/capture.hpp>
#include <common/pattern/pattern.hpp>
#include <common/point_cloud/point_cloud.hpp>
#include <calibration/calibration.hpp>
#include <geometry/geometry.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>


#include <vector>
#include <string>
#include <math.h>

/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{

/** @brief Default Constructor origin set = false, set name GEOMETRY_DEBUG*/
Geometry::Geometry(){
    this->origin_set_ = false;
    this->is_setup_ = false;
    this->debug_.SetName("GEOMETRY_DEBUG(" + dlp::Number::ToString(this)+ "): ");
}

/** @brief Destroys object */
Geometry::~Geometry(){
    this->Clear();
}

/** @brief Releases all cv::Mat data objects */
void Geometry::Clear(){
    this->origin_set_ = false;
    this->origin_calibration_.Clear();
    this->origin_.ray.release();
    this->origin_.plane_columns.clear();
    this->origin_.plane_rows.clear();

    for(unsigned int iView = 0; iView < this->viewport_.size(); iView++){
        this->viewport_.at(iView).ray.release();
    }
}

/** @brief  Method to set point cloud scaling and distance thresholds
 *  @param[in] settings  \ref dlp::Parameters object
 */
ReturnCode Geometry::Setup(const dlp::Parameters &settings){
    ReturnCode ret;

    this->Clear();

    if(settings.isEmpty())
        return ret.AddError(GEOMETRY_SETTINGS_EMPTY);

    settings.Get(&this->filter_rays_enable_);
    settings.Get(&this->filter_rays_max_error_);
    settings.Get(&this->max_distance_);
    settings.Get(&this->min_distance_);

    settings.Get(&this->oversample_columns_);
    settings.Get(&this->oversample_rows_);
    settings.Get(&this->oversample_angled_positive_);
    settings.Get(&this->oversample_angled_negative_);

    settings.Get(&this->smooth_disparity_);

    settings.Get(&this->generate_planes_vertical_);
    settings.Get(&this->generate_planes_horizontal_);
    settings.Get(&this->generate_planes_diamond_angle_1_);
    settings.Get(&this->generate_planes_diamond_angle_2_);



    settings.Get(&this->scale_xyz_);
    settings.Get(&this->positive_direction_x_);
    settings.Get(&this->positive_direction_y_);

    this->scale_ = this->scale_xyz_.Get();

    if(this->positive_direction_x_.Get() == dlp::Geometry::PositiveDirectionX::RIGHT)
        this->dir_x_ =  1.0 * this->scale_;
    else
        this->dir_x_ = -1.0 * this->scale_;

    if(this->positive_direction_y_.Get() == dlp::Geometry::PositiveDirectionY::DOWN)
        this->dir_y_ =  1.0 * this->scale_;
    else
        this->dir_y_ = -1.0 * this->scale_;

    this->dir_z_ =  1.0 * this->scale_;

    this->is_setup_ = true;

    return ret;
}

/**
 * @brief Retrieves object settings
 * @param[out] settings         \ref dlp::Parameters object is empty
 * @retval GEOMETRY_NULL_POINTER    Return argument is NULL
 */
ReturnCode Geometry::GetSetup(dlp::Parameters *settings)const{
    ReturnCode ret;

    if(!settings)
        return ret.AddError(GEOMETRY_NULL_POINTER);


    settings->Set(this->filter_rays_enable_);
    settings->Set(this->filter_rays_max_error_);
    settings->Set(this->max_distance_);
    settings->Set(this->min_distance_);
    settings->Set(this->oversample_columns_);
    settings->Set(this->oversample_rows_);
    settings->Set(this->smooth_disparity_);

    return ret;
}


bool Geometry::GenerateOpticalPoints(const unsigned int &columns,
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
                                     unsigned long long &total_rows){

    unsigned long long xCol;
    unsigned long long yRow;
    unsigned long long iRay = 0;

    total_columns   = columns * over_sample_columns;
    total_rows      = rows * over_sample_rows;
    ray_count       = total_columns * total_rows;
    cv::Mat points;
    try{
        points.create(ray_count, 1, CV_64FC2);
        undistorted_points.create(ray_count, 1, CV_64FC2);
    }
    catch(...){
        return false;
    }

    // Create all of the points and compress if needed
    for(     yRow = 0; yRow < total_rows;    yRow++){
        for( xCol = 0; xCol < total_columns; xCol++){
            cv::Point2d temp;

            temp.x = double(xCol) * compress_x / over_sample_columns;
            temp.y = double(yRow) * compress_y / over_sample_rows;

            points.at<cv::Point2d>(iRay) = temp;
            iRay++;
        }
    }

    // Add shifts if required for a diamond array
    bool shift_row_enable    = false;
    bool shift_column_enable = false;
    unsigned int shift_x_count = 0;
    unsigned int shift_y_count = 0;
    if((shift_rows > 0.0) || (shift_columns > 0.0)){
        for(     yRow = 0; yRow < total_rows;    yRow++){
            for( xCol = 0; xCol < total_columns; xCol++){

                // Calculate the optical ray number
                iRay = xCol + (yRow*total_columns);

                // Load the optical point
                cv::Point2d temp = points.at<cv::Point2d>(iRay);

                // Shift the points
                if(shift_row_enable)    temp.x += shift_rows;
                if(shift_column_enable) temp.y += shift_columns;

                shift_x_count++;

                if(shift_x_count == over_sample_columns){
                    shift_column_enable = !shift_column_enable;
                    shift_x_count = 0;
                }

                // Save the shifted point
                points.at<cv::Point2d>(iRay) = temp;
            }


            shift_y_count++;
            if(shift_y_count == over_sample_rows){
                shift_row_enable = !shift_row_enable;
                shift_y_count = 0;
            }
        }
    }

    // Use the intrinsic and distortion parameters to correct for the optics
    cv::undistortPoints(points, undistorted_points, intrinsics, distortion);

    return true;
}

/** @brief  Creates System Geometry Origin/Reference point using \ref dlp::Calibration::Data
 *          and generates optical rays
 *  @retval GEOMETRY_CALIBRATION_NOT_COMPLETE   Supplied calibration data is NOT complete
 */
ReturnCode Geometry::SetOriginView(const dlp::Calibration::Data &origin_calib){
    ReturnCode ret;

    this->debug_.Msg("Setting origin viewpoint...");

    // Check that calibration is compelete
    if(!origin_calib.isComplete())
        return ret.AddError(GEOMETRY_CALIBRATION_NOT_COMPLETE);

    // Remove all previous views
    this->Clear();

    // Save the origin claibration
    this->origin_calibration_ = origin_calib;

    // Set the origin view center to 0,0,0
    this->debug_.Msg("Setting origin center to 0,0,0");
    this->origin_.center.x = 0.0;
    this->origin_.center.y = 0.0;
    this->origin_.center.z = 0.0;

    // Get calibration resolution
    unsigned int columns;
    unsigned int rows;
    unsigned long long ray_count;
    unsigned long long total_columns;
    unsigned long long total_rows;
    unsigned long long total_angled_lines;

    origin_calib.GetModelResolution(&columns,&rows);
    this->debug_.Msg("Model resolution = " + dlp::Number::ToString(columns) +" by " + dlp::Number::ToString(rows));


    // Get calibration data
    cv::Mat intrinsics;
    cv::Mat distortion;
    cv::Mat extrinsics;
    double  error;

    origin_calib.GetData(&intrinsics,&extrinsics,&distortion,&error);

    std::stringstream msg;
    msg << "Retrieved origin calibration" <<
           "\n\nIntrinsic (pixels) = \n" << intrinsics  <<
           "\n\nDistortion(pixels) = \n" << distortion <<
           "\n\nExtrinsic (real) = \n"   << extrinsics  << std::endl;
    this->debug_.Msg(msg);

    // Compute the optical rays
    this->debug_.Msg("Computing the optical rays...");

    // If diamond array projector add offset of 0.5 for every other row
    if(!origin_calib.isCamera() && (rows > columns)){
        this->debug_.Msg( "Projector has DMD with diamond array");
    }

    unsigned int xCol;
    unsigned int yRow;
    unsigned int iCode;
    unsigned int jPoint;
    cv::Mat original_rays;//(    ray_count, 1, CV_64FC2);
    cv::Mat undistorted_rays;//( original_rays.rows, 1, CV_64FC2);

    if(!origin_calib.isCamera() && (rows > columns)){
        // Diamond array
        this->debug_.Msg("Diamond array...");
        GenerateOpticalPoints(columns,
                              rows,
                              1.0, //const float &compress_x,
                              0.5, //const float &compress_y,
                              0.5,//const float &shift_x,
                              0.0,//const float &shift_y,
                              this->oversample_columns_.Get(), //const unsigned int &over_sample_columns,
                              this->oversample_rows_.Get(), //const unsigned int &over_sample_rows,
                              intrinsics,
                              distortion,
                              undistorted_rays,
                              ray_count,
                              total_columns,
                              total_rows);

    }
    else{
        // Orthogonal
        this->debug_.Msg("Orthogonal array...");
        GenerateOpticalPoints(columns,
                              rows,
                              1.0, //const float &compress_x,
                              1.0, //const float &compress_y,
                              0.0,//const float &shift_x,
                              0.0,//const float &shift_y,
                              this->oversample_columns_.Get(), //const unsigned int &over_sample_columns,
                              this->oversample_rows_.Get(), //const unsigned int &over_sample_rows,
                              intrinsics,
                              distortion,
                              undistorted_rays,
                              ray_count,
                              total_columns,
                              total_rows);
    }

    // Calculate total number of angled lines
    total_angled_lines = total_columns + (total_rows/2);



    unsigned int iRay = 0;

    // Translate to 3 dimensional rays
    this->origin_.ray.release();
    this->origin_.ray.create(total_rows,total_columns,CV_64FC3);

    xCol = 0;
    yRow = 0;
    iCode = 0;
    this->debug_.Msg("Normalizing optical rays...");
    for(iRay = 0; iRay < ray_count;iRay++){
        cv::Point2d ray             = undistorted_rays.at<cv::Point2d>(iRay);   // Not normalized
        double      ray_length      = sqrt(pow(ray.x,2)+pow(ray.y,2)+1.0);      // Assume unit length for z
        cv::Point3d ray_normalized;

        ray_normalized.x = ray.x / ray_length;
        ray_normalized.y = ray.y / ray_length;
        ray_normalized.z = 1.0   / ray_length;

        // Save the normalized ray in origin rays
        this->origin_.ray.at<cv::Point3d>(yRow,xCol) = ray_normalized;

        // Increment pixel location counters
        xCol++;

        if(xCol == total_columns){
            xCol = 0;
            yRow++;
        }
    }

    if(this->generate_planes_horizontal_.Get()){
        // Create row planes
        this->debug_.Msg("Create horizontal planes...");

        this->origin_.plane_rows.clear();

        for( yRow = 0; yRow < total_rows; yRow++){


            cv::Mat points(total_columns + 1,3,CV_64FC1);
            for( xCol = 0; xCol < total_columns; xCol++){
                points.at<double>(xCol,0) = this->origin_.ray.at<cv::Point3d>(yRow,xCol).x;
                points.at<double>(xCol,1) = this->origin_.ray.at<cv::Point3d>(yRow,xCol).y;
                points.at<double>(xCol,2) = this->origin_.ray.at<cv::Point3d>(yRow,xCol).z;
            }

            points.at<double>(total_columns,0) = this->origin_.center.x;
            points.at<double>(total_columns,1) = this->origin_.center.y;
            points.at<double>(total_columns,2) = this->origin_.center.z;

            // Fit the row and center points to a plane
            PlaneEquation plane_eq = Geometry::FitPlane(points);

            // Add plane equation to vector
            this->origin_.plane_rows.push_back(plane_eq);
        }
    }

    if(this->generate_planes_vertical_.Get()){
        // Create column planes
        this->debug_.Msg("Create vertical planes...");

        this->origin_.plane_columns.clear();

        for( xCol = 0; xCol < total_columns; xCol++){

            cv::Mat points(total_rows + 1,3,CV_64FC1);
            for( yRow = 0; yRow < total_rows; yRow++){
                points.at<double>(yRow,0) = this->origin_.ray.at<cv::Point3d>(yRow,xCol).x;
                points.at<double>(yRow,1) = this->origin_.ray.at<cv::Point3d>(yRow,xCol).y;
                points.at<double>(yRow,2) = this->origin_.ray.at<cv::Point3d>(yRow,xCol).z;
            }

            points.at<double>(total_rows,0) = this->origin_.center.x;
            points.at<double>(total_rows,1) = this->origin_.center.y;
            points.at<double>(total_rows,2) = this->origin_.center.z;

            // Fit the row and center points to a plane
            PlaneEquation plane_eq = Geometry::FitPlane(points);

            // Add plane equation to vector
            this->origin_.plane_columns.push_back(plane_eq);
        }
    }

    if(this->generate_planes_diamond_angle_1_.Get()){

        // Create column planes
        this->debug_.Msg("Create angled_positive planes...");


        // code     // points
        std::vector<std::vector<dlp::Point > > code_points;

        // Size the vectors to the origin dimensions
        code_points.resize(total_angled_lines);

        for(     yRow = 0; yRow < total_rows;    yRow++){
            for( xCol = 0; xCol < total_columns; xCol++){

                // Calculate the code
                unsigned int code = (yRow/2) + xCol;

                // Add the pixel to that code vector
                dlp::Point temp;
                temp.x = this->origin_.ray.at<cv::Point3d>(yRow,xCol).x;
                temp.y = this->origin_.ray.at<cv::Point3d>(yRow,xCol).y;
                temp.z = this->origin_.ray.at<cv::Point3d>(yRow,xCol).z;

                code_points.at(code).push_back(temp);
            }
        }

        this->origin_.plane_diamond_angle_1.clear();

        for( iCode = 0; iCode < total_angled_lines; iCode++){

            cv::Mat points(code_points.at(iCode).size() + 1,3,CV_64FC1);

            for( jPoint = 0; jPoint < code_points.at(iCode).size(); jPoint++){
                points.at<double>(jPoint,0) = code_points.at(iCode).at(jPoint).x;
                points.at<double>(jPoint,1) = code_points.at(iCode).at(jPoint).y;
                points.at<double>(jPoint,2) = code_points.at(iCode).at(jPoint).z;
            }

            points.at<double>(code_points.at(iCode).size(),0) = this->origin_.center.x;
            points.at<double>(code_points.at(iCode).size(),1) = this->origin_.center.y;
            points.at<double>(code_points.at(iCode).size(),2) = this->origin_.center.z;

            // Fit the row and center points to a plane
            PlaneEquation plane_eq = Geometry::FitPlane(points);

            // Add plane equation to vector
            this->origin_.plane_diamond_angle_1.push_back(plane_eq);
        }
    }


    if(this->generate_planes_diamond_angle_2_.Get()){
        this->debug_.Msg("Create angled negative planes...");

        // code     // points
        std::vector<std::vector<dlp::Point > > code_points;

        // Size the vectors to the origin dimensions
        code_points.resize(total_angled_lines);

        for(     yRow = 0; yRow < total_rows;    yRow++){
            for( xCol = 0; xCol < total_columns; xCol++){

                // Calculate the code
                unsigned int code = ((total_rows - yRow)/2) + xCol;

                // Add the pixel to that code vector
                dlp::Point temp;
                temp.x = this->origin_.ray.at<cv::Point3d>(yRow,xCol).x;
                temp.y = this->origin_.ray.at<cv::Point3d>(yRow,xCol).y;
                temp.z = this->origin_.ray.at<cv::Point3d>(yRow,xCol).z;
                code_points.at(code).push_back(temp);
            }
        }

        this->origin_.plane_diamond_angle_2.clear();

        for( iCode = 0; iCode < total_angled_lines; iCode++){

            cv::Mat points(code_points.at(iCode).size() + 1,3,CV_64FC1);

            for( jPoint = 0; jPoint < code_points.at(iCode).size(); jPoint++){
                points.at<double>(jPoint,0) = code_points.at(iCode).at(jPoint).x;
                points.at<double>(jPoint,1) = code_points.at(iCode).at(jPoint).y;
                points.at<double>(jPoint,2) = code_points.at(iCode).at(jPoint).z;
            }

            points.at<double>(code_points.at(iCode).size(),0) = this->origin_.center.x;
            points.at<double>(code_points.at(iCode).size(),1) = this->origin_.center.y;
            points.at<double>(code_points.at(iCode).size(),2) = this->origin_.center.z;

            // Fit the row and center points to a plane
            PlaneEquation plane_eq = Geometry::FitPlane(points);

            // Add plane equation to vector
            this->origin_.plane_diamond_angle_2.push_back(plane_eq);
        }
    }

    this->debug_.Msg("Origin viewport set");
    this->origin_set_ = true;

    return ret;
}

/** @brief  Creates a new \ref dlp::Geometry::ViewPoint with supplied
 *          \ref dlp::Calibration::Data. This method allows for multiple
 *          view ports which allows for multi-camera setups
 *  @retval GEOMETRY_NULL_POINTER               Return argument is NULL
 *  @retval GEOMETRY_CALIBRATION_NOT_COMPLETE   Supplied calibration data is NOT complete
 *  @retval GEOMETRY_NO_ORIGIN_SET              Origin view point has NOT been created
 */
ReturnCode Geometry::AddView(const dlp::Calibration::Data &viewport_calib,
                                             unsigned int *ret_viewport_id){

    ReturnCode ret;

    this->debug_.Msg("Adding viewport...");

    // Check that return viewport id int is not null
    if(!ret_viewport_id)
        return ret.AddError(GEOMETRY_NULL_POINTER);

    // Check that calibration is compelete
    if(!viewport_calib.isComplete())
        return ret.AddError(GEOMETRY_CALIBRATION_NOT_COMPLETE);

    // Check that the origin has been set
    if(!this->origin_set_)
        return ret.AddError(GEOMETRY_NO_ORIGIN_SET);

    Geometry::ViewPoint viewport_temp;

    // Get origin calibration data
    cv::Mat origin_intrinsic;
    cv::Mat origin_distortion;
    cv::Mat origin_extrinsic;
    double  origin_error;
    cv::Mat viewport_intrinsic;
    cv::Mat viewport_distortion;
    cv::Mat viewport_extrinsic;
    double  viewport_error;

    // Get origin calibration data
    this->origin_calibration_.GetData(&origin_intrinsic,
                                      &origin_extrinsic,
                                      &origin_distortion,
                                      &origin_error);




    // Get viewport calibration data
    viewport_calib.GetData(&viewport_intrinsic,
                           &viewport_extrinsic,
                           &viewport_distortion,
                           &viewport_error);

    std::stringstream msg;
    msg << "Retrieved new viewport calibration" <<
           "\n\nIntrinsic (pixels) = \n" << viewport_intrinsic  <<
           "\n\nDistortion(pixels) = \n" << viewport_distortion <<
           "\n\nExtrinsic (real) = \n"   << viewport_extrinsic  << std::endl;
    this->debug_.Msg(msg);

    // Get origin rotation and translation data.
    cv::Mat origin_rotation;
    cv::Mat origin_rotation_transposed;
    cv::Mat origin_rotation_3x3(3,3,CV_64FC1);
    cv::Mat origin_translation;
    cv::Mat origin_translation_transposed;
    cv::Mat viewport_rotation;
    cv::Mat viewport_rotation_transposed;
    cv::Mat viewport_rotation_3x3(3,3,CV_64FC1);
    cv::Mat viewport_translation;
    cv::Mat viewport_translation_transposed;
    cv::Mat viewport_center(3,1,CV_64FC1);
    cv::Mat empty_3x3(3,3,CV_64FC1);

    // Get extrinsic translation and rotation data
    origin_rotation    = origin_extrinsic.row(dlp::Calibration::Data::EXTRINSIC_ROW_ROTATION).clone();
    origin_translation = origin_extrinsic.row(dlp::Calibration::Data::EXTRINSIC_ROW_TRANSLATION).clone();


    viewport_rotation    = viewport_extrinsic.row(dlp::Calibration::Data::EXTRINSIC_ROW_ROTATION).clone();
    viewport_translation = viewport_extrinsic.row(dlp::Calibration::Data::EXTRINSIC_ROW_TRANSLATION).clone();

    // Convert the rotation data to 3x3 matrices
    this->debug_.Msg("Converting 3x1 extrinsic data to 3x3 matricies...");
    cv::Rodrigues(origin_rotation,origin_rotation_3x3);
    cv::Rodrigues(viewport_rotation,viewport_rotation_3x3);

    // Transpose origin rotation and translation
    cv::transpose(origin_rotation,origin_rotation_transposed);
    cv::transpose(origin_translation,origin_translation_transposed);

    // Transpose viewport rotation and translation
    cv::transpose(viewport_rotation,viewport_rotation_transposed);
    cv::transpose(viewport_translation,viewport_translation_transposed);

    // Calculate the viewports center point
    this->debug_.Msg("Calculating new viewport center location...");
    cv::gemm(viewport_rotation_3x3, viewport_translation_transposed, -1, empty_3x3, 0, viewport_center, CV_GEMM_A_T);
    cv::gemm(origin_rotation_3x3, viewport_center, 1, origin_translation_transposed, 1, viewport_center, 0);

    viewport_temp.center.x = viewport_center.at<double>(0);
    viewport_temp.center.y = viewport_center.at<double>(1);
    viewport_temp.center.z = viewport_center.at<double>(2);


    msg.str(std::string());
    msg << "Origin viewport center = " << this->origin_.center << std::endl;
    msg << "New viewport center    = " << viewport_temp.center << std::endl;
    this->debug_.Msg(msg);

    // Get viewport calibration resolution
    unsigned int columns;
    unsigned int rows;
    unsigned int ray_count;

    viewport_calib.GetModelResolution(&columns,&rows);
    this->debug_.Msg("New model resolution = " + dlp::Number::ToString(columns) +" by " + dlp::Number::ToString(rows));

    ray_count = columns * rows;
    this->debug_.Msg("Total number of rays = " + dlp::Number::ToString(ray_count));

    // Compute the optical rays
    this->debug_.Msg("Computing the optical rays...");
    cv::Mat original_rays(    ray_count, 1, CV_64FC2);
    cv::Mat undistorted_rays( ray_count, 1, CV_64FC2);

    unsigned int xCol;
    unsigned int yRow;
    unsigned int iRay = 0;
    for(     yRow = 0; yRow < rows;    yRow++){
        for( xCol = 0; xCol < columns; xCol++){
            cv::Point2d temp;
            temp.x = (double)xCol;
            temp.y = (double)yRow;

            // If diamond array projector add offset of 0.5 for every other row
            if(!viewport_calib.isCamera() && (rows > columns)){
                // Correct for row compression
                temp.y = temp.y * 0.5;

                // Correct for row shifting
                if((yRow % 2) == 1) temp.x = temp.x + 0.5;
            }

            original_rays.at<cv::Point2d>(iRay) = temp;
            iRay++;
        }
    }

    // Undistort the optical rays
    this->debug_.Msg("Undistorting optical rays...");
    cv::undistortPoints(original_rays, undistorted_rays, viewport_intrinsic, viewport_distortion);

    // Translate to 3 dimensional rays
    this->debug_.Msg("Normalizing optical rays...");
    viewport_temp.ray.create(rows,columns,CV_64FC3);
    cv::Mat viewport_rays(3, ray_count, CV_64FC1);

    dlp::Point::Cloud ray_cloud;
    for(iRay = 0; iRay < ray_count;iRay++){
        cv::Point2d ray             = undistorted_rays.at<cv::Point2d>(iRay);   // Not normalized
        double      ray_length      = sqrt(pow(ray.x,2)+pow(ray.y,2)+1.0);      // Assume unit length for z
        dlp::Point  ray_point;

        // Save the normalized ray in origin rays
        viewport_rays.at<double>(0,iRay) = ray.x / ray_length;
        viewport_rays.at<double>(1,iRay) = ray.y / ray_length;
        viewport_rays.at<double>(2,iRay) = 1.0   / ray_length;

        ray_point.x = viewport_rays.at<double>(0,iRay);
        ray_point.y = viewport_rays.at<double>(1,iRay);
        ray_point.z = viewport_rays.at<double>(2,iRay);
        ray_cloud.Add(ray_point);
    }


    // Rotate projector optical rays into the origin coordinate system.
    this->debug_.Msg("Rotating and transposing new viewport rays into origin coordinate system...");
    cv::Mat R(3, 3, CV_64FC1);

    cv::gemm(origin_rotation_3x3, viewport_rotation_3x3, 1, empty_3x3, 0, R, CV_GEMM_B_T);
    cv::gemm(R, viewport_rays, 1, empty_3x3, 0,viewport_rays, 0);



    xCol = 0;
    yRow = 0;
    for(iRay = 0; iRay < ray_count;iRay++){
        cv::Point3d ray;

        ray.x = viewport_rays.at<double>(0,iRay);
        ray.y = viewport_rays.at<double>(1,iRay);
        ray.z = viewport_rays.at<double>(2,iRay);



        // Save the normalized ray in origin rays
        viewport_temp.ray.at<cv::Point3d>(yRow,xCol) = ray;

        // Increment pixel location counters
        xCol++;

        if(xCol == columns){
            xCol = 0;
            yRow++;
        }
    }


    this->viewport_.push_back(viewport_temp);

    (*ret_viewport_id) = this->viewport_.size() - 1;


    ray_cloud.Add(dlp::Point(0,0,0));

    //ray_cloud.SaveXYZ("ray_cloud_viewport_" + dlp::Number::ToString(*ret_viewport_id) +  ".xyz",' ');

    this->debug_.Msg("Viewport " + dlp::Number::ToString((*ret_viewport_id)) + " added");

    return ret;

}

/** @brief Finds 3D line intersections using a line from the origin point and a line from the view point
*   @param[in] origin_x     Origin_x is the decoded value from the vertical structured light patterns
*   @param[in] origin_y     Origin_y is the decoded value from the horizontal structured light patterns
*   @param[in] viewport_id  Is the ID for the camera to calculate intersections from
*   @param[in] viewport_x   Viewport x is the camera column pixel location
*   @param[in] viewport_y   Viewport y is the camera row pixel location
*   @param[out] ret_xyz         Pointer to return dlp::Point
*   @retval GEOMETRY_VIEWPORT_ID_OUT_OF_RANGE   Requested viewport does NOT exist
*   @retval GEOMETRY_ORIGIN_RAY_OUT_OF_RANGE    Attempting to access origin optical ray that does NOT exist
*   @retval GEOMETRY_VIEWPORT_RAY_OUT_OF_RANGE  Attempting to access view point optical ray that does NOT exist
*   @retval GEOMETRY_NULL_POINTER               Return pointer argument is NULL
*/
ReturnCode Geometry::Find3dLineLineIntersection(const unsigned int &origin_x, const unsigned int &origin_y,
                                            const unsigned int &viewport_id,
                                            const unsigned int &viewport_x, const unsigned int &viewport_y,
                                                         Point *ret_xyz){

    ReturnCode ret;

    // Check viewport id
    if(viewport_id >= this->viewport_.size())
        return ret.AddError(GEOMETRY_VIEWPORT_ID_OUT_OF_RANGE);


    // Check  origin ray positions
    if((origin_x >= (unsigned int) this->origin_.ray.cols) ||
       (origin_y >= (unsigned int) this->origin_.ray.rows))
        return ret.AddError(GEOMETRY_ORIGIN_RAY_OUT_OF_RANGE);

    // Check  viewport ray positions
    if((viewport_x >= (unsigned int) this->viewport_.at(viewport_id).ray.cols) ||
       (viewport_y >= (unsigned int) this->viewport_.at(viewport_id).ray.rows))
        return ret.AddError(GEOMETRY_VIEWPORT_RAY_OUT_OF_RANGE);


    // Check pointers
    if(!ret_xyz)
        return ret.AddError(GEOMETRY_NULL_POINTER);

    // Find the intersection and distance
    this->Unsafe_Find3dLineLineIntersection(origin_x,    origin_y,
                                        viewport_id, viewport_x, viewport_y,
                                        ret_xyz);

    return ret;
}

/** @brief Finds 3D line intersections using a line from the origin point and a line from the view point
*   @param[in] origin_x     Origin_x is the decoded value from the vertical structured light patterns
*   @param[in] origin_y     Origin_y is the decoded value from the horizontal structured light patterns
*   @param[in] viewport_id  Is the ID for the camera to calculate intersections from
*   @param[in] viewport_x   Viewport x is the camera column pixel location
*   @param[in] viewport_y   Viewport y is the camera row pixel location
*   @param[out] ret_xyz         Pointer to return dlp::Point
*/
void Geometry::Unsafe_Find3dLineLineIntersection(const unsigned int &origin_x, const unsigned int &origin_y,
                                             const unsigned int &viewport_id,
                                             const unsigned int &viewport_x, const unsigned int &viewport_y,
                                                  dlp::Point *ret_xyz){

    // Origin offset and vector
    cv::Point3d q1 = this->origin_.center;
    cv::Point3d v1 = this->origin_.ray.at<cv::Point3d>(origin_y,origin_x);


    // Viewport offset and vector
    cv::Point3d q2 = this->viewport_.at(viewport_id).center;
    cv::Point3d v2 = this->viewport_.at(viewport_id).ray.at<cv::Point3d>(viewport_y,viewport_x);

    // Find difference between origin and viewport center
    cv::Point3d q_diff = q1 - q2;

    // Calculate dot products
    double v1_dot_v1     = v1.dot(v1);
    double v1_dot_v2     = v1.dot(v2);
    double v2_dot_v2     = v2.dot(v2);
    double q_diff_dot_v1 = q_diff.dot(v1);
    double q_diff_dot_v2 = q_diff.dot(v2);

    // Calculate scale factors
    double denom =  (v1_dot_v1*v2_dot_v2) - (v1_dot_v2*v1_dot_v2);
    double s     =  (v1_dot_v2/denom)*q_diff_dot_v2 - (v2_dot_v2/denom)*q_diff_dot_v1;
    double t     = -(v1_dot_v2/denom)*q_diff_dot_v1 + (v1_dot_v1/denom)*q_diff_dot_v2;

    // Calculate closest XYZ point between the two lines
    cv::Point3d intersection = ((q1+s*v1) + (q2+t*v2)) * 0.5;

    // Convert the cv::Point3d to dlp::PointXYZ
    ret_xyz->x = this->dir_x_ * intersection.x;
    ret_xyz->y = this->dir_y_ * intersection.y;
    ret_xyz->z = this->dir_z_ * intersection.z;

    // Calculate the distance from the viewpoint
    ret_xyz->distance = v2.dot(intersection-q2) * this->scale_;
}


ReturnCode Geometry::Find3dPlaneLineIntersection(const unsigned int &origin_plane,
                                        dlp::Pattern::Orientation orientation,
                                        const unsigned int &viewport_id,
                                        const unsigned int &viewport_x, const unsigned int &viewport_y,
                                        Point *ret_xyz){
    ReturnCode ret;

    // Check viewport id
    if(viewport_id >= this->viewport_.size())
        return ret.AddError(GEOMETRY_VIEWPORT_ID_OUT_OF_RANGE);


    // Check  origin ray positions
    if( orientation == dlp::Pattern::Orientation::HORIZONTAL){
        if(origin_plane >= (unsigned int) this->origin_.plane_rows.size())
            return ret.AddError(GEOMETRY_ORIGIN_PLANE_OUT_OF_RANGE);
    }
    else if( orientation == dlp::Pattern::Orientation::VERTICAL){
        if(origin_plane >= (unsigned int) this->origin_.plane_columns.size())
            return ret.AddError(GEOMETRY_ORIGIN_PLANE_OUT_OF_RANGE);
    }
    else if( orientation == dlp::Pattern::Orientation::DIAMOND_ANGLE_2){
        if(origin_plane >= (unsigned int) this->origin_.plane_diamond_angle_2.size())
            return ret.AddError(GEOMETRY_ORIGIN_PLANE_OUT_OF_RANGE);
    }
    else if( orientation == dlp::Pattern::Orientation::DIAMOND_ANGLE_1){
        if(origin_plane >= (unsigned int) this->origin_.plane_diamond_angle_1.size())
            return ret.AddError(GEOMETRY_ORIGIN_PLANE_OUT_OF_RANGE);
    }
    else{
        return ret.AddError(GEOMETRY_PLANE_ORIENTATION_INVALID);
    }

    // Check  viewport ray positions
    if((viewport_x >= (unsigned int) this->viewport_.at(viewport_id).ray.cols) ||
       (viewport_y >= (unsigned int) this->viewport_.at(viewport_id).ray.rows))
        return ret.AddError(GEOMETRY_VIEWPORT_RAY_OUT_OF_RANGE);


    // Check pointers
    if(!ret_xyz)
        return ret.AddError(GEOMETRY_NULL_POINTER);

    // Find the intersection and distance
    this->Unsafe_Find3dPlaneLineIntersection(origin_plane, orientation,
                                             viewport_id, viewport_x, viewport_y,
                                             ret_xyz);
    return ret;
}

void Geometry::Unsafe_Find3dPlaneLineIntersection(const unsigned int &origin_plane,
                                                  dlp::Pattern::Orientation orientation,
                                                  const unsigned int &viewport_id,
                                                  const unsigned int &viewport_x, const unsigned int &viewport_y,
                                                  Point *ret_xyz){
    // Origin offset and vector
    PlaneEquation plane_eq;

    if( orientation == dlp::Pattern::Orientation::HORIZONTAL){
        plane_eq = this->origin_.plane_rows.at(origin_plane);
    }
    else if( orientation == dlp::Pattern::Orientation::VERTICAL){

        plane_eq = this->origin_.plane_columns.at(origin_plane);
    }
    else if( orientation == dlp::Pattern::Orientation::DIAMOND_ANGLE_1){

        plane_eq = this->origin_.plane_diamond_angle_1.at(origin_plane);
    }
    else if( orientation == dlp::Pattern::Orientation::DIAMOND_ANGLE_2){

        plane_eq = this->origin_.plane_diamond_angle_2.at(origin_plane);
    }

    // Viewport offset and vector
    cv::Point3d q = this->viewport_.at(viewport_id).center;
    cv::Point3d v = this->viewport_.at(viewport_id).ray.at<cv::Point3d>(viewport_y,viewport_x);

    // Calculate dot products
    double n_dot_q     = plane_eq.w.dot(q);
    double n_dot_v     = plane_eq.w.dot(v);

    // Calculate distance from viewpoint
    ret_xyz->distance = (plane_eq.d - n_dot_q)/n_dot_v;

    // Calculate closest XYZ point
    ret_xyz->x = q.x + ret_xyz->distance*v.x;
    ret_xyz->y = q.y + ret_xyz->distance*v.y;
    ret_xyz->z = q.z + ret_xyz->distance*v.z;

    ret_xyz->distance = ret_xyz->distance * this->scale_;

    ret_xyz->x = ret_xyz->x * this->dir_x_;
    ret_xyz->y = ret_xyz->y * this->dir_y_;
    ret_xyz->z = ret_xyz->z * this->dir_z_;

}



bool Geometry::ConvertAngledToXY(const int &angled_positive, const int &angled_negative,
                                       int *x, int *y){
    // Angled negative starts from column 0
    // Angled positive starts from last column

    // Check pointers
    if((!x)||(!y))  return false;

    int angled_positive_converted = angled_positive - (this->origin_.ray.rows/2);

    int angled_difference =  abs(angled_positive_converted-angled_negative);
    float x_temp = angled_negative - ((float)angled_difference/2);
    float y_temp = (this->origin_.ray.rows - 1) - ((float)angled_difference);

    (*x) = (int) x_temp;
    (*y) = (int) y_temp;

    if( (*x) < 0 ) return false;
    if( (*y) < 0 ) return false;
    if( (*x) >= this->origin_.ray.cols ) return false;
    if( (*y) >= this->origin_.ray.rows ) return false;

    return true;
}


/** @brief Generates \ref dlp::Point::Cloud by finding the line intersections
 *         between the requested view port rays and the origin rays
 *  @param[in]  viewport_id         \ref dlp::Geometry::ViewPoint ID to select correct optical rays
 *  @param[in]  disparity_1         \ref dlp::DisparityMap generated from dlp::StructuredLight module set for \ref dlp::Pattern::Orientation::VERTICAL
 *  @param[in]  disparity_2         \ref dlp::DisparityMap generated from dlp::StructuredLight module set for \ref dlp::Pattern::Orientation::HORIZONTAL
 *  @param[out] ret_cloud           Pointer to return \ref dlp::Point::Cloud
 *  @param[out] ret_distancemap     Pointer to return \ref dlp::Image depth map
 *  @retval GEOMETRY_VIEWPORT_ID_OUT_OF_RANGE   Requested view port does NOT exist
 *  @retval GEOMETRY_NULL_POINTER               Return argument is NULL
 */
ReturnCode Geometry::GeneratePointCloud(const unsigned int &viewport_id,
                                        dlp::DisparityMap  &disparity_1,
                                        dlp::DisparityMap  &disparity_2,
                                        dlp::Point::Cloud  *ret_cloud,
                                        dlp::Image         *ret_distancemap){
    ReturnCode ret;

    // Check viewport id
    if(viewport_id >= this->viewport_.size())
        return ret.AddError(GEOMETRY_VIEWPORT_ID_OUT_OF_RANGE);

    // Check pointers
    if(!ret_cloud)
        return ret.AddError(GEOMETRY_NULL_POINTER);

    if(!ret_distancemap)
        return ret.AddError(GEOMETRY_NULL_POINTER);

    // Check  viewport resolution with the disparity maps
    unsigned int disparity_image_columns;
    unsigned int disparity_image_rows;

    disparity_1.GetColumns(&disparity_image_columns);
    disparity_1.GetRows(&disparity_image_rows);

    if((disparity_image_columns != (unsigned int) this->viewport_.at(viewport_id).ray.cols) ||
       (disparity_image_rows    != (unsigned int) this->viewport_.at(viewport_id).ray.rows))
        return ret.AddError(GEOMETRY_DISPARITY_MAP_RESOLUTION_INVALID);

    // Check  viewport resolution with row disparity
    disparity_2.GetColumns(&disparity_image_columns);
    disparity_2.GetRows(&disparity_image_rows);

    if((disparity_image_columns != (unsigned int) this->viewport_.at(viewport_id).ray.cols) ||
       (disparity_image_rows    != (unsigned int) this->viewport_.at(viewport_id).ray.rows))
        return ret.AddError(GEOMETRY_DISPARITY_MAP_RESOLUTION_INVALID);

    // Check that disparity maps are the correct pattern orientation
    dlp::Pattern::Orientation disparity_orientation_1;
    dlp::Pattern::Orientation disparity_orientation_2;

    disparity_1.GetOrientation(&disparity_orientation_1);
    disparity_2.GetOrientation(&disparity_orientation_2);

    // Check that the disparity maps are not equal
    if(disparity_orientation_1 == disparity_orientation_2)
        return ret.AddError(GEOMETRY_DISPARITY_MAPS_SAME_ORIENTATION);

    // Check that the disparity map pair are either vertical and horizontal
    // or angled positive and angled negative and setup several variables
    // The correct pair is found
    int *ptr_disparity_value_column;
    int *ptr_disparity_value_row;
    int *ptr_disparity_value_angled_negative;
    int *ptr_disparity_value_angled_positive;
    int disparity_value_1 = 0;
    int disparity_value_2 = 0;
    int disparity_value_angled_column = 0;
    int disparity_value_angled_row = 0;
    bool orientations_correct = false;
    bool convert_angled = false;
    unsigned int orientation_1_sampling = 0;
    unsigned int orientation_2_sampling = 0;
    int orientation_1_disparity_max = 0;
    int orientation_2_disparity_max = 0;

    if( (disparity_orientation_1 == dlp::Pattern::Orientation::VERTICAL) &&
        (disparity_orientation_2 == dlp::Pattern::Orientation::HORIZONTAL)){
        convert_angled = false;
        orientations_correct = true;

        ptr_disparity_value_column = &disparity_value_1;
        ptr_disparity_value_row    = &disparity_value_2;

        orientation_1_sampling = this->oversample_columns_.Get();
        orientation_2_sampling = this->oversample_rows_.Get();

        orientation_1_disparity_max = this->origin_.ray.cols;
        orientation_2_disparity_max = this->origin_.ray.rows;
    }

    if( (disparity_orientation_1 == dlp::Pattern::Orientation::HORIZONTAL) &&
        (disparity_orientation_2 == dlp::Pattern::Orientation::VERTICAL)){
        convert_angled = false;
        orientations_correct = true;

        ptr_disparity_value_column = &disparity_value_2;
        ptr_disparity_value_row    = &disparity_value_1;

        orientation_1_sampling = this->oversample_rows_.Get();
        orientation_2_sampling = this->oversample_columns_.Get();

        orientation_1_disparity_max = this->origin_.ray.rows;
        orientation_2_disparity_max = this->origin_.ray.cols;
    }

    if( (disparity_orientation_1 == dlp::Pattern::Orientation::DIAMOND_ANGLE_2) &&
        (disparity_orientation_2 == dlp::Pattern::Orientation::DIAMOND_ANGLE_1)){
        convert_angled = true;
        orientations_correct = true;

        ptr_disparity_value_column = &disparity_value_angled_column;
        ptr_disparity_value_row    = &disparity_value_angled_row;
        ptr_disparity_value_angled_negative = &disparity_value_1;
        ptr_disparity_value_angled_positive = &disparity_value_2;

        orientation_1_sampling = this->oversample_angled_negative_.Get();
        orientation_2_sampling = this->oversample_angled_positive_.Get();

        orientation_1_disparity_max = this->origin_.ray.cols + this->origin_.ray.rows - 1;
        orientation_2_disparity_max = this->origin_.ray.cols + this->origin_.ray.rows - 1;
    }

    if( (disparity_orientation_1 == dlp::Pattern::Orientation::DIAMOND_ANGLE_1) &&
        (disparity_orientation_2 == dlp::Pattern::Orientation::DIAMOND_ANGLE_2)){
        convert_angled = true;
        orientations_correct = true;

        ptr_disparity_value_column = &disparity_value_angled_column;
        ptr_disparity_value_row    = &disparity_value_angled_row;
        ptr_disparity_value_angled_negative = &disparity_value_2;
        ptr_disparity_value_angled_positive = &disparity_value_1;

        orientation_1_sampling = this->oversample_angled_positive_.Get();
        orientation_2_sampling = this->oversample_angled_negative_.Get();

        orientation_1_disparity_max = this->origin_.ray.cols + this->origin_.ray.rows - 1;
        orientation_2_disparity_max = this->origin_.ray.cols + this->origin_.ray.rows - 1;
    }

    if(!orientations_correct)
        return ret.AddError(GEOMETRY_DISPARITY_MAPS_MISMATCHED);


    // Get the disparity map oversampling
    dlp::DisparityMap disparity_1_copy(disparity_1);
    dlp::DisparityMap disparity_2_copy(disparity_2);

    unsigned int disparity_1_sampling;
    unsigned int disparity_2_sampling;

    disparity_1_copy.GetDisparitySampling(&disparity_1_sampling);
    disparity_2_copy.GetDisparitySampling(&disparity_2_sampling);

    // Convert disparity_1_copy values to match the specified oversample value
    if(orientation_1_sampling != disparity_1_sampling){

        // Resample the values to match the geometry module's settings
        for(    unsigned int yRow = 0; yRow < disparity_image_rows;    yRow++){
            for(unsigned int xCol = 0; xCol < disparity_image_columns; xCol++){
                int disparity_value = DisparityMap::EMPTY_PIXEL;

                // Get the disparity values
                disparity_1_copy.Unsafe_GetPixel(xCol, yRow, &disparity_value);

                // Generate new value
                disparity_value = (disparity_value * orientation_1_sampling) / disparity_1_sampling;

                // Save the new value
                disparity_1_copy.Unsafe_SetPixel(xCol, yRow, disparity_value);
            }
        }
    }


    // Convert disparity_2_copy values to match the specified oversample value
    if(orientation_2_sampling != disparity_2_sampling){

        // Resample the values to match the geometry module's settings
        for(    unsigned int yRow = 0; yRow < disparity_image_rows;    yRow++){
            for(unsigned int xCol = 0; xCol < disparity_image_columns; xCol++){
                int disparity_value = DisparityMap::EMPTY_PIXEL;

                // Get the disparity values
                disparity_2_copy.Unsafe_GetPixel(xCol, yRow, &disparity_value);

                // Generate new value
                disparity_value = (disparity_value * orientation_2_sampling) / disparity_2_sampling;

                // Save the new value
                disparity_2_copy.Unsafe_SetPixel(xCol, yRow, disparity_value);
            }
        }
    }

    // Check if image should be smoothed
    if(this->smooth_disparity_.Get()){
        cv::Mat map_1;
        cv::Mat map_1_copy;
        cv::Mat map_1_smooth;

        cv::Mat map_2;
        cv::Mat map_2_copy;
        cv::Mat map_2_smooth;

        disparity_1_copy.Unsafe_GetOpenCVData(&map_1);
        disparity_2_copy.Unsafe_GetOpenCVData(&map_2);

        // Clone the original data
        map_1_copy = map_1.clone();
        map_2_copy = map_2.clone();

        map_1_copy.convertTo(map_1_copy,CV_32F);
        map_2_copy.convertTo(map_2_copy,CV_32F);

        cv::bilateralFilter( map_1_copy, map_1_smooth, orientation_1_sampling*2, orientation_1_sampling*3*2, orientation_1_sampling*3/2);
        cv::bilateralFilter( map_2_copy, map_2_smooth, orientation_2_sampling*2, orientation_2_sampling*3*2, orientation_2_sampling*3/2);

        map_1_smooth.convertTo(map_1,CV_32S);
        map_2_smooth.convertTo(map_2,CV_32S);
    }

    // Allocate memory for distance map
    ret_distancemap->Clear();
    ret_distancemap->Create(disparity_image_columns,
                            disparity_image_rows,
                            Image::Format::MONO_DOUBLE);
    ret_distancemap->FillImage((double)dlp::DisparityMap::EMPTY_PIXEL);

    // Create point map to track all points to specfic origin rays
    // rows     // columns  // points
    std::vector<std::vector<std::vector<dlp::Point> > > point_cloud;
    std::vector<std::vector<std::vector<  double  > > > point_cloud_distance;

    // Check if viewpoint rays should be filtered
    if(this->filter_rays_enable_.Get()){
        // Size the vectors to the origin dimensions
        point_cloud.resize(this->origin_.ray.rows);
        point_cloud_distance.resize(this->origin_.ray.rows);
        for( int yRow = 0; yRow < this->origin_.ray.rows; yRow++){
            point_cloud[yRow].resize(this->origin_.ray.cols);
            point_cloud_distance[yRow].resize(this->origin_.ray.cols);
        }
    }

    // Generate the point cloud
    ret_cloud->Clear();
    double max_origin_distance = this->max_distance_.Get();
    double min_origin_distance = this->min_distance_.Get();
    bool   check_distance      = (max_origin_distance != min_origin_distance);
    bool valid_disparity_value = false;
    for(    unsigned int yRow = 0; yRow < disparity_image_rows;    yRow++){
        for(unsigned int xCol = 0; xCol < disparity_image_columns; xCol++){
            valid_disparity_value = false;
            disparity_value_1 = DisparityMap::EMPTY_PIXEL;
            disparity_value_2 = DisparityMap::EMPTY_PIXEL;

            // Get the disparity values
            disparity_1_copy.Unsafe_GetPixel( xCol, yRow, &disparity_value_1);
            disparity_2_copy.Unsafe_GetPixel( xCol, yRow, &disparity_value_2);

            // If using angled planes convert to column and row
            if(convert_angled){
                // Convert values and check that the disparity values are valid
                valid_disparity_value = this->ConvertAngledToXY( (*ptr_disparity_value_angled_positive),
                                                                 (*ptr_disparity_value_angled_negative),
                                                                  &disparity_value_angled_column,
                                                                  &disparity_value_angled_row);
            }
            else{
                // Vertical and horizontal orientations are used so only check their
                // validity and do NOT convert
                if((disparity_value_2 <  orientation_2_disparity_max) &&
                   (disparity_value_1 <  orientation_1_disparity_max) &&
                   (disparity_value_2 >= 0) &&
                   (disparity_value_1 >= 0)){
                    valid_disparity_value = true;
                }
                else{
                    valid_disparity_value = false;
                }

            }

            // Calculate the point in space if disparity values are valid
            if(valid_disparity_value){
                dlp::Point point;

                this->Unsafe_Find3dLineLineIntersection((unsigned int) (*ptr_disparity_value_column),
                                                        (unsigned int) (*ptr_disparity_value_row),       // Origin ray
                                                        viewport_id, xCol, yRow, // Viewport ray
                                                        &point);                 // Return point and distance from origin

                // Check that z is greater than zero
                if(point.z > 0){

                    // If the distance is within the set min and max origin distances
                    // or no distance check is needed add the point to the cloud
                    if( (!check_distance) ||
                            ((point.distance <= max_origin_distance) &&
                             (point.distance >= min_origin_distance))){

                        // Save the distance
                        ret_distancemap->Unsafe_SetPixel(xCol,yRow,point.distance);

                        // Check if viewpoint rays should be filtered
                        if(this->filter_rays_enable_.Get()){
                            // Save the point and origin distance
                            point_cloud[(*ptr_disparity_value_row)][(*ptr_disparity_value_column)].push_back(point);
                            point_cloud_distance[(*ptr_disparity_value_row)][(*ptr_disparity_value_column)].push_back(point.distance);
                        }
                        else{
                            // Add the point
                            ret_cloud->Add(point);
                        }
                    }

                }
            }
        }
    }

    // If viewpoint rays should be filtered reprocess the cloud
    if(this->filter_rays_enable_.Get()){

        this->debug_.Msg("Filtering cloud...");

        // Claculate maximum error
        double max_error = this->filter_rays_max_error_.Get() / 100;

        for(     int yRow = 0; yRow < this->origin_.ray.rows; yRow++){
            for( int xCol = 0; xCol < this->origin_.ray.cols; xCol++){

                // Check that there are enough points
                if(point_cloud[yRow][xCol].size() > 0){

                    // Calculate average
                    double sum     = 0;
                    double average = 0;
                    for(unsigned int iPoint = 0; iPoint < point_cloud[yRow][xCol].size(); iPoint++){
                        sum = sum + point_cloud_distance[yRow][xCol][iPoint];
                    }
                    average = sum / (double)point_cloud[yRow][xCol].size();

                    // Calculate the error for each point and keep points within threshold
                    std::vector<dlp::Point> valid_points;
                    for(unsigned int iPoint = 0; iPoint < point_cloud[yRow][xCol].size(); iPoint++){
                        double percent_error = std::abs(average - point_cloud_distance[yRow][xCol][iPoint])
                                               / point_cloud_distance[yRow][xCol][iPoint];

                        // If the error is below threshold save point
                        if(percent_error <= max_error){
                            valid_points.push_back(point_cloud[yRow][xCol][iPoint]);
                        }
                    }

                    // Calculate the average point location if more than zero points
                    if(valid_points.size() > 0){
                        dlp::Point valid_point;
                        for(unsigned int iPoint = 0; iPoint < valid_points.size(); iPoint++){
                            valid_point.x += valid_points[iPoint].x;
                            valid_point.y += valid_points[iPoint].y;
                            valid_point.z += valid_points[iPoint].z;
                        }
                        valid_point.x = valid_point.x / valid_points.size();
                        valid_point.y = valid_point.y / valid_points.size();
                        valid_point.z = valid_point.z / valid_points.size();

                        // Save the point
                        ret_cloud->Add(valid_point);
                    }
                }
            }
        }
    }

    return ret;
}


ReturnCode Geometry::GeneratePointCloud(const unsigned int &viewport_id,
                                        dlp::DisparityMap &disparity_map,
                                        dlp::Point::Cloud *ret_cloud,
                                        Image *ret_distancemap){
    ReturnCode ret;
    dlp::DisparityMap disparity_map_copy(disparity_map);

    // Check viewport id
    if(viewport_id >= this->viewport_.size())
        return ret.AddError(GEOMETRY_VIEWPORT_ID_OUT_OF_RANGE);

    // Check pointers
    if(!ret_cloud)
        return ret.AddError(GEOMETRY_NULL_POINTER);

    if(!ret_distancemap)
        return ret.AddError(GEOMETRY_NULL_POINTER);

    // Check that disparity maps are the correct pattern orientation
    dlp::Pattern::Orientation disparity_orientation;
    disparity_map_copy.GetOrientation(&disparity_orientation);
    if((disparity_orientation != dlp::Pattern::Orientation::VERTICAL) &&
       (disparity_orientation != dlp::Pattern::Orientation::HORIZONTAL) &&
       (disparity_orientation != dlp::Pattern::Orientation::DIAMOND_ANGLE_2) &&
       (disparity_orientation != dlp::Pattern::Orientation::DIAMOND_ANGLE_1))
        return ret.AddError(GEOMETRY_DISPARITY_MAP_ORIENTATION_INVALID);

    // Get the disparity map oversampling
    unsigned int disparity_sampling;
    unsigned int geometry_sampling;
    disparity_map_copy.GetDisparitySampling(&disparity_sampling);

    // Get the maximum disparity value
    unsigned int disparity_max = this->origin_.plane_columns.size() - 1;

    if( disparity_orientation == dlp::Pattern::Orientation::HORIZONTAL){
        disparity_max       = this->origin_.plane_rows.size();
        geometry_sampling   = this->oversample_rows_.Get();
    }
    else if( disparity_orientation == dlp::Pattern::Orientation::VERTICAL){
        disparity_max       = this->origin_.plane_columns.size();
        geometry_sampling   = this->oversample_columns_.Get();
    }
    else if( disparity_orientation == dlp::Pattern::Orientation::DIAMOND_ANGLE_1){
        disparity_max       = this->origin_.plane_diamond_angle_1.size();
        geometry_sampling   = this->oversample_angled_positive_.Get();
    }
    else if( disparity_orientation == dlp::Pattern::Orientation::DIAMOND_ANGLE_2){
        disparity_max       = this->origin_.plane_diamond_angle_2.size();
        geometry_sampling   = this->oversample_angled_negative_.Get();
    }


    // Check  viewport resolution with column disparity
    unsigned int disparity_image_columns;
    unsigned int disparity_image_rows;

    disparity_map_copy.GetColumns(&disparity_image_columns);
    disparity_map_copy.GetRows(&disparity_image_rows);

    if((disparity_image_columns != (unsigned int) this->viewport_.at(viewport_id).ray.cols) ||
       (disparity_image_rows    != (unsigned int) this->viewport_.at(viewport_id).ray.rows))
        return ret.AddError(GEOMETRY_DISPARITY_MAP_RESOLUTION_INVALID);

    if(geometry_sampling != disparity_sampling){

        // Resample the values to match the geometry module's settings
        for(    unsigned int yRow = 0; yRow < disparity_image_rows;    yRow++){
            for(unsigned int xCol = 0; xCol < disparity_image_columns; xCol++){
                int disparity_value = DisparityMap::EMPTY_PIXEL;

                // Get the disparity values
                disparity_map_copy.Unsafe_GetPixel(xCol, yRow, &disparity_value);

                // Generate new value
                disparity_value = (disparity_value * geometry_sampling) / disparity_sampling;

                // Save the new value
                disparity_map_copy.Unsafe_SetPixel(xCol, yRow, disparity_value);
            }
        }
    }

    // Check if image should be smoothed
    if(this->smooth_disparity_.Get()){
        cv::Mat original;
        cv::Mat map;
        cv::Mat smooth;
        disparity_map_copy.Unsafe_GetOpenCVData(&map);

        // Clone the original data
        original = map.clone();
        original.convertTo(original,CV_32F);
        cv::bilateralFilter ( original, smooth, geometry_sampling*2, geometry_sampling*3*2, geometry_sampling*3/2);

        smooth.convertTo(map,CV_32S);
    }


    // Allocate memory for the distance map
    ret_distancemap->Clear();
    ret_distancemap->Create(disparity_image_columns,
                            disparity_image_rows,
                            Image::Format::MONO_DOUBLE);
    ret_distancemap->FillImage((double)dlp::DisparityMap::EMPTY_PIXEL);


    // Clear the point cloud
    ret_cloud->Clear();
    double max_origin_distance = this->max_distance_.Get();
    double min_origin_distance = this->min_distance_.Get();
    bool   check_distance      = (max_origin_distance != min_origin_distance);

    for(    unsigned int yRow = 0; yRow < disparity_image_rows;    yRow++){
        for(unsigned int xCol = 0; xCol < disparity_image_columns; xCol++){
            int disparity_value = DisparityMap::EMPTY_PIXEL;

            // Get the disparity values
            disparity_map_copy.Unsafe_GetPixel(xCol, yRow, &disparity_value);

            // Check that neither pixels are invalid
            if((disparity_value < (int) disparity_max) &&
               (disparity_value >= 0)){

                // Calculate the point in space
                dlp::Point point;

                this->Unsafe_Find3dPlaneLineIntersection((unsigned int) disparity_value,
                                                                        disparity_orientation,
                                                    viewport_id, xCol, yRow,                        // Viewport ray
                                                    &point);                             // Return point and distance from origin


                // Check that z is greater than zero
                if(point.z > 0){

                    // If the distance is within the set min and max origin distances
                    // or no distance check is needed add the point to the cloud
                    if( (!check_distance) ||
                       ((point.distance <= max_origin_distance) &&
                        (point.distance >= min_origin_distance))){

                        // Save the distance
                        ret_distancemap->Unsafe_SetPixel(xCol,yRow,point.distance);

                        // Save the point
                        ret_cloud->Add(point);
                    }
                }
            }
        }
    }

    return ret;
}

ReturnCode Geometry::ConvertDistanceMapToColor(const dlp::Image &distance_map, dlp::Image *color_depth){
    ReturnCode ret;

    if(distance_map.isEmpty())
        return ret.AddError(IMAGE_EMPTY);

    dlp::Image::Format distance_format;
    distance_map.GetDataFormat(&distance_format);

    if(distance_format != dlp::Image::Format::MONO_DOUBLE)
        return ret.AddError(IMAGE_STORED_IN_DIFFERENT_FORMAT);

    // Check pointer
    if(!color_depth)
        return ret.AddError(GEOMETRY_NULL_POINTER);



    // Allocate memory for the distance map
    double min_distance = 0;
    double max_distance = 0;


    // Create the depth map based
    unsigned int rows;
    unsigned int columns;

    distance_map.GetColumns(&columns);
    distance_map.GetRows(&rows);

    // Find the min and max distances
    for(    unsigned int xCol = 0; xCol < columns; xCol++){
        for(unsigned int yRow = 0; yRow < rows; yRow++){
            double distance;

            // Get the distamce value
            distance_map.Unsafe_GetPixel( xCol, yRow, &distance);

            if(distance != (double)dlp::DisparityMap::EMPTY_PIXEL){

                // Check if the min and max distances have not been set before
                if(max_distance == 0 && min_distance == 0){
                    max_distance = distance;
                    min_distance = distance;
                }

                // Check the point distance for the depth map
                if(distance < min_distance) min_distance = distance;
                if(distance > max_distance) max_distance = distance;
            }
        }
    }

    // Allocate memory for the color depth image
    color_depth->Clear();
    color_depth->Create(columns,rows,Image::Format::RGB_UCHAR);
    color_depth->FillImage(PixelRGB(0,0,0));

    // Convert distance to color
    for(    unsigned int xCol = 0; xCol < columns; xCol++){
        for(unsigned int yRow = 0; yRow < rows; yRow++){
            double          distance;
            PixelRGB        depth;
            unsigned char   depth_temp;

            // Get the distamce value
            distance_map.Unsafe_GetPixel( xCol, yRow, &distance);

            // Check that distance is greater than zero
            if(distance != (double)dlp::DisparityMap::EMPTY_PIXEL){

                // Scale the distance point so that it is a value between 0 and 255
                depth_temp = (unsigned char) ((max_distance - distance) * 255 / (max_distance - min_distance));

                // Determine the color
                if(depth_temp < 43){
                    depth.r = depth_temp * 6;
                    depth.g = 0;
                    depth.b = depth_temp * 6;
                }
                if(depth_temp > 42 && depth_temp < 85){
                    depth.r = 255 - (depth_temp - 43) * 6;
                    depth.g = 0;
                    depth.b = 255;
                }
                if(depth_temp > 84 && depth_temp < 128){
                    depth.r = 0;
                    depth.g = (depth_temp - 85) * 6;
                    depth.b = 255;
                }
                if(depth_temp > 127 && depth_temp < 169){
                    depth.r = 0;
                    depth.g = 255;
                    depth.b = 255 - (depth_temp - 128) * 6;
                }
                if(depth_temp > 168 && depth_temp < 212){
                    depth.r = (depth_temp - 169) * 6;
                    depth.g = 255;
                    depth.b = 0;
                }
                if(depth_temp > 211 && depth_temp < 254){
                    depth.r = 255;
                    depth.g = 255 - (depth_temp - 212) * 6;
                    depth.b = 0;
                }
                if(depth_temp > 253){
                    depth.r = 255;
                    depth.g = 0;
                    depth.b = 0;
                }

                // Save the value
                color_depth->Unsafe_SetPixel(xCol,yRow,depth);
            }
        }
    }

    return ret;
}


ReturnCode Geometry::CalculateFlatness(const dlp::Point::Cloud &cloud, double *flatness){
    ReturnCode ret;

    // Check pointer
    if(!flatness)
        return ret.AddError(GEOMETRY_NULL_POINTER);

    // Check that point cloud is NOT empty
    if(cloud.GetCount() == 0)
        return ret.AddError(GEOMETRY_POINT_CLOUD_EMPTY);

    // Convert point cloud to cv::Mat
    cv::Mat cloud_matrix(cloud.GetCount(),3,CV_64FC1);
    for(unsigned long long iPoint = 0; iPoint < cloud.GetCount(); iPoint++){
        dlp::Point temp;
        cloud.Get(iPoint,&temp);
        cloud_matrix.at<double>(iPoint,0) = temp.x;
        cloud_matrix.at<double>(iPoint,1) = temp.y;
        cloud_matrix.at<double>(iPoint,2) = temp.z;
    }

    // Calculate the covariance
    cv::Mat covariance;
    cv::Mat centroid;
    cv::calcCovarMatrix(cloud_matrix, covariance, centroid, CV_COVAR_NORMAL | CV_COVAR_ROWS);

    cv::Mat W;
    cv::Mat V;
    cv::Mat U;


    cv::SVD::compute(covariance, W, U, V, CV_SVD_V_T);

    dlp::Geometry::PlaneEquation plane_eq;

    plane_eq.w.x = V.at<double>(2,0);
    plane_eq.w.y = V.at<double>(2,1);
    plane_eq.w.z = V.at<double>(2,2);


    plane_eq.d  = 0.0;
    plane_eq.d += plane_eq.w.x * centroid.at<double>(0,1);
    plane_eq.d += plane_eq.w.y * centroid.at<double>(0,1);
    plane_eq.d += plane_eq.w.z * centroid.at<double>(0,1);

    // Release memory
    covariance.release();
    centroid.release();
    W.release();
    V.release();
    U.release();

    // Calculate the eigen values and vectors
    cv::Mat eigen_values;
    cv::Mat eigen_vectors;
    cv::eigen(covariance,eigen_values,eigen_vectors);

    // Return the flatness
    (*flatness) = eigen_values.at<double>(0,2);

    return ret;
}


dlp::Geometry::PlaneEquation Geometry::FitPlane( const cv::Mat &points){
    dlp::Geometry::PlaneEquation plane_eq;
    plane_eq.w.x = 0;
    plane_eq.w.y = 0;
    plane_eq.w.z = 0;
    plane_eq.d   = 0;


    // Check that point cloud is NOT empty
    if((points.rows > 3) && (points.cols == 3)){

        // Calculate the covariance
        cv::Mat covariance;
        cv::Mat centroid;
        cv::calcCovarMatrix(points, covariance, centroid, CV_COVAR_NORMAL | CV_COVAR_ROWS);

        cv::Mat W;
        cv::Mat V;
        cv::Mat U;


        cv::SVD::compute(covariance, W, U, V, CV_SVD_V_T);

        plane_eq.w.x = V.at<double>(2,0);
        plane_eq.w.y = V.at<double>(2,1);
        plane_eq.w.z = V.at<double>(2,2);

        plane_eq.d  = 0.0;
        plane_eq.d += plane_eq.w.x * centroid.at<double>(0,1);
        plane_eq.d += plane_eq.w.y * centroid.at<double>(0,1);
        plane_eq.d += plane_eq.w.z * centroid.at<double>(0,1);

        // Release memory
        covariance.release();
        centroid.release();
        W.release();
        V.release();
        U.release();
    }

    return plane_eq;
}

namespace Number{
template <> std::string ToString<dlp::Geometry::PositiveDirectionX>( dlp::Geometry::PositiveDirectionX direction ){
    switch(direction){
    case dlp::Geometry::PositiveDirectionX::LEFT:       return "LEFT";
    case dlp::Geometry::PositiveDirectionX::RIGHT:      return "RIGHT";
    case dlp::Geometry::PositiveDirectionX::INVALID:    return "INVALID";
    }
    return "INVALID";
}

template <> std::string ToString<dlp::Geometry::PositiveDirectionY>( dlp::Geometry::PositiveDirectionY direction ){
    switch(direction){
    case dlp::Geometry::PositiveDirectionY::UP:         return "UP";
    case dlp::Geometry::PositiveDirectionY::DOWN:       return "DOWN";
    case dlp::Geometry::PositiveDirectionY::INVALID:    return "INVALID";
    }
    return "INVALID";
}

}

namespace String{
template <> dlp::Geometry::PositiveDirectionX ToNumber( const std::string &text, unsigned int base ){
    // Ignore base variable
    if (text.compare("LEFT") == 0){
        return dlp::Geometry::PositiveDirectionX::LEFT;
    }
    else if (text.compare("RIGHT") == 0){
        return dlp::Geometry::PositiveDirectionX::RIGHT;
    }
    else{
        return dlp::Geometry::PositiveDirectionX::INVALID;
    }
}

template <> dlp::Geometry::PositiveDirectionY ToNumber( const std::string &text, unsigned int base ){
    // Ignore base variable
    if (text.compare("UP") == 0){
        return dlp::Geometry::PositiveDirectionY::UP;
    }
    else if (text.compare("DOWN") == 0){
        return dlp::Geometry::PositiveDirectionY::DOWN;
    }
    else{
        return dlp::Geometry::PositiveDirectionY::INVALID;
    }
}


}

}
