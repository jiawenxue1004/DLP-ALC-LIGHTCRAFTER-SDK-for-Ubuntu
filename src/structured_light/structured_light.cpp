/** @file   structured_light.cpp
 *  @brief  Contains setting entries and methods to set and retrieve entries
 *  for the structured light class.
 */

#include <common/debug.hpp>
#include <common/returncode.hpp>
#include <common/image/image.hpp>
#include <common/other.hpp>
#include <common/parameters.hpp>
#include <common/capture/capture.hpp>
#include <common/pattern/pattern.hpp>
#include <common/returncode.hpp>
#include <structured_light/structured_light.hpp>

/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{

/** @brief Returns the number of patterns */
unsigned int StructuredLight::GetTotalPatternCount(){
    return this->sequence_count_total_;
}

/** @brief Default Constructor
 * Sets name to "STRUCTURED LIGHT: " and leaves it in a totally uncomplete setup
 * ie no cameras/projectors images or patterns*/
StructuredLight::StructuredLight(){

    this->debug_.SetName("STRUCTURED LIGHT(" + dlp::Number::ToString(this)+ "): ");

    this->is_setup_   = false;
    this->is_decoded_ = false;

    this->projector_set_ = false;

    this->disparity_map_.Clear();

    this->sequence_count_total_ = 0;

    this->pattern_rows_.Set(0);
    this->pattern_columns_.Set(0);
    this->pattern_orientation_.Set(dlp::Pattern::Orientation::VERTICAL);
}

StructuredLight::~StructuredLight(){
    // Release the disparity map image data
    this->disparity_map_.Clear();
}

/** @brief Retrieves the DLP Platform resolution
 *  @retval STRUCTURED_LIGHT_NOT_SETUP \ref Setup() has not been run*/
ReturnCode StructuredLight::SetDlpPlatform( const dlp::DLP_Platform &platform ){
    ReturnCode ret;

    this->debug_.Msg("Retrieving DLP Platform resolution...");

    // Check that DLP_Platform is setup
    if(!platform.isPlatformSetup())
        return ret.AddError(STRUCTURED_LIGHT_NOT_SETUP);

    // Grab the resolution from DMD to set the model and image settings
    unsigned int rows;
    unsigned int columns;

    platform.GetRows(&rows);
    platform.GetColumns(&columns);

    // Save these values to the model and image settings
    this->pattern_rows_.Set(rows);
    this->pattern_columns_.Set(columns);

    this->debug_.Msg("Projector resolution  = " +
                     this->pattern_columns_.GetEntryValue() +
                     " by " +
                     this->pattern_rows_.GetEntryValue());

    this->projector_set_ = true;

    return ret;
}


}
