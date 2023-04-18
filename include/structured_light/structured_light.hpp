/** @file   structured_light.hpp
 *  @brief  Contains definitions for the DLP SDK structured light base classes
 *  @copyright  2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#ifndef DLP_SDK_STRUCTURED_LIGHT_HPP
#define DLP_SDK_STRUCTURED_LIGHT_HPP

#include <common/returncode.hpp>
#include <common/debug.hpp>
#include <common/other.hpp>
#include <common/image/image.hpp>
#include <common/parameters.hpp>
#include <common/capture/capture.hpp>
#include <common/pattern/pattern.hpp>
#include <common/disparity_map.hpp>
#include <common/module.hpp>
#include <dlp_platforms/dlp_platform.hpp>

#define STRUCTURED_LIGHT_NOT_SETUP                                      "STRUCTURED_LIGHT_NOT_SETUP"
#define STRUCTURED_LIGHT_PATTERN_SEQUENCE_NULL                          "STRUCTURED_LIGHT_PATTERN_SEQUENCE_NULL"
#define STRUCTURED_LIGHT_CAPTURE_SEQUENCE_EMPTY                         "STRUCTURED_LIGHT_CAPTURE_SEQUENCE_EMPTY"
#define STRUCTURED_LIGHT_CAPTURE_SEQUENCE_SIZE_INVALID                  "STRUCTURED_LIGHT_CAPTURE_SEQUENCE_SIZE_INVALID"
#define STRUCTURED_LIGHT_CAPTURE_INVALID                                "STRUCTURED_LIGHT_CAPTURE_INVALID"
#define STRUCTURED_LIGHT_PATTERN_SIZE_INVALID                           "STRUCTURED_LIGHT_PATTERN_SIZE_INVALID"
#define STRUCTURED_LIGHT_SETTINGS_PATTERN_ROWS_MISSING                  "STRUCTURED_LIGHT_SETTINGS_PATTERN_ROWS_MISSING"
#define STRUCTURED_LIGHT_SETTINGS_PATTERN_COLUMNS_MISSING               "STRUCTURED_LIGHT_SETTINGS_PATTERN_COLUMNS_MISSING"
#define STRUCTURED_LIGHT_SETTINGS_PATTERN_COLOR_MISSING                 "STRUCTURED_LIGHT_SETTINGS_PATTERN_COLOR_MISSING"
#define STRUCTURED_LIGHT_SETTINGS_PATTERN_ORIENTATION_MISSING           "STRUCTURED_LIGHT_SETTINGS_PATTERN_ORIENTATION_MISSING"
#define STRUCTURED_LIGHT_SETTINGS_IMAGE_ROWS_MISSING                    "STRUCTURED_LIGHT_SETTINGS_IMAGE_ROWS_MISSING"
#define STRUCTURED_LIGHT_SETTINGS_IMAGE_COLUMNS_MISSING                 "STRUCTURED_LIGHT_SETTINGS_IMAGE_COLUMNS_MISSING"
#define STRUCTURED_LIGHT_SETTINGS_SEQUENCE_INCLUDE_INVERTED_MISSING     "STRUCTURED_LIGHT_SETTINGS_SEQUENCE_INCLUDE_INVERTED_MISSING"
#define STRUCTURED_LIGHT_SETTINGS_SEQUENCE_COUNT_MISSING                "STRUCTURED_LIGHT_SETTINGS_SEQUENCE_COUNT_MISSING"
#define STRUCTURED_LIGHT_NULL_POINTER_ARGUMENT                          "STRUCTURED_LIGHT_NULL_POINTER_ARGUMENT"
#define STRUCTURED_LIGHT_DATA_TYPE_INVALID                              "STRUCTURED_LIGHT_DATA_TYPE_INVALID"

/** @brief Contains all DLP SDK classes, functions, etc. */
namespace dlp{

/** @class StructuredLight
 *  @brief Contains base class for 3D structured light pattern generation and decoding classes
 */
class StructuredLight: public dlp::Module{
public:
    /** @defgroup StructuredLight
     *  @brief Contains base class for 3D structured light pattern generation and decoding classes
     *  @{
     */


    class Parameters{
    public:
        DLP_NEW_PARAMETERS_ENTRY(PatternColor,       "STRUCTURED_LIGHT_PARAMETERS_PATTERN_COLOR",   dlp::Pattern::Color, dlp::Pattern::Color::WHITE);
        DLP_NEW_PARAMETERS_ENTRY(PatternRows,        "STRUCTURED_LIGHT_PARAMETERS_PATTERN_ROWS",        unsigned int, 0);
        DLP_NEW_PARAMETERS_ENTRY(PatternColumns,     "STRUCTURED_LIGHT_PARAMETERS_PATTERN_COLUMNS",     unsigned int, 0);
        DLP_NEW_PARAMETERS_ENTRY(PatternOrientation, "STRUCTURED_LIGHT_PARAMETERS_PATTERN_ORIENTATION", dlp::Pattern::Orientation, dlp::Pattern::Orientation::VERTICAL);
    };

    StructuredLight();
    ~StructuredLight();

    virtual ReturnCode GeneratePatternSequence(Pattern::Sequence *pattern_sequence) = 0;
    virtual ReturnCode DecodeCaptureSequence(Capture::Sequence *capture_sequence,dlp::DisparityMap *disparity_map) = 0;

    ReturnCode SetDlpPlatform( const dlp::DLP_Platform &platform );

    unsigned int GetTotalPatternCount();

protected:
    bool                                is_decoded_;
    bool                                projector_set_;
    unsigned int                        sequence_count_total_;

    dlp::DisparityMap                   disparity_map_;

    Parameters::PatternColor        pattern_color_;
    Parameters::PatternRows         pattern_rows_;
    Parameters::PatternColumns      pattern_columns_;
    Parameters::PatternOrientation  pattern_orientation_;
};

}
/** @} */

#endif // DLP_SDK_STRUCTURED_LIGHT_HPP
