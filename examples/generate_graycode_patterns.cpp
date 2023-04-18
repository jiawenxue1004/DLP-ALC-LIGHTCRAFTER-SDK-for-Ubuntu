#include <dlp_sdk.hpp>
#include <string>

void SavePatternImageData(const dlp::Pattern::Sequence &sequence, const std::string basename){
    unsigned int count = 0;

    count = sequence.GetCount();

    for(unsigned int iPattern = 0; iPattern < count; iPattern++){
        dlp::Pattern temp_pattern;

        // Get the pattern
        sequence.Get(iPattern, &temp_pattern);

        // Save the image data
        std::cout << "Saving image " << basename + dlp::Number::ToString(iPattern) + ".bmp" << std::endl;
        temp_pattern.image_data.Save(basename + dlp::Number::ToString(iPattern) + ".bmp");
    }

    return;
}

int main()
{
    // Variables used throughout code
    dlp::ReturnCode             ret;
    dlp::GrayCode               structured_light;
    dlp::Parameters             settings;
    dlp::Pattern::Sequence      sequence;
    dlp::Pattern::Color         color       = dlp::Pattern::Color::WHITE;   // Since all patterns are greyscale this does not matter for the saved images
    dlp::Pattern::Orientation   orientation = dlp::Pattern::Orientation::VERTICAL;
    bool use_inverted = true;
    bool decode_pixels = true;
    unsigned int region_count = 0;
    unsigned int width  = 600;
    unsigned int height = 400;
    unsigned int pattern_resolution = width;
    unsigned int pattern_count;
    std::string basename = "pattern_";
    std::string image_name = "image";

    // Program menu
    int menu_select = 0;
    int orientation_select = 0;

    do{

        // Update the basename
        basename = dlp::Number::ToString(orientation) + "_" +
                   dlp::Number::ToString(width) + "x" + dlp::Number::ToString(height) + "_";

        // Print menu
        std::cout << std::endl;
        std::cout << "Structured Light Pattern Generator\n" << std::endl;
        std::cout << "0: Exit " << std::endl;
        std::cout << "1: Set resolution ("<< width << "x" << height << ")" << std::endl;
        std::cout << "2: Set orientation ("<< dlp::Number::ToString(orientation) << ")" << std::endl;
        if(use_inverted)  std::cout << "3: Set threshold method (use inverted)" << std::endl;
        else              std::cout << "3: Set threshold method (use albedo)" << std::endl;
        if(decode_pixels) std::cout << "4: Set pixel of region decoding (pixels)" << std::endl;
        else              std::cout << "4: Set pixel of region decoding (" <<dlp::Number::ToString(region_count) << ")" << std::endl;
        std::cout << "5: Generate patterns" << std::endl;
        std::cout  << std::endl << "Enter menu item: ";
        DLP_STD_CIN_GET(menu_select)

        // Execute selection
        switch(menu_select){
        case 0:
            break;
        case 1: // Resolution
            std::cout << "Please enter pattern resolution width: ";
            std::cin  >> width;

            std::cout << "Please enter pattern resolution height: ";
            std::cin  >> height;
            break;
        case 2: // Orientation
            // Program menu
            orientation_select = 0;

            do{
                std::cout << std::endl;
                std::cout << "Available pattern orientations\n" << std::endl;
                std::cout << "0: Vertical   (all rows are identical)" << std::endl;
                std::cout << "1: Horizontal (all columns are identical)" << std::endl;
                std::cout << "2: Diamond Angle 1 (straight lines on diamond arrays)" << std::endl;
                std::cout << "3: Diamond Angle 2 (straight lines on diamond arrays)" << std::endl;
                std::cout  << std::endl << "Select orientation: ";

                // Get menu selection
                DLP_STD_CIN_GET(orientation_select);

                switch(orientation_select){
                case 0:
                    orientation = dlp::Pattern::Orientation::VERTICAL;
                    pattern_resolution = width;
                    break;
                case 1:
                    orientation = dlp::Pattern::Orientation::HORIZONTAL;
                    pattern_resolution = height;
                    break;
                case 2:
                    orientation = dlp::Pattern::Orientation::DIAMOND_ANGLE_1;
                    pattern_resolution = width + (height/2);
                    break;
                case 3:
                    orientation = dlp::Pattern::Orientation::DIAMOND_ANGLE_2;
                    pattern_resolution = width + (height/2);
                    break;
                default:
                    std::cout << "Invalid menu item selected!" << std::endl;
                    break;
                }
                pattern_count= (unsigned int)ceil(log2((double)pattern_resolution));
            }while(orientation_select > 3);
            break;
        case 3:
            std::cout << std::endl;
            std::cout << "Gray Code using inverted patterns:" << std::endl;
            std::cout << "- Each pattern is projected twice: once normal and inverted" << std::endl;
            std::cout << "  i.e. Project 000000111111 and then its inverse 111111000000" << std::endl;
            std::cout << "- Threshold calculated for each pixel for each pattern" << std::endl;
            std::cout << "- Double pattern count" << std::endl;
            std::cout << "- Less sensitive to ambient light changes during sequence" << std::endl;

            std::cout << std::endl;
            std::cout << "Gray Code with albedo threshold:" << std::endl;
            std::cout << "- First pattern has all pixels are on and second pattern all pixels off" << std::endl;
            std::cout << "- Threshold calculated for each pixel at beginning of sequence " << std::endl;
            std::cout << "- Pattern count increases by 2" << std::endl;
            std::cout << "- More sensitive to ambient light changes during sequence" << std::endl;

            std::cout << "\nEnter 1 to use inverted patterns or 0 to use alebdo: ";
            DLP_STD_CIN_GET(use_inverted);
            break;
        case 4:
            std::cout << std::endl;
            std::cout << "Gray Code decoding pixels:" << std::endl;
            std::cout << "- Patterns are symetrical across orientation axis" << std::endl;
            std::cout << "- Each pixel has unique code if all patterns are used" << std::endl;
            std::cout << "- Pattern count to decode all pixels = ceil(log2(pixel_resolution)" << std::endl;

            std::cout << std::endl;
            std::cout << "Gray Code decoding regions:" << std::endl;
            std::cout << "- Patterns are NOT necessarily symetrical" << std::endl;
            std::cout << "- Regions must be equal in pixel count" << std::endl;
            std::cout << "- Regions have unique code" << std::endl;
            std::cout << "- Pattern count = ceil(log2(region_count)" << std::endl;

            std::cout << "\nEnter 1 to decode pixels or 0 to decode regions: ";
            DLP_STD_CIN_GET(decode_pixels);
            if(!decode_pixels) {
                std::cout << "Enter number of regions to measure (resolution must be divisable by number of regions)? ";
                DLP_STD_CIN_GET(region_count);
            }
//            else{
//                // Determine the maximum number of patterns
//                pattern_count= (unsigned int)ceil(log2((double)pattern_resolution));
//            }
            break;
//        case 3: // Gray Code
//            structured_light = new dlp::GrayCode;
//            settings.Clear();
//            sequence.Clear();


//            // Setup the module
//            do{
//                unsigned int include_inverted   = 0;
//                unsigned int use_regions        = 0;
//                unsigned int region_count       = 0;
//                unsigned int pattern_count      = 0;

//                image_name = "GRAYCODE_";

//                std::cout << "Include inverted patterns or use Alebdro threshold(0 = albedo threshold, 1 = use inverted)? ";
//                std::cin  >> include_inverted;
//                if(include_inverted != 1){
//                    include_inverted = 0;
//                    image_name += "ALBEDO_";
//                }
//                else{
//                    image_name += "USE_INVERTED_";
//                }

//                std::cout << "Decode exact pixels or measure equal regions (0 = pixels, 1 = regions)? ";
//                std::cin  >> use_regions;
//                if(use_regions != 0) {
//                    std::cout << "Enter number of regions to measure (resolution must be divisable by number of regions)? ";
//                    std::cin  >> region_count;
//                    image_name += dlp::Number::ToString(region_count) + "_REGIONS_";
//                }
//                else{
//                    use_regions = 0;    // Assume pixel decoding

//                    // Determine the maximum number of patterns
//                    pattern_count= (unsigned int)ceil(log2((double)pattern_resolution));
//                }

//                settings.Set(dlp::StructuredLight::Parameters::PatternColor(color));
//                settings.Set(dlp::StructuredLight::Parameters::PatternOrientation(orientation));
//                settings.Set(dlp::StructuredLight::Parameters::PatternColumns(width));
//                settings.Set(dlp::StructuredLight::Parameters::PatternRows(height));
//                settings.Set(dlp::GrayCode::Parameters::IncludeInverted((bool) include_inverted));
//                settings.Set(dlp::GrayCode::Parameters::PixelThreshold( 5 )); // Not used for generating patterns but required for setup

//                if(use_regions != 0){
//                    settings.Set(dlp::GrayCode::Parameters::MeasureRegions(region_count));
//                }
//                else{
//                    settings.Set(dlp::GrayCode::Parameters::SequenceCount(pattern_count));
//                }

//                // Setup the module
//                ret = structured_light->Setup(settings);

//                // Print the result
//                std::cout << "Setting up structured light module..." << ret.ToString() << std::endl;
//            }while(ret.hasErrors());

//            // Generate the patterns
//            std::cout << "Generating structured light module patterns..." << std::endl;
//            structured_light->GeneratePatternSequence(&sequence);

//            // Save the patterns
//            SavePatternImageData(sequence,image_name + basename);

//            // Clear variables and deallocate memory
//            settings.Clear();
//            sequence.Clear();
//            delete structured_light;
//            break;
//        case 4: // Three Phase
//            structured_light = new dlp::ThreePhase;
//            settings.Clear();
//            sequence.Clear();


//            // Setup the module
//            do{
//                unsigned int pixels_per_period = 0;
//                unsigned int bit_depth  = 0;
//                unsigned int use_hybrid = 1;
//                unsigned int use_hybrid_inverted = 0;
//                dlp::Pattern::Bitdepth pattern_bitdepth;


//                image_name = "THREE_PHASE_";

//                std::cout << "Enter the number of pixels per period (must be multiple of 8): ";
//                std::cin  >> pixels_per_period;
//                image_name += dlp::Number::ToString(pixels_per_period) + "PXL_PERIOD_";

//                std::cout << "Enter the bitdepth (maximum 8. Largest intensity will be (2^bitdepth) - 1): ";
//                std::cin  >> bit_depth;
//                if(bit_depth > 8) bit_depth = 8;
//                if(bit_depth < 1) bit_depth = 8;
//                image_name += dlp::Number::ToString(bit_depth) + "BPP_VALUES_";

//                // Convert integer to pattern bitdepth
//                switch (bit_depth) {
//                case 1:
//                    pattern_bitdepth = dlp::Pattern::Bitdepth::MONO_1BPP;
//                    break;
//                case 2:
//                    pattern_bitdepth = dlp::Pattern::Bitdepth::MONO_2BPP;
//                    break;
//                case 3:
//                    pattern_bitdepth = dlp::Pattern::Bitdepth::MONO_3BPP;
//                    break;
//                case 4:
//                    pattern_bitdepth = dlp::Pattern::Bitdepth::MONO_4BPP;
//                    break;
//                case 5:
//                    pattern_bitdepth = dlp::Pattern::Bitdepth::MONO_5BPP;
//                    break;
//                case 6:
//                    pattern_bitdepth = dlp::Pattern::Bitdepth::MONO_6BPP;
//                    break;
//                case 7:
//                    pattern_bitdepth = dlp::Pattern::Bitdepth::MONO_7BPP;
//                    break;
//                case 8:
//                    pattern_bitdepth = dlp::Pattern::Bitdepth::MONO_8BPP;
//                    break;
//                default:
//                    pattern_bitdepth = dlp::Pattern::Bitdepth::MONO_8BPP;
//                    break;
//                }

//                std::cout << "Include inverted patterns or use Alebdro threshold for GrayCode unwrapping (0 = albedo threshold, 1 = use inverted)? ";
//                std::cin  >> use_hybrid_inverted;
//                if(use_hybrid_inverted != 1){
//                    use_hybrid_inverted = 0;
//                    image_name += "UNWRAP_GRAYCODE_ALBEDO_";
//                }
//                else{
//                    image_name += "UNWRAP_GRAYCODE_USE_INVERTED_";
//                }


//                settings.Set(dlp::StructuredLight::Parameters::PatternColor(color));
//                settings.Set(dlp::StructuredLight::Parameters::PatternOrientation(orientation));
//                settings.Set(dlp::StructuredLight::Parameters::PatternColumns(width));
//                settings.Set(dlp::StructuredLight::Parameters::PatternRows(height));

//                settings.Set(dlp::ThreePhase::Parameters::Bitdepth(pattern_bitdepth));
//                settings.Set(dlp::ThreePhase::Parameters::PixelsPerPeriod(pixels_per_period));
//                settings.Set(dlp::ThreePhase::Parameters::UseHybridUnwrap(true));

//                // Settings to GrayCode unwrapping
//                settings.Set(dlp::GrayCode::Parameters::IncludeInverted((bool) use_hybrid_inverted));
//                settings.Set(dlp::GrayCode::Parameters::PixelThreshold( 5 )); // Not used for generating patterns but required for setup

//                // Setup the module
//                ret = structured_light->Setup(settings);

//                // Print the result
//                std::cout << "Setting up structured light module..." << ret.ToString() << std::endl;
//            }while(ret.hasErrors());

//            // Generate the patterns
//            std::cout << "Generating structured light module patterns..." << std::endl;
//            structured_light->GeneratePatternSequence(&sequence);

//            // Save the patterns
//            SavePatternImageData(sequence,image_name + basename);

//            // Clear variables and deallocate memory
//            settings.Clear();
//            sequence.Clear();
//            delete structured_light;
//            break;
        default:
            std::cout << "Invalid menu selection! \n" << std::endl;
        }

        std::cout << std::endl;
        std::cout << std::endl;

    }while(menu_select != 0);

    // Select options per module

    // Generate the images

    // Save the images

    // Deallocate the structured light module







    return(0);
}

