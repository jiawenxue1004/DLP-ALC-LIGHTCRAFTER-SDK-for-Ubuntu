/** @file   three_phase.hpp
 *  @brief  Contains definitions for the DLP SDK gray code classes
 *  @copyright  2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#ifndef DLP_SDK_THREE_PHASE_HPP
#define DLP_SDK_THREE_PHASE_HPP

#include <common/returncode.hpp>
#include <common/debug.hpp>
#include <common/parameters.hpp>
#include <common/capture/capture.hpp>
#include <common/pattern/pattern.hpp>

#include <structured_light/structured_light.hpp>
#include <structured_light/gray_code/gray_code.hpp>

#define THREE_PHASE_FREQUENCY_MISSING                   "THREE_PHASE_FREQUENCY_MISSING"
#define THREE_PHASE_PIXELS_PER_PERIOD_MISSING           "THREE_PHASE_PIXELS_PER_PERIOD_MISSING"
#define THREE_PHASE_PIXELS_PER_PERIOD_NOT_DIVISIBLE_BY_EIGHT           "THREE_PHASE_PIXELS_PER_PERIOD_NOT_DIVISIBLE_BY_EIGHT"
#define THREE_PHASE_BITDEPTH_MISSING                    "THREE_PHASE_BITDEPTH_MISSING"
#define THREE_PHASE_BITDEPTH_TOO_SMALL                  "THREE_PHASE_BITDEPTH_TOO_SMALL"
#define THREE_PHASE_USE_HYBRID_UNWRAP_MISSING           "THREE_PHASE_USE_HYBRID_UNWRAP_MISSING"
#define THREE_PHASE_ONLY_HYBRID_UNWRAP_SUPPORTED        "THREE_PHASE_ONLY_HYBRID_UNWRAP_SUPPORTED"
#define THREE_PHASE_HYBRID_UNWRAP_MODULE_SETUP_FAILED   "THREE_PHASE_HYBRID_UNWRAP_MODULE_SETUP_FAILED"
#define THREE_PHASE_PI              3.14159265359
#define THREE_PHASE_TWO_THIRDS_PI   2.09439510239

/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{

/** @class      ThreePhase
 *  @ingroup    StructuredLight
 *  @brief      Structured Light subclass used to generate and decode Three Phase sinusoidal patterns
 *              binary patterns
 */
class ThreePhase: public dlp::StructuredLight{
public:

    class Parameters{
    public:
        DLP_NEW_PARAMETERS_ENTRY(Frequency,         "THREE_PHASE_PARAMETERS_FREQUENCY", double, 2.0);
        DLP_NEW_PARAMETERS_ENTRY(PixelsPerPeriod,   "THREE_PHASE_PARAMETERS_PIXELS_PER_PERIOD", unsigned int, 8);
        DLP_NEW_PARAMETERS_ENTRY(Bitdepth,          "THREE_PHASE_PARAMETERS_BITDEPTH",  dlp::Pattern::Bitdepth, dlp::Pattern::Bitdepth::MONO_8BPP);
        DLP_NEW_PARAMETERS_ENTRY(UseHybridUnwrap,   "THREE_PHASE_PARAMETERS_USE_HYBRID_UNWRAP", bool, true);
        DLP_NEW_PARAMETERS_ENTRY(Oversampling,      "THREE_PHASE_PARAMETERS_OVERSAMPLE", unsigned int, 1);
        DLP_NEW_PARAMETERS_ENTRY(RepeatPhases,      "THREE_PHASE_PARAMETERS_REPEAT_PHASES", unsigned int, 1);
    };

    ThreePhase();
    ~ThreePhase();

    ReturnCode Setup(const dlp::Parameters &settings);
    ReturnCode GetSetup( dlp::Parameters *settings) const;

    ReturnCode GeneratePatternSequence(Pattern::Sequence *pattern_sequence);
    ReturnCode DecodeCaptureSequence(Capture::Sequence *capture_sequence,dlp::DisparityMap *disparity_map);

private:

    Parameters::Frequency       frequency_;
    Parameters::PixelsPerPeriod pixels_per_period_;
    Parameters::Bitdepth        bitdepth_;
    Parameters::UseHybridUnwrap use_hybrid_;
    Parameters::Oversampling    over_sample_;
    Parameters::RepeatPhases    repeat_phases_;

    dlp::GrayCode hybrid_unwrap_module_;
    dlp::GrayCode::Parameters::MeasureRegions  hybrid_region_count_;
    dlp::GrayCode::Parameters::IncludeInverted hybrid_include_inverted_;
    dlp::GrayCode::Parameters::PixelThreshold  hybrid_pixel_threshold_;

    float phase_counts_;
    float maximum_value_;
    unsigned int  resolution_;
};
}


#endif // DLP_SDK_THREE_PHASE_HPP
