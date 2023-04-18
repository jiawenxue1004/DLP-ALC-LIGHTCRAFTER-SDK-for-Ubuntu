/** @file   calibration.hpp
 *  @brief  Contains definitions for calibrating cameras and projectors
 *  @copyright 2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 *
 *  The calibration.hpp file defines the camera and projector calibration
 *  models for use in 3D metrology applications using DLP technology. The
 *  following objects are defined:
 *
 *  - dlp::Calibration::Data
 *  - dlp::Calibration::Camera
 *  - dlp::Calibration::Projector
 *
 */

#ifndef DLP_SDK_CALIBRATION_HPP
#define DLP_SDK_CALIBRATION_HPP

// DLP Structured Light SDK header files
#include <common/debug.hpp>                     // Adds dlp::Debug
#include <common/module.hpp>                    // Adds dlp::Module
#include <common/returncode.hpp>                // Adds dlp::ReturnCode
#include <common/image/image.hpp>               // Adds dlp::Image
#include <common/parameters.hpp>                // Adds dlp::Parameter
#include <camera/camera.hpp>                    // Adds dlp::Camera
#include <dlp_platforms/dlp_platform.hpp>       // Adds dlp::DLP_Platform

// OpenCV header files
#include <opencv2/opencv.hpp>                   // Adds OpenCV image container

// C++ standard header files
#include <vector>                               // Adds std::vector
#include <string>                               // Adds std::string

// Shortcut cv::Mat definitions for calibration matrices
// when using cv::Mat::create()  rows, columns, data format
#define DLP_CV_INTRINSIC_SETUP      3, 3, CV_64FC1
#define DLP_CV_EXTRINSIC_SETUP      2, 3, CV_64FC1
#define DLP_CV_DISTORTION_SETUP     5, 1, CV_64FC1
#define DLP_CV_HOMOGRAPHY_SETUP     3, 3, CV_64FC1


// DLP Structured Light error messages for dlp::ReturnCodes
#define CALIBRATION_DATA_NULL_POINTER_INTRINSIC             "CALIBRATION_DATA_NULL_POINTER_INTRINSIC"
#define CALIBRATION_DATA_NULL_POINTER_EXTRINSIC             "CALIBRATION_DATA_NULL_POINTER_EXTRINSIC"
#define CALIBRATION_DATA_NULL_POINTER_DISTORTION            "CALIBRATION_DATA_NULL_POINTER_DISTORTION"
#define CALIBRATION_DATA_NULL_POINTER_REPROJECTION_ERROR    "CALIBRATION_DATA_NULL_POINTER_REPROJECTION_ERROR"

#define CALIBRATION_DATA_NULL_POINTER_COLUMNS               "CALIBRATION_DATA_NULL_POINTER_COLUMNS"
#define CALIBRATION_DATA_NULL_POINTER_ROWS                  "CALIBRATION_DATA_NULL_POINTER_ROWS"
#define CALIBRATION_DATA_NOT_COMPLETE                       "CALIBRATION_DATA_NOT_COMPLETE"

#define CALIBRATION_DATA_FILE_EXTENSION_INVALID             "CALIBRATION_DATA_FILE_EXTENSION_INVALID"
#define CALIBRATION_DATA_FILE_SAVE_FAILED                   "CALIBRATION_DATA_FILE_SAVE_FAILED"
#define CALIBRATION_DATA_FILE_LOAD_FAILED                   "CALIBRATION_DATA_FILE_LOAD_FAILED"
#define CALIBRATION_DATA_FILE_INVALID                       "CALIBRATION_DATA_FILE_INVALID"

#define CALIBRATION_NOT_SETUP                               "CALIBRATION_NOT_SETUP"
#define CALIBRATION_NOT_COMPLETE                            "CALIBRATION_NOT_COMPLETE"
#define CALIBRATION_NOT_FROM_CAMERA                         "CALIBRATION_NOT_FROM_CAMERA"
#define CALIBRATION_NULL_POINTER_SETTINGS                   "CALIBRATION_NULL_POINTER_SETTINGS"
#define CALIBRATION_NULL_POINTER_SUCCESS                    "CALIBRATION_NULL_POINTER_SUCCESS"
#define CALIBRATION_NULL_POINTER_SUCCESSFUL                 "CALIBRATION_NULL_POINTER_SUCCESSFUL"
#define CALIBRATION_NULL_POINTER_TOTAL_REQUIRED             "CALIBRATION_NULL_POINTER_TOTAL_REQUIRED"
#define CALIBRATION_NULL_POINTER_DATA                       "CALIBRATION_NULL_POINTER_DATA"
#define CALIBRATION_NULL_POINTER_CALIBRATION_IMAGE          "CALIBRATION_NULL_POINTER_CALIBRATION_IMAGE"
#define CALIBRATION_NULL_POINTER_REPROJECTION_ERROR         "CALIBRATION_NULL_POINTER_REPROJECTION_ERROR"
#define CALIBRATION_NULL_POINTER_PROJECTED_BOARD            "CALIBRATION_NULL_POINTER_PROJECTED_BOARD"

#define CALIBRATION_PARAMETERS_MODEL_SIZE_MISSING                         "CALIBRATION_PARAMETERS_MODEL_SIZE_MISSING"
#define CALIBRATION_PARAMETERS_IMAGE_SIZE_MISSING                         "CALIBRATION_PARAMETERS_IMAGE_SIZE_MISSING"

#define CALIBRATION_PARAMETERS_NUMBER_BOARDS_MISSING                      "CALIBRATION_PARAMETERS_NUMBER_BOARDS_MISSING"
#define CALIBRATION_PARAMETERS_NUMBER_BOARDS_INVALID                      "CALIBRATION_PARAMETERS_NUMBER_BOARDS_INVALID"

#define CALIBRATION_PARAMETERS_IMAGE_SIZE_MISSING                         "CALIBRATION_PARAMETERS_IMAGE_SIZE_MISSING"
#define CALIBRATION_PARAMETERS_IMAGE_SIZE_INVALID                         "CALIBRATION_PARAMETERS_IMAGE_SIZE_INVALID"
#define CALIBRATION_PARAMETERS_BOARD_TYPE_MISSING                         "CALIBRATION_PARAMETERS_BOARD_TYPE_MISSING"
#define CALIBRATION_PARAMETERS_BOARD_TYPE_INVALID                         "CALIBRATION_PARAMETERS_BOARD_TYPE_INVALID"
#define CALIBRATION_PARAMETERS_BOARD_FEATURE_SIZE_MISSING                 "CALIBRATION_PARAMETERS_BOARD_FEATURE_SIZE_MISSING"
#define CALIBRATION_PARAMETERS_BOARD_FEATURE_SIZE_INVALID                 "CALIBRATION_PARAMETERS_BOARD_FEATURE_SIZE_INVALID"
#define CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_MISSING             "CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_MISSING"
#define CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_INVALID             "CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_INVALID"
#define CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_IN_PIXELS_MISSING   "CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_IN_PIXELS_MISSING"
#define CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_IN_PIXELS_INVALID   "CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_IN_PIXELS_INVALID"
#define CALIBRATION_PARAMETERS_PATTERN_TYPE_NOT_SUPPORTED                 "CALIBRATION_PARAMETERS_PATTERN_TYPE_NOT_SUPPORTED"
#define CALIBRATION_PARAMETERS_PATTERN_SIZE_MISSING                       "CALIBRATION_PARAMETERS_PATTERN_SIZE_MISSING"
#define CALIBRATION_PARAMETERS_PATTERN_SIZE_INVALID                       "CALIBRATION_PARAMETERS_PATTERN_SIZE_INVALID"
#define CALIBRATION_PARAMETERS_PATTERN_POINT_DISTANCE_MISSING             "CALIBRATION_PARAMETERS_PATTERN_POINT_DISTANCE_MISSING"
#define CALIBRATION_PARAMETERS_PATTERN_POINT_DISTANCE_INVALID             "CALIBRATION_PARAMETERS_PATTERN_POINT_DISTANCE_INVALID"
#define CALIBRATION_PARAMETERS_PATTERN_POINT_LOCATION_OUT_OF_RANGE        "CALIBRATION_PARAMETERS_PATTERN_POINT_LOCATION_OUT_OF_RANGE"
#define CALIBRATION_PARAMETERS_PATTERN_BORDER_DISTANCE_MISSING            "CALIBRATION_PARAMETERS_PATTERN_BORDER_DISTANCE_MISSING"
#define CALIBRATION_PARAMETERS_PATTERN_BORDER_DISTANCE_INVALID            "CALIBRATION_PARAMETERS_PATTERN_BORDER_DISTANCE_INVALID"
#define CALIBRATION_PARAMETERS_TANGENT_DISTORTION_MISSING                 "CALIBRATION_PARAMETERS_TANGENT_DISTORTION_MISSING"
#define CALIBRATION_PARAMETERS_SIXTH_ORDER_DISTORTION_MISSING             "CALIBRATION_PARAMETERS_SIXTH_ORDER_DISTORTION_MISSING"
#define CALIBRATION_PARAMETERS_FIX_ASPECT_RATIO_MISSING                   "CALIBRATION_PARAMETERS_FIX_ASPECT_RATIO_MISSING"

#define CALIBRATION_IMAGE_EMPTY                     "CALIBRATION_IMAGE_EMPTY"
#define CALIBRATION_PRINTED_IMAGE_EMPTY             "CALIBRATION_PRINTED_IMAGE_EMPTY"
#define CALIBRATION_COMBO_IMAGE_EMPTY               "CALIBRATION_COMBO_IMAGE_EMPTY"
#define CALIBRATION_IMAGE_RESOLUTION_INVALID        "CALIBRATION_IMAGE_RESOLUTION_INVALID"
#define CALIBRATION_IMAGE_RESOLUTION_MISMATCH       "CALIBRATION_IMAGE_RESOLUTION_MISMATCH"
#define CALIBRATION_IMAGE_CONVERT_TO_MONO_FAILED    "CALIBRATION_IMAGE_CONVERT_TO_MONO_FAILED"
#define CALIBRATION_IMAGE_VECTOR_SIZE_MISMATCH      "CALIBRATION_IMAGE_VECTOR_SIZE_MISMATCH"

#define CALIBRATION_BOARD_NOT_DETECTED              "CALIBRATION_BOARD_NOT_DETECTED"
#define CALIBRATION_NO_BOARDS_ADDED                 "CALIBRATION_NO_BOARDS_ADDED"

#define CALIBRATION_CAMERA_CALIBRATION_MISSING                  "CALIBRATION_CAMERA_CALIBRATION_MISSING"
#define CALIBRATION_CAMERA_CALIBRATION_HOMOGRAPHIES_MISSING     "CALIBRATION_CAMERA_CALIBRATION_HOMOGRAPHIES_MISSING"


/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{

/** @class  Calibration
 *  @brief  Umbrella class for calibration data, camera, and projector routines
 *          using the OpenCV.
 *
 *  The Calibration class contains classes used for system calibration
 *  including: Camera and Projector models, Settings, Errors, and Data container.
 *
 *  @warning No instances of the Calibration class should be made. The class exists
 *           so that the private calibration data could be modified by the nested
 *           camera and projector calibration classes.
 */
class Calibration{
public:
    /** @defgroup   group_Calibration Calibration
     *  @brief      Contains classes for camera and projector calibration
     *  @{
     */

    /** @class  Parameters
     *  @brief  Contains the dlp::Parameters::Entry objects for the Setup() routines
     *          for dlp::Calibration::Camera and dlp::Calibration::Projector
     *
     */
    class Parameters{
    public:
        DLP_NEW_PARAMETERS_ENTRY(ModelRows,             "CALIBRATION_PARAMETERS_MODEL_ROWS",                unsigned int,     0);
        DLP_NEW_PARAMETERS_ENTRY(ModelColumns,          "CALIBRATION_PARAMETERS_MODEL_COLUMNS",             unsigned int,     0);
        DLP_NEW_PARAMETERS_ENTRY(ModelOffsetVertical,   "CALIBRATION_PARAMETERS_MODEL_OFFSET_VERTICAL",            float, 100.0);
        DLP_NEW_PARAMETERS_ENTRY(ModelOffsetHorizontal, "CALIBRATION_PARAMETERS_MODEL_OFFSET_HORIZONTAL",          float,   0.0);
        DLP_NEW_PARAMETERS_ENTRY(ModelPixelSize,        "CALIBRATION_PARAMETERS_MODEL_PIXEL_SIZE",                 float,   0.0);
        DLP_NEW_PARAMETERS_ENTRY(ModelFocalLength,      "CALIBRATION_PARAMETERS_MODEL_FOCAL_LENGTH",               float,   0.0);

        DLP_NEW_PARAMETERS_ENTRY(ImageRows,             "CALIBRATION_PARAMETERS_IMAGE_ROWS",        unsigned int,  0);
        DLP_NEW_PARAMETERS_ENTRY(ImageColumns,          "CALIBRATION_PARAMETERS_IMAGE_COLUMNS",     unsigned int,  0);

        DLP_NEW_PARAMETERS_ENTRY(BoardCount,                        "CALIBRATION_PARAMETERS_BOARD_COUNT",                           unsigned int,   0);
        DLP_NEW_PARAMETERS_ENTRY(BoardFeatureColumns,               "CALIBRATION_PARAMETERS_BOARD_FEATURE_COLUMNS",                 unsigned int,   0);
        DLP_NEW_PARAMETERS_ENTRY(BoardFeatureColumnDistance,        "CALIBRATION_PARAMETERS_BOARD_FEATURE_COLUMN_DISTANCE",               double, 0.0);
        DLP_NEW_PARAMETERS_ENTRY(BoardFeatureColumnDistancePixels,  "CALIBRATION_PARAMETERS_BOARD_FEATURE_COLUMN_DISTANCE_PIXELS",  unsigned int,   0);
        DLP_NEW_PARAMETERS_ENTRY(BoardFeatureColumnOffsetPixels,    "CALIBRATION_PARAMETERS_BOARD_FEATURE_COLUMN_OFFSET_PIXELS",    unsigned int,   0);
        DLP_NEW_PARAMETERS_ENTRY(BoardFeatureRows,                  "CALIBRATION_PARAMETERS_BOARD_FEATURE_ROWS",                    unsigned int,   0);
        DLP_NEW_PARAMETERS_ENTRY(BoardFeatureRowDistance,           "CALIBRATION_PARAMETERS_BOARD_FEATURE_ROW_DISTANCE",                  double, 0.0);
        DLP_NEW_PARAMETERS_ENTRY(BoardFeatureRowDistancePixels,     "CALIBRATION_PARAMETERS_BOARD_FEATURE_ROW_DISTANCE_PIXELS",     unsigned int,   0);
        DLP_NEW_PARAMETERS_ENTRY(BoardFeatureRowOffsetPixels,       "CALIBRATION_PARAMETERS_BOARD_FEATURE_ROW_OFFSET_PIXELS",       unsigned int,   0);
        DLP_NEW_PARAMETERS_ENTRY(BoardForeground,   "CALIBRATION_PARAMETERS_BOARD_FOREGROUND",  dlp::PixelRGB,  dlp::PixelRGB(255,255,255));
        DLP_NEW_PARAMETERS_ENTRY(BoardBackground,   "CALIBRATION_PARAMETERS_BOARD_BACKGROUND",  dlp::PixelRGB,  dlp::PixelRGB(  0,  0,  0));

        DLP_NEW_PARAMETERS_ENTRY(SetTangentDistZero, "CALIBRATION_PARAMETERS_SET_TANGENT_DIST_TO_ZERO", bool, false);
        DLP_NEW_PARAMETERS_ENTRY(FixSixthOrderDist,  "CALIBRATION_PARAMETERS_FIX_SIXTH_ORDER_DIST",     bool, false);
        DLP_NEW_PARAMETERS_ENTRY(FixAspectRatio,     "CALIBRATION_PARAMETERS_FIX_ASPECT_RATIO",         bool,  true);
    };

    /** @class Data
     *  @brief Container for calibration model distortion coefficients and
     *         intrinsic/extrinsic parameters.
     *
     *  In addition to containing calibration data, this class can save and load the
     *  data using XML files using \ref Data::Save() and \ref Data::Load().
     *
     *  This calibration data object contains the information required to generate
     *  the geometrical rays of a model for the system geometry. See \ref dlp::Geometry for
     *  more information.
     */
    class Data{
        friend class dlp::Calibration; /**< Allows \ref Calibration::Camera and \ref Calibration::Projector to modify private calibration data */
    public:
        /** @brief Enumeration to access rotation and translation rows from cv::Mat extrinsic object */
        enum ExtrinsicRow{
            EXTRINSIC_ROW_ROTATION    = 0,      /**< Row location of rotation vector in extrinsic cv::Mat object    */
            EXTRINSIC_ROW_TRANSLATION = 1       /**< Row location of translation vector in extrinsic cv::Mat object */
        };

        Data();
        ~Data();
        Data(const Data &data);
        Data & operator=(const Data &data);

        void Clear();

        bool isComplete() const;
        bool isCamera() const;

        ReturnCode GetData(cv::Mat *intrinsic,
                          cv::Mat *extrinsic,
                          cv::Mat *distortion,
                          double  *reprojection_error)const;
        ReturnCode GetImageResolution(unsigned int *columns,
                                     unsigned int *rows)const;
        ReturnCode GetModelResolution(unsigned int *columns,
                                     unsigned int *rows)const;

        ReturnCode Save(const std::string &filename);
        ReturnCode Load(const std::string &filename);

    private:
        bool calibration_complete_;         /**< Boolean flag to mark that calibration object is completed calibration data */
        bool calibration_of_camera_;        /**< Boolean flag to mark if the calibration object is a camera. If false the data is for a projector */

        // Calibration image resolution
        unsigned int image_columns_;        /**< Number of columns for calibration board image captures that generated calibration data */
        unsigned int image_rows_;           /**< Number of rows for calibration board image captures that generated calibration data */

        // Model resolution
        unsigned int model_columns_;        /**< Number of pixel columns the calibration model contains */
        unsigned int model_rows_;           /**< Number of pixel rows the calibration model contains */

        // Final calibration data
        cv::Mat intrinsic_;                 /**< OpenCV matrix which contains the focal length and point of calibrated model */
        cv::Mat distortion_;                /**< OpenCV matrix which contains the lens distortion coefficients */
        cv::Mat extrinsic_;                 /**< OpenCV matrix which contains the translation and rotation vectors of the calibration model relative to the calibraiton board */
        std::vector<cv::Mat> homography_;   /**< OpenCV matrix which contains the homography transforms to convert projected chessboard feature pixel locations to 2D real-world locations */

        PixelArrayConfiguration pixel_array_config_;

        double reprojection_error_; /**< Sum of errors between actual calibration baord feature locations and the reprojected estimations */
    };


    /** @class Camera
     *  @brief Contains methods for calibrating a camera.
     *
     *  To calibrate a camera, the following steps should be performed:
     *  -# Create \ref dlp::Calibration::Camera object
     *  -# Setup the module with \ref dlp::Calibration::Camera::Setup()
     *     - If using dlp::Camera object, the resolution of the camera calibration module can be set using \ref dlp::Calibration::Camera::SetCamera() before calling \ref dlp::Calibration::Camera::Setup()
     *  -# Create the calibration board using \ref dlp::Calibration::Camera::GenerateCalibrationBoard()
     *  -# Add images using \ref dlp::Calibration::Camera::AddCalibrationBoard()
     *     - If calibrating a projector concurrently and the projector board was not found, use \ref dlp::Calibration::Camera::RemoveLastCalibrationBoard() to remove the most recently added board. This ensures the homographies for each calibration board position match between the camera and projector calibration modules
     *  -# Add images until \ref dlp::Calibration::Camera::GetCalibrationProgress() shows that enough images have been successfully added
     *  -# After adding the number of images requires, call dlp::Calibration::Camera::Calibrate() to perform the calibration
     *  -# Retrieve the calibration data using \ref dlp::Calibration::Camera::GetCalibrationData()
     *     - This calibration data can be saved for later use
     *     - If calibration data needs to be updated, it can be loaded into the module by using \ref dlp::Calibration::Camera::SetCalibrationData() . The camera calibration module still needs to be setup.
     *
     */
    class Camera: public dlp::Module{
    public:
        Camera();
        ~Camera();

        // Methods to clear the calibration object
        void ClearAll();
        void ClearCalibrationData();
        void ClearCalibrationImagePoints();

        // Object status methods
        bool isCalibrationComplete() const;

        // Object Get methods
        ReturnCode GetSetup(dlp::Parameters *settings) const;
        ReturnCode GetCalibrationProgress(unsigned int *successfull, unsigned int *total_required) const;
        ReturnCode GetCalibrationData(dlp::Calibration::Data *data ) const;

        // Object setup methods
        ReturnCode SetCamera(const dlp::Camera &camera);
        ReturnCode SetCalibrationData(dlp::Calibration::Data &data );
        ReturnCode Setup(const dlp::Parameters &settings);

        // Calibration board generation
        ReturnCode GenerateCalibrationBoard( dlp::Image *calibration_pattern ) const;

        // Add calibration board methods
        ReturnCode AddCalibrationBoard(const dlp::Image   &board_image,   bool *success);

        // Update extrinsic data with single calibration board
        //ReturnCode UpdateExtrinsicsWithCalibrationBoard(const dlp::Image &board_image, bool *success);

        // Method to remove most recent calibration board added
        ReturnCode RemoveLastCalibrationBoard();


        // Calibrate methods
        ReturnCode Calibrate(double *reprojection_error);
        ReturnCode Calibrate(double *reprojection_error,
                            const bool &update_intrinsic,
                            const bool &update_distortion,
                            const bool &update_extrinsic);


    protected:

        // Setting objects
        Parameters::ModelRows       model_rows_;        /**< See \ref dlp::Calibration::Camera::Setup() */
        Parameters::ModelColumns    model_columns_;     /**< See \ref dlp::Calibration::Camera::Setup() */

        Parameters::ImageRows       image_rows_;        /**< See \ref dlp::Calibration::Camera::Setup() */
        Parameters::ImageColumns    image_columns_;     /**< See \ref dlp::Calibration::Camera::Setup() */

        Parameters::BoardCount      board_number_required_; /**< See \ref dlp::Calibration::Camera::Setup() */

        Parameters::BoardForeground  board_color_foreground_;   /**< See \ref dlp::Calibration::Camera::Setup() */
        Parameters::BoardBackground  board_color_background_;   /**< See \ref dlp::Calibration::Camera::Setup() */

        Parameters::BoardFeatureColumns              board_columns_;                    /**< See \ref dlp::Calibration::Camera::Setup() */
        Parameters::BoardFeatureColumnDistance       board_column_distance_;            /**< See \ref dlp::Calibration::Camera::Setup() */
        Parameters::BoardFeatureColumnDistancePixels board_column_distance_in_pixels_;  /**< See \ref dlp::Calibration::Camera::Setup() */
        Parameters::BoardFeatureColumnOffsetPixels   board_column_offset_pixels_;       /**< See \ref dlp::Calibration::Camera::Setup() */
        Parameters::BoardFeatureRows                 board_rows_;                       /**< See \ref dlp::Calibration::Camera::Setup() */
        Parameters::BoardFeatureRowDistance          board_row_distance_;               /**< See \ref dlp::Calibration::Camera::Setup() */
        Parameters::BoardFeatureRowDistancePixels    board_row_distance_in_pixels_;     /**< See \ref dlp::Calibration::Camera::Setup() */
        Parameters::BoardFeatureRowOffsetPixels      board_row_offset_pixels_;          /**< See \ref dlp::Calibration::Camera::Setup() */

        Parameters::SetTangentDistZero  zero_tangent_distortion_;       /**< See \ref dlp::Calibration::Camera::Setup() */
        Parameters::FixSixthOrderDist   fix_sixth_order_distortion_;    /**< See \ref dlp::Calibration::Camera::Setup() */

        bool camera_set_;           /**< Boolean flag to track if SetCamera() was used  */
        Data calibration_data_;     /**< Member to store calibration data for model     */
        unsigned int board_number_successes_;   /**< Number of calibration boards successfully added for calibration */


        std::vector<cv::Point3f> calibration_board_feature_points_xyz_; /**<
            Stores the physical x, y, z position of the calibration board features
            in space. The values are determined during Setup() method.
            @note Assumes board is planar and that z = 0. */

        std::vector<std::vector<cv::Point3f>> object_points_xyz_; /**<
            Vector to store calibration_board_feature_points_xyz_ for each calibration
            image. This creates the asummption the the camera is moving and the calibration
            board stays in the same position in space
            @note The same calibration board must be used for all calibration images! */

        // image_points_xy_ stores the x, y locaiton of the features in the calibration image
        // which is the camera pixel location of the feature.
        std::vector<std::vector<cv::Point2f>> image_points_xy_;

    private:
        DISALLOW_COPY_AND_ASSIGN(Camera);
    };


    /** @class Projector
     *  @brief Contains methods for calibrating a projector by using a camera.
     *
     *  Projector calibration must be performed concurrently with a camera calibration.
     *
     *  To calibrate a projector, the following steps should be performed:
     *  -# Create \ref dlp::Calibration::Projector object
     *  -# Setup the module with \ref dlp::Calibration::Projector::Setup()
     *     - If using dlp::DLP_Platform object, the resolution of the camera calibration module can be set using \ref dlp::Calibration::Projector::SetDlpPlatform() before calling \ref dlp::Calibration::Camera::Setup()
     *  -# Create the calibration board using \ref dlp::Calibration::Projector::GenerateCalibrationBoard()
     *  -# The calibration board image should be added to a dlp::Pattern and then dlp::Pattern::Sequence
     *  -# Prepare the DLP Platform using dlp::DLP_Platform::PreparePatternSequence()
     *  -# Add images using \ref dlp::Calibration::Projector::AddCalibrationBoard()
     *     - If the projector board was not found, use \ref dlp::Calibration::Camera::RemoveLastCalibrationBoard() to remove the most recently added board. This ensures the homographies for each calibration board position match between the camera and projector calibration modules
     *  -# Add images until \ref dlp::Calibration::Projector::GetCalibrationProgress() shows that enough images have been successfully added
     *  -# After adding the number of images requires, call dlp::Calibration::Projector::Calibrate() to perform the calibration
     *  -# Retrieve the calibration data using \ref dlp::Calibration::Projector::GetCalibrationData()
     *     - This calibration data can be saved for later use
     *     - If calibration data needs to be updated, it can be loaded into the module by using \ref dlp::Calibration::Camera::SetCalibrationData() . The camera calibration module still needs to be setup.
     *
     *
     */
    class Projector : public dlp::Calibration::Camera{
    public:
        Projector();
        ~Projector();

        // Override the ClearAll and ClearCalibration to account for addition projector calibration data
        void ClearAll();
        void ClearCalibrationData();

        // Add setup for DlpPlatform
        ReturnCode SetDlpPlatform( const dlp::DLP_Platform &platform );  // Grabs the model resolution and mirror orientation

        // Required method to set camera calibration data
        ReturnCode SetCameraCalibration(const dlp::Calibration::Data &camera_calibration);

        // Override the camera Setup() method so that the DLP_Platform mirror type
        // type is used for the calibration_board_feature_points_xyz_ generation and also to check that
        // camera calibration has been provided
        ReturnCode Setup(const dlp::Parameters &settings);


        // Calibration board generation that matches board size to resolution
        ReturnCode GenerateCalibrationBoard( dlp::Image *calibration_pattern ) const;


        // These methods automatically remove the printed board from the combination
        ReturnCode RemovePrinted_AddProjectedBoard(const dlp::Image &projector_all_on,
                                                   const dlp::Image &projector_all_off,
                                                   const dlp::Image &board_image_printed_and_projected,
                                                         dlp::Image *board_image_projected,
                                                               bool *success);

        // Update extrinsic data with single calibration board
        ReturnCode RemovePrinted_UpdateExtrinsicsWithProjectedBoard(const dlp::Image &projector_all_on,
                                                                    const dlp::Image &projector_all_off,
                                                                    const dlp::Image &board_image_printed_and_projected,
                                                                          dlp::Image *board_image_projected,
                                                                                bool *success);


        // Overload the camera calibration methods because projector calibration is different
        ReturnCode Calibrate(double *reprojection_error);
        ReturnCode Calibrate(double *reprojection_error,
                            const bool &update_intrinsic,
                            const bool &update_distortion,
                            const bool &update_extrinsic);



    protected:
        // Settings for vertical and horizontal offset
        Parameters::ModelOffsetVertical     offset_vertical_;       /**< See \ref dlp::Calibration::Projector::Setup() */
        Parameters::ModelOffsetHorizontal   offset_horizontal_;     /**< See \ref dlp::Calibration::Projector::Setup() */

        // Variables for the effective model size
        float effective_model_width_;       /**< Member to store the width of DMD array in pixels */
        float effective_model_height_;      /**< Member to store the height of DMD array @note For diamond array DMDs the effective height is half the number of pixels */

        Parameters::ModelPixelSize   pixel_size_;           /**< See \ref dlp::Calibration::Projector::Setup() */
        Parameters::ModelFocalLength focal_length_;         /**< See \ref dlp::Calibration::Projector::Setup() */
        Parameters::FixAspectRatio   fix_aspect_ratio_;     /**< See \ref dlp::Calibration::Projector::Setup() */


        float effective_pixel_size_um_;
        float estimated_focal_length_mm_;

        dlp::Calibration::Data camera_calibration_data_;    /**< Member to store calibration data of the camera used to capture projected calibration board */

        bool projecter_set_;    /**<  Boolean flag to track if \ref dlp::Calibration::Projector::SetDlpPlatform() was used */

        DLP_Platform::Mirror   projector_mirror_type_; /**< Member to track DLP_Platform mirror type for calibration_board_feature_points_xyz_ generation */

    private:
        DISALLOW_COPY_AND_ASSIGN(Projector);
    };
    /** @}*/
};
}

#endif  //#ifndef DLP_SDK_CALIBRATION_HPP

