/** @file       lcr4500.hpp
 *  @brief      Contains definitions for the DLP SDK LightCrafter 4500 class
 *  @copyright  2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#ifndef DLP_LCR6500_HPP
#define DLP_LCR6500_HPP

#include <atomic>
#include <string>

#include <common/returncode.hpp>
#include <common/other.hpp>
#include <common/parameters.hpp>
#include <common/pattern/pattern.hpp>
#include <common/image/image.hpp>

#include <dlp_platforms/dlp_platform.hpp>
#include <dlp_platforms/lightcrafter_6500/flashdevice.hpp>

#include <dlp_platforms/lightcrafter_6500/dlpc900_image.hpp>

#define LCR6500_FLASH_FW_VERSION_ADDRESS    0xF902C000

#define LCR6500_TRUE            1
#define LCR6500_FALSE           0

#define LCR6500_TRIGGER_OUT_1   1
#define LCR6500_TRIGGER_OUT_2   2

#define LCR6500_COMMAND_FAILED  -1

#define LCR6500_PATTERN_SEQUENCE_VALIDATION_FAILED          "LCR6500_PATTERN_SEQUENCE_VALIDATION_FAILED"
#define LCR6500_PATTERN_SEQUENCE_START_FAILED               "LCR6500_PATTERN_SEQUENCE_START_FAILED"

#define LCR6500_CONNECTION_FAILED                           "LCR6500_CONNECTION_FAILED"
#define LCR6500_NOT_CONNECTED                               "LCR6500_NOT_CONNECTED"
#define LCR6500_SETUP_FAILURE                               "LCR6500_SETUP_FAILURE"

#define LCR6500_SETUP_POWER_STANDBY_FAILED                  "LCR6500_SETUP_POWER_STANDBY_FAILED"
#define LCR6500_SETUP_SHORT_AXIS_FLIP_FAILED                "LCR6500_SETUP_SHORT_AXIS_FLIP_FAILED"
#define LCR6500_SETUP_LONG_AXIS_FLIP_FAILED                 "LCR6500_SETUP_LONG_AXIS_FLIP_FAILED"
#define LCR6500_SETUP_LED_SEQUENCE_AND_ENABLES_FAILED       "LCR6500_SETUP_LED_SEQUENCE_AND_ENABLES_FAILED"
#define LCR6500_SETUP_INVERT_LED_PWM_FAILED                 "LCR6500_SETUP_INVERT_LED_PWM_FAILED"
#define LCR6500_SETUP_LED_CURRENTS_FAILED                   "LCR6500_SETUP_LED_CURRENTS"
#define LCR6500_SETUP_LED_RED_EDGE_DELAYS_FAILED            "LCR6500_SETUP_LED_RED_EDGE_DELAYS_FAILED"
#define LCR6500_SETUP_LED_GREEN_EDGE_DELAYS_FAILED          "LCR6500_SETUP_LED_GREEN_EDGE_DELAYS_FAILED"
#define LCR6500_SETUP_LED_BLUE_EDGE_DELAYS_FAILED           "LCR6500_SETUP_LED_BLUE_EDGE_DELAYS_FAILED"
#define LCR6500_SETUP_INPUT_SOURCE_FAILED                   "LCR6500_SETUP_INPUT_SOURCE_FAILED"
#define LCR6500_SETUP_PARALLEL_PORT_CLOCK_FAILED            "LCR6500_SETUP_PARALLEL_PORT_CLOCK_FAILED"
#define LCR6500_SETUP_DATA_SWAP_FAILED                      "LCR6500_SETUP_DATA_SWAP_FAILED"
#define LCR6500_SETUP_INVERT_DATA_FAILED                    "LCR6500_SETUP_INVERT_DATA_FAILED"
#define LCR6500_SETUP_DISPLAY_MODE_FAILED                   "LCR6500_SETUP_DISPLAY_MODE_FAILED"
#define LCR6500_SETUP_TEST_PATTERN_COLOR_FAILED             "LCR6500_SETUP_TEST_PATTERN_COLOR_FAILED"
#define LCR6500_SETUP_TEST_PATTERN_FAILED                   "LCR6500_SETUP_TEST_PATTERN_FAILED"
#define LCR6500_SETUP_FLASH_IMAGE_FAILED                    "LCR6500_SETUP_FLASH_IMAGE_FAILED"
#define LCR6500_SETUP_TRIGGER_INPUT_1_DELAY_FAILED          "LCR6500_SETUP_TRIGGER_INPUT_1_DELAY_FAILED"
#define LCR6500_SETUP_TRIGGER_OUTPUT_1_FAILED               "LCR6500_SETUP_TRIGGER_OUTPUT_1_FAILED"
#define LCR6500_SETUP_TRIGGER_OUTPUT_2_FAILED               "LCR6500_SETUP_TRIGGER_OUTPUT_2_FAILED"

#define LCR6500_CALIBRATION_PATTERNS_NOT_PREPARED           "LCR6500_CALIBRATION_PATTERNS_NOT_PREPARED"
#define LCR6500_PATTERN_SEQUENCE_NOT_PREPARED               "LCR6500_PATTERN_SEQUENCE_NOT_PREPARED"
#define LCR6500_IN_CALIBRATION_MODE                         "LCR6500_IN_CALIBRATION_MODE"

#define LCR6500_IMAGE_RESOLUTION_INVALID                    "LCR6500_IMAGE_RESOLUTION_INVALID"
#define LCR6500_IMAGE_FORMAT_INVALID                        "LCR6500_IMAGE_FORMAT_INVALID"

#define LCR6500_PATTERN_NUMBER_PARAMETER_MISSING            "LCR6500_PATTERN_NUMBER_PARAMETER_MISSING"
#define LCR6500_PATTERN_FLASH_INDEX_PARAMETER_MISSING       "LCR6500_PATTERN_FLASH_INDEX_PARAMETER_MISSING"


#define LCR6500_PATTERN_SEQUENCE_BUFFERSWAP_TIME_ERROR      "LCR6500_PATTERN_SEQUENCE_BUFFERSWAP_TIME_ERROR"

#define LCR6500_IMAGE_FILE_FORMAT_INVALID                   "LCR6500_IMAGE_FILE_FORMAT_INVALID"
#define LCR6500_IMAGE_LIST_TOO_LONG                         "LCR6500_IMAGE_LIST_TOO_LONG"
#define LCR6500_IMAGE_MEMORY_ALLOCATION_FAILED              "LCR6500_IMAGE_MEMORY_ALLOCATION_FAILED"
#define LCR6500_FLASH_IMAGE_INDEX_INVALID                   "LCR6500_FLASH_IMAGE_INDEX_INVALID"
#define LCR6500_FIRMWARE_UPLOAD_IN_PROGRESS                 "LCR6500_FIRMWARE_UPLOAD_IN_PROGRESS"
#define LCR6500_FIRMWARE_FILE_INVALID                       "LCR6500_FIRMWARE_FILE_INVALID"
#define LCR6500_FIRMWARE_FILE_NOT_FOUND                     "LCR6500_FIRMWARE_FILE_NOT_FOUND"
#define LCR6500_FIRMWARE_FILE_NAME_INVALID                  "LCR6500_FIRMWARE_FILE_NAME_INVALID"
#define LCR6500_DLPC350_FIRMWARE_FILE_NOT_FOUND             "LCR6500_DLPC350_FIRMWARE_FILE_NOT_FOUND"
#define LCR6500_FIRMWARE_FLASH_PARAMETERS_FILE_NOT_FOUND    "LCR6500_FIRMWARE_FLASH_PARAMETERS_FILE_NOT_FOUND"
#define LCR6500_UNABLE_TO_ENTER_PROGRAMMING_MODE            "LCR6500_UNABLE_TO_ENTER_PROGRAMMING_MODE"
#define LCR6500_GET_FLASH_MANUFACTURER_ID_FAILED            "LCR6500_GET_FLASH_MANUFACTURER_ID_FAILED"
#define LCR6500_GET_FLASH_DEVICE_ID_FAILED                  "LCR6500_GET_FLASH_DEVICE_ID_FAILED"
#define LCR6500_FLASHDEVICE_PARAMETERS_NOT_FOUND            "LCR6500_FLASHDEVICE_PARAMETERS_NOT_FOUND"
#define LCR6500_FIRMWARE_FLASH_ERASE_FAILED                 "LCR6500_FIRMWARE_FLASH_ERASE_FAILED"
#define LCR6500_FIRMWARE_MEMORY_ALLOCATION_FAILED           "LCR6500_FIRMWARE_MEMORY_ALLOCATION_FAILED"
#define LCR6500_FIRMWARE_NOT_ENOUGH_MEMORY                  "LCR6500_FIRMWARE_NOT_ENOUGH_MEMORY"
#define LCR6500_FIRMWARE_UPLOAD_FAILED                      "LCR6500_FIRMWARE_UPLOAD_FAILED"
#define LCR6500_FIRMWARE_CHECKSUM_VERIFICATION_FAILED       "LCR6500_FIRMWARE_CHECKSUM_VERIFICATION_FAILED"
#define LCR6500_FIRMWARE_CHECKSUM_MISMATCH                  "LCR6500_FIRMWARE_CHECKSUM_MISMATCH"
#define LCR6500_FIRMWARE_IMAGE_BASENAME_EMPTY               "LCR6500_FIRMWARE_IMAGE_BASENAME_EMPTY"
#define LCR6500_NULL_POINT_ARGUMENT_PARAMETERS              "LCR6500_NULL_POINT_ARGUMENT_PARAMETERS"
#define LCR6500_NULL_POINT_ARGUMENT_MINIMUM_EXPOSURE        "LCR6500_NULL_POINT_ARGUMENT_MINIMUM_EXPOSURE"
#define LCR6500_MEASURE_FLASH_LOAD_TIMING_FAILED            "LCR6500_MEASURE_FLASH_LOAD_TIMING_FAILED"
#define LCR6500_READ_FLASH_LOAD_TIMING_FAILED               "LCR6500_READ_FLASH_LOAD_TIMING_FAILED"

#define LCR6500_GET_STATUS_FAILED                           "LCR6500_GET_STATUS_FAILED"
#define LCR6500_GET_OPERATING_MODE_FAILED                   "LCR6500_GET_OPERATING_MODE_FAILED"

#define LCR6500_SEQUENCE_VALIDATION_FAILED                      "LCR6500_SEQUENCE_VALIDATION_FAILED"
#define LCR6500_SEQUENCE_VALIDATION_EXP_OR_PERIOD_OOO           "LCR6500_SEQUENCE_VALIDATION_EXP_OR_PERIOD_OOO"
#define LCR6500_SEQUENCE_VALIDATION_PATTERN_NUMBER_INVALID      "LCR6500_SEQUENCE_VALIDATION_PATTERN_NUMBER_INVALID"
#define LCR6500_SEQUENCE_VALIDATION_OVERLAP_BLACK               "LCR6500_SEQUENCE_VALIDATION_OVERLAP_BLACK"
#define LCR6500_SEQUENCE_VALIDATION_BLACK_MISSING               "LCR6500_SEQUENCE_VALIDATION_BLACK_MISSING"
#define LCR6500_SEQUENCE_VALIDATION_EXP_PERIOD_DELTA_INVALID    "LCR6500_SEQUENCE_VALIDATION_EXP_PERIOD_DELTA_INVALID"

#define LCR6500_PATTERN_DISPLAY_FAILED                      "LCR6500_PATTERN_DISPLAY_FAILED"

#define LCR6500_ADD_EXP_LUT_ENTRY_FAILED                    "LCR6500_ADD_EXP_LUT_ENTRY_FAILED"

#define LCR6500_SEND_EXP_PATTERN_LUT_FAILED                 "LCR6500_SEND_EXP_PATTERN_LUT_FAILED"
#define LCR6500_SEND_EXP_IMAGE_LUT_FAILED                   "LCR6500_SEND_EXP_IMAGE_LUT_FAILED"

#define LCR6500_SET_POWER_MODE_FAILED                       "LCR6500_SET_POWER_MODE_FAILED"
#define LCR6500_SET_OPERATING_MODE_FAILED                   "LCR6500_SET_OPERATING_MODE_FAILED"
#define LCR6500_SET_TRIGGER_OUTPUT_CONFIG_FAILED            "LCR6500_SET_TRIGGER_OUTPUT_CONFIG_FAILED"
#define LCR6500_SET_PATTERN_DISPLAY_MODE_FAILED             "LCR6500_SET_PATTERN_DISPLAY_MODE_FAILED"
#define LCR6500_SET_PATTERN_TRIGGER_MODE_FAILED             "LCR6500_SET_PATTERN_TRIGGER_MODE_FAILED"
#define LCR6500_SET_VAR_EXP_PATTERN_CONFIG_FAILED           "LCR6500_SET_VAR_EXP_PATTERN_CONFIG_FAILED"





/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{

struct LCR6500_LUT_Entry{
    unsigned int    pattern_lut_index;
    unsigned long   exposure_us;
    unsigned long   dark_time_us;
    bool            clear_after_exposure;
    unsigned char   bitdepth;   // 0 = bitdepth 1
    unsigned char   led_select;
    bool            wait_for_trigger;
    bool            enable_trigger_2;
    unsigned int    pattern_image_index;
    unsigned char   pattern_bit_position;
};

/** @class      LCr6500
 *  @ingroup    DLP_Platforms
 *  @brief      Contains classes for LightCrafter 4500 projector settings and operations.
 *
 *  The LCr4500 class contains several classes used for setting projector parameters,
 *  and controlling projector operation.
 *
 *  The LCr4500 class is specific to the DLP LightCrafter 4500 and should not be used
 *  with other DLP platforms.
 *
 */
class LCr6500 : public DLP_Platform{
public:

    // Add LUT options

    static const unsigned int PATTERN_LUT_SIZE;
    static const unsigned int IMAGE_LUT_SIZE;

    // Should this be here?
    class HardwareStatus{
    public:
        enum Enum: bool{
            INTERNAL_INITIALIZATION_SUCCESSFUL = true,  /**< Internal initialization successful */
            INTERNAL_INITIALIZATION_ERROR      = false  /**< Internal initialization error */
        };
    };

    class PowerMode{
    public:
        enum Enum: unsigned char{
            NORMAL  = 0,    /**< Normal operation. The selected external source will be displayed */
            STANDBY = 1,    /**< Standby mode. Places DLPC900 in standby state and powers down d0 wr 0 the DMD interface */
            RESET   = 2     /**< Perform a software reset */
        };
    };

    class InputDataChannelSwap{

        class PortNumber{
        public:
            enum Enum: bool{
                PORT1 = false,  /**< Set port to Port 1 */
                PORT2 = true    /**< Set port to Port 2 */
            };
        };

        // The DLPC950 interprets Channel A as Green, Channel B as Red, and Channel C as Blue.
        class Subchannels{
        public:
            enum Enum: unsigned int{
                ABC_TO_ABC = 0, /**< No swapping of data subchannels */
                ABC_TO_CAB = 1, /**< Data subchannels are right shifted and circularly rotated  */
                ABC_TO_BCA = 2, /**< Data subchannels are left shifted and circularly rotated   */
                ABC_TO_ACB = 3, /**< Data subchannels B and C are swapped x4 wr                 */
                ABC_TO_BAC = 4, /**< Data subchannels A and B are swapped                       */
                ABC_TO_CBA = 5, /**< Data subchannels A and C are swapped                       */
                INVALID
            };
        };
    };

    class ImageFlip{
    public:
        enum Enum: bool{
            FLIP   = true,  /**< Flip image enabled  */
            NORMAL = false  /**< Flip image disabled */
        };
    };


    class Video{
    public:

        class InputSource{
        public:
            enum Enum : unsigned int{
                PARALLEL_INTERFACE      = 0,    /**< Parallel port input source set */
                INTERNAL_TEST_PATTERNS  = 1,    /**< Internal test pattern source set */
                FLASH_IMAGES            = 2,    /**< Flash images source set */
                SOLID_CURTAIN           = 3,    /**< Solid color curtain source set */
                INVALID
            };
        };

        class ParallelPortWidth{
        public:
            enum Enum : unsigned int{
                BITS_30 = 0,    /**< Parallel port width of 30 bits */
                BITS_24 = 1,    /**< Parallel port width of 24 bits */
                BITS_20 = 2,    /**< Parallel port width of 20 bits */
                BITS_16 = 3,    /**< Parallel port width of 16 bits */
                INVALID
            };
        };

        class PixelMode{
        public:
            enum Enum : unsigned int{
                PORT_1_SINGLE_PIXEL_MODE = 0,   /**< Data Port 1, Single Pixel mode */
                PORT_2_SINGLE_PIXEL_MODE = 1,   /**< Data Port 2 , Single Pixel mode */
                PORT_1_2_DUAL_PIXEL_MODE = 2,   /**< Data Port 1-2, Dual Pixel mode. Even pixel on port 1, Odd pixel on port 2 */
                PORT_2_1_DUAL_PIXEL_MODE = 3,   /**< Data Port 2-1 Dual Pixel mode. Even pixel on port 2, Odd pixel on port 1 */
                INVALID
            };
        };
        class ClockConfiguration{
        public:
            enum Enum : unsigned int{
                PIXEL_CLOCK_1 = 0, /**< Set clock A as parallel clock source */
                PIXEL_CLOCK_2 = 1, /**< Set clock B as parallel clock source */
                PIXEL_CLOCK_3 = 2, /**< Set clock C as parallel clock source */
                INVALID
            };
        };

        class DataEnable{
        public:
            enum Enum : bool{
                DATA_ENABLE_1 = false,  /**< Data Enable 1 */
                DATA_ENABLE_2 = true    /**< Data Enable 2 */
            };
        };

        class VSyncHSync{
        public:
            enum Enum : bool{
                P1_VSYNC_AND_P1_HSYNC = false,  /**< P1 VSync and P1 HSync */
                P2_VSYNC_AND_P2_HSYNC = true    /**< P2 VSync and P2 HSync */
            };
        };

        class DisplayCurtain{
        public:
            static const unsigned char MAXIMUM_COLOR_VALUE;
        };

        class TestPattern{
        public:
            static const unsigned int COLOR_MAXIMUM;
            enum Enum : unsigned int{
                SOLID_FIELD       = 0,  /**< Sets the test pattern to a single color for every pixel in the field   */
                HORIZONTAL_RAMP   = 1,  /**< Sets the test pattern to a horizontal ramp of intensities              */
                VERTICAL_RAMP     = 2,  /**< Sets the test pattern to a vertical ramp of intensities                */
                HORIZONTAL_LINES  = 3,  /**< Sets the test pattern to display a series of horizontal lines          */
                DIAGONAL_LINES    = 4,  /**< Sets the test pattern to display a series of diagonal lines            */
                VERTICAL_LINES    = 5,  /**< Sets the test pattern to display a series of vertical lines            */
                GRID              = 6,  /**< Sets the test pattern to display a grid                                */
                CHECKERBOARD      = 7,  /**< Sets the test pattern to display a checkerboard definable by the user  */
                RGB_RAMP          = 8,  /**< Sets the test pattern to display an RGB ramp                           */
                COLOR_BARS        = 9,  /**< Sets the test pattern to display colored bars                          */
                STEP_BARS         = 10, /**< Sets the test pattern to display stepped bars                          */
                INVALID
            };
        };
    };

    class OperatingMode{
    public:
        enum Enum: unsigned int{
            VIDEO_MODE_NORMAL       = 0,    /**< Sets the LightCrafter 6500 to normal video mode */
            PATTERN_MODE_PRESTORED  = 1,    /**< Sets the LightCrafter 6500 to pre-stored pattern mode (images from flash) */
            VIDEO_MODE_PATTERN      = 2,    /**< Sets the LightCrafter 6500 to Video pattern mode */
            PATTERN_MODE_ON_THE_FLY = 3,    /**< Sets the LightCrafter 6500 to Pattern-On-The-Fly mode (Images loaded through USB/I2C) */
            INVALID
        };
    };

    class InvertData{
    public:
        enum Enum: bool{
            INVERT = true,    /**< Inverted operation. A data value of 0 will flip the mirror to output light */
            NORMAL = false    /**< Normal operation. A data value of 1 will flip the mirror to output light   */
        };
    };

    class Led{
    public:
        static const unsigned char MAXIMUM_CURRENT;
    };



    class TriggerOut1{
    public:
        static const int MINIMUM_DELAY;
        static const int MAXIMUM_DELAY;

         enum InvertOutput: bool{
             NONINVERTED = false,
             INVERTED = true
         };
    };

    class ImageIndex{
    public:
        static const unsigned int  MAXIMUM_INDEX;
    };

    /** @class Pattern
     *  @brief Class containing objects for the settings of individual patterns.
     */
    class Pattern{
    public:

        /** @class Source
         *  @brief Object for setting the input source of the pattern in the sequence.
         *
         *  DLP SDK does not currently support using a source other than flash images.
         *  A setting of true uses video port input, false sets flash image input.
         */

        class Led{
        public:
            enum Enum: unsigned int{
                NONE     = 0,   /**< No LED on                                  */
                RED      = 1,   /**< Red LED on                                 */
                GREEN    = 2,   /**< Green LED on                               */
                YELLOW   = 3,   /**< Red and Green LEDs on simultaneously       */
                BLUE     = 4,   /**< Blue LED on simultaneously                 */
                MAGENTA  = 5,   /**< Red and Blue LEDs on simultaneously        */
                CYAN     = 6,   /**< Green and Blue LEDs on simultaneously      */
                WHITE    = 7,   /**< Red, Green and Blue LEDs on simultaneously */
                INVALID  = 8
            };

           };

        class PatternStartStop{
        public:
            enum Enum : unsigned int{
                STOP    = 0,    /** Stop Pattern Display Sequence. The next "Start" command will restart
the pattern sequence from the beginning */
                PAUSE   = 1,    /** Pause Pattern Display Sequence. The next "Start" command will start d0 wr
0 the pattern sequence by re-displaying the current pattern in the sequence. */
                START   = 2,    /** Start Pattern Display Sequence */
                INVALID
            };
        };

        class Bitdepth{
        public:
            enum Enum: unsigned int{
                MONO_1BPP = 1,  /**< 1 bit per pixel, 1 color */
                MONO_2BPP = 2,  /**< 2 bit per pixel, 1 color */
                MONO_3BPP = 3,  /**< 3 bit per pixel, 1 color */
                MONO_4BPP = 4,  /**< 4 bit per pixel, 1 color */
                MONO_5BPP = 5,  /**< 5 bit per pixel, 1 color */
                MONO_6BPP = 6,  /**< 6 bit per pixel, 1 color */
                MONO_7BPP = 7,  /**< 7 bit per pixel, 1 color */
                MONO_8BPP = 8,  /**< 8 bit per pixel, 1 color */
                INVALID   = 9
            };
        };

        class Exposure{
        public:
            static const unsigned long int MAXIMUM;
            static const unsigned long int PERIOD_DIFFERENCE_MINIMUM;

            static unsigned long int MININUM(const dlp::Pattern::Bitdepth &bitdepth);

        };

    };

    class I2C{
    public:

        class I2CPortSelect{
        public:
            enum Enum : unsigned int{
                PORT1 = 1,  /**< Select I2C Port 1 */
                PORT2 = 2,  /**< Select I2C Port 2 */
                INVALID
            };
        };
    };

    class Parameters{
    public:
        DLP_NEW_PARAMETERS_ENTRY(WhiteBmpFilename,             "LCR6500_PARAMETERS_WHITE_BMP_FILENAME",std::string,"resources/lcr6500/white.bmp");
        DLP_NEW_PARAMETERS_ENTRY(BlackBmpFilename,             "LCR6500_PARAMETERS_BLACK_BMP_FILENAME",std::string,"resources/lcr6500/black.bmp");


        DLP_NEW_PARAMETERS_ENTRY(FlagUseDefault,            "LCR6500_PARAMETERS_USE_DEFAULT", bool, false);

        //DLP_NEW_PARAMETERS_ENTRY(OperatingMode,             "LCR6500_PARAMETERS_OPERATING_MODE",        dlp::LCr6500::OperatingMode::Enum, dlp::LCr6500::OperatingMode::VIDEO);
        DLP_NEW_PARAMETERS_ENTRY(InvertData,                "LCR6500_PARAMETERS_INVERT_DATA",           dlp::LCr6500::InvertData::Enum, dlp::LCr6500::InvertData::NORMAL);
        //DLP_NEW_PARAMETERS_ENTRY(PowerMode,                 "LCR6500_PARAMETERS_POWER_MODE_STANDBY",    dlp::LCr6500::PowerMode::Enum,   dlp::LCr6500::PowerMode::NORMAL);

        DLP_NEW_PARAMETERS_ENTRY(ImageFlipShortAxis,        "LCR6500_PARAMETERS_IMAGE_FLIP_SHORT", dlp::LCr6500::ImageFlip::Enum, dlp::LCr6500::ImageFlip::NORMAL);
        DLP_NEW_PARAMETERS_ENTRY(ImageFlipLongAxis,         "LCR6500_PARAMETERS_IMAGE_FLIP_LONG",  dlp::LCr6500::ImageFlip::Enum, dlp::LCr6500::ImageFlip::NORMAL);

        DLP_NEW_PARAMETERS_ENTRY(VideoInputSource,          "LCR6500_PARAMETERS_VIDEO_INPUT_SOURCE",        dlp::LCr6500::Video::InputSource::Enum,       dlp::LCr6500::Video::InputSource::PARALLEL_INTERFACE);
        DLP_NEW_PARAMETERS_ENTRY(VideoParallelPortWidth,    "LCR6500_PARAMETERS_VIDEO_PARALLEL_PORT_WIDTH", dlp::LCr6500::Video::ParallelPortWidth::Enum,  dlp::LCr6500::Video::ParallelPortWidth::BITS_24);
        //DLP_NEW_PARAMETERS_ENTRY(VideoParallelClockPort,    "LCR6500_PARAMETERS_VIDEO_PARALLEL_CLOCK_PORT", dlp::LCr6500::Video::ParallelClockPort::Enum,   dlp::LCr6500::Video::ParallelClockPort::PORT_1_CLOCK_A);
        //DLP_NEW_PARAMETERS_ENTRY(VideoSwapDataParallel,     "LCR6500_PARAMETERS_VIDEO_SWAP_DATA_PARALLEL",  dlp::LCr6500::Video::DataSwap::SubChannels::Enum,     dlp::LCr6500::Video::DataSwap::SubChannels::ABC_TO_BAC);
        //DLP_NEW_PARAMETERS_ENTRY(VideoSwapDataFpd,          "LCR6500_PARAMETERS_VIDEO_SWAP_DATA_FPD",       dlp::LCr6500::Video::DataSwap::SubChannels::Enum,     dlp::LCr6500::Video::DataSwap::SubChannels::ABC_TO_ABC);

        DLP_NEW_PARAMETERS_ENTRY(VideoTestPattern,                  "LCR6500_PARAMETERS_VIDEO_TEST_PATTERN",                    dlp::LCr6500::Video::TestPattern::Enum,      dlp::LCr6500::Video::TestPattern::CHECKERBOARD);
        DLP_NEW_PARAMETERS_ENTRY(VideoTestPatternForegroundRed,     "LCR6500_PARAMETERS_VIDEO_TEST_PATTERN_FOREGROUND_RED",     unsigned int, 1023);
        DLP_NEW_PARAMETERS_ENTRY(VideoTestPatternForegroundGreen,   "LCR6500_PARAMETERS_VIDEO_TEST_PATTERN_FOREGROUND_GREEN",   unsigned int, 1023);
        DLP_NEW_PARAMETERS_ENTRY(VideoTestPatternForegroundBlue,    "LCR6500_PARAMETERS_VIDEO_TEST_PATTERN_FOREGROUND_BLUE",    unsigned int, 1023);
        DLP_NEW_PARAMETERS_ENTRY(VideoTestPatternBackgroundRed,     "LCR6500_PARAMETERS_VIDEO_TEST_PATTERN_BACKGROUND_RED",     unsigned int,    0);
        DLP_NEW_PARAMETERS_ENTRY(VideoTestPatternBackgroundGreen,   "LCR6500_PARAMETERS_VIDEO_TEST_PATTERN_BACKGROUND_GREEN",   unsigned int,    0);
        DLP_NEW_PARAMETERS_ENTRY(VideoTestPatternBackgroundBlue,    "LCR6500_PARAMETERS_VIDEO_TEST_PATTERN_BACKGROUND_BLUE",    unsigned int,    0);

        //DLP_NEW_PARAMETERS_ENTRY(VideoDisplayCurtain,   "LCR6500_PARAMETERS_VIDEO_DISPLAY_CURTAIN", unsigned char, 0)

        DLP_NEW_PARAMETERS_ENTRY(VideoFlashImage,       "LCR6500_PARAMETERS_VIDEO_FLASH_IMAGE",     unsigned char, 0);


        DLP_NEW_PARAMETERS_ENTRY(LED_SequenceAutomatic, "LCR6500_PARAMETERS_LED_SEQUENCE_AUTOMATIC", bool,  true);
        DLP_NEW_PARAMETERS_ENTRY(LED_EnableRed,         "LCR6500_PARAMETERS_LED_ENABLE_RED",         bool, false);
        DLP_NEW_PARAMETERS_ENTRY(LED_EnableGreen,       "LCR6500_PARAMETERS_LED_ENABLE_GREEN",       bool, false);
        DLP_NEW_PARAMETERS_ENTRY(LED_EnableBlue,        "LCR6500_PARAMETERS_LED_ENABLE_BLUE",        bool, false);

        DLP_NEW_PARAMETERS_ENTRY(LED_InvertPWM,         "LCR6500_PARAMETERS_LED_INVERT_PWM",                   bool, false);
        DLP_NEW_PARAMETERS_ENTRY(LED_CurrentRed,        "LCR6500_PARAMETERS_LED_CURRENT_RED",         unsigned char,   100);
        DLP_NEW_PARAMETERS_ENTRY(LED_CurrentGreen,      "LCR6500_PARAMETERS_LED_CURRENT_GREEN",       unsigned char,   100);
        DLP_NEW_PARAMETERS_ENTRY(LED_CurrentBlue,       "LCR6500_PARAMETERS_LED_CURRENT_BLUE",        unsigned char,   100);

        DLP_NEW_PARAMETERS_ENTRY(LED_DelayRisingRed,    "LCR6500_PARAMETERS_LED_DELAY_RISING_RED",    unsigned char, 0);
        DLP_NEW_PARAMETERS_ENTRY(LED_DelayRisingGreen,  "LCR6500_PARAMETERS_LED_DELAY_RISING_GREEN",  unsigned char, 0);
        DLP_NEW_PARAMETERS_ENTRY(LED_DelayRisingBlue,   "LCR6500_PARAMETERS_LED_DELAY_RISING_BLUE",   unsigned char, 0);

        DLP_NEW_PARAMETERS_ENTRY(LED_DelayFallingRed,   "LCR6500_PARAMETERS_LED_DELAY_FALLING_RED",   unsigned char, 0);
        DLP_NEW_PARAMETERS_ENTRY(LED_DelayFallingGreen, "LCR6500_PARAMETERS_LED_DELAY_FALLING_GREEN", unsigned char, 0);
        DLP_NEW_PARAMETERS_ENTRY(LED_DelayFallingBlue,  "LCR6500_PARAMETERS_LED_DELAY_FALLING_BLUE",  unsigned char, 0);

        //DLP_NEW_PARAMETERS_ENTRY(TriggerSource,             "LCR6500_PARAMETERS_TRIGGER_SOURCE", dlp::LCr6500::Pattern::TriggerSource::Enum, dlp::LCr6500::Pattern::TriggerSource::INTERNAL);

        DLP_NEW_PARAMETERS_ENTRY(TriggerIn1Delay,           "LCR6500_PARAMETERS_TRIGGER_IN_1_DELAY",    unsigned int, 105);
        DLP_NEW_PARAMETERS_ENTRY(TriggerIn1Invert,          "LCR6500_PARAMETERS_TRIGGER_IN_1_INVERT",   bool,     false);
        DLP_NEW_PARAMETERS_ENTRY(TriggerIn2Invert,          "LCR6500_PARAMETERS_TRIGGER_IN_2_INVERT",   bool,     false);

        DLP_NEW_PARAMETERS_ENTRY(TriggerOut1Invert,         "LCR6500_PARAMETERS_TRIGGER_OUT_1_INVERT",          bool, false);
        DLP_NEW_PARAMETERS_ENTRY(TriggerOut2Invert,         "LCR6500_PARAMETERS_TRIGGER_OUT_2_INVERT",          bool, false);

        DLP_NEW_PARAMETERS_ENTRY(TriggerOut1DelayRising,    "LCR6500_PARAMETERS_TRIGGER_OUT_1_DELAY_RISING",  short, 0);
        DLP_NEW_PARAMETERS_ENTRY(TriggerOut1DelayFalling,   "LCR6500_PARAMETERS_TRIGGER_OUT_1_DELAY_FALLING", short, 0);
        DLP_NEW_PARAMETERS_ENTRY(TriggerOut2DelayRising,    "LCR6500_PARAMETERS_TRIGGER_OUT_2_DELAY_RISING",  short, 0);
        DLP_NEW_PARAMETERS_ENTRY(TriggerOut2DelayFalling,   "LCR6500_PARAMETERS_TRIGGER_OUT_2_DELAY_FALLING", short, 0);

        DLP_NEW_PARAMETERS_ENTRY(PatternBitplane,             "LCR6500_PARAMETERS_PATTERN_BITPLANE",            unsigned int, 0);
        DLP_NEW_PARAMETERS_ENTRY(PatternBitplaneRed,          "LCR6500_PARAMETERS_PATTERN_BITPLANE_RED",        unsigned int, 0);
        DLP_NEW_PARAMETERS_ENTRY(PatternBitplaneGreen,        "LCR6500_PARAMETERS_PATTERN_BITPLANE_GREEN",      unsigned int, 0);
        DLP_NEW_PARAMETERS_ENTRY(PatternBitplaneBlue,         "LCR6500_PARAMETERS_PATTERN_BITPLANE_BLUE",       unsigned int, 0);
        DLP_NEW_PARAMETERS_ENTRY(PatternImageIndex,         "LCR6500_PARAMETERS_PATTERN_IMAGE_INDEX",       unsigned int, 0);
        DLP_NEW_PARAMETERS_ENTRY(PatternImageIndexRed,      "LCR6500_PARAMETERS_PATTERN_IMAGE_INDEX_RED",   unsigned int, 0);
        DLP_NEW_PARAMETERS_ENTRY(PatternImageIndexGreen,    "LCR6500_PARAMETERS_PATTERN_IMAGE_INDEX_GREEN", unsigned int, 0);
        DLP_NEW_PARAMETERS_ENTRY(PatternImageIndexBlue,     "LCR6500_PARAMETERS_PATTERN_IMAGE_INDEX_BLUE",  unsigned int, 0);

        DLP_NEW_PARAMETERS_ENTRY(PatternWaitForTrigger,     "LCR6500_PARAMETERS_PATTERN_WAIT_FOR_TRIGGER",  bool,  false);

        DLP_NEW_PARAMETERS_ENTRY(PatternInvert,             "LCR6500_PARAMETERS_PATTERN_INVERT",            bool,   false);
        DLP_NEW_PARAMETERS_ENTRY(PatternShareExposure,      "LCR6500_PARAMETERS_PATTERN_SHARE_EXPOSURE",    bool,   false);
        DLP_NEW_PARAMETERS_ENTRY(PatternSequenceRepeat,     "LCR6500_PARAMETERS_PATTERN_SEQUENCE_REPEAT",   bool,   false);
    };

    LCr6500();
    ~LCr6500();

    // DLP_Platform Pure Virtual Functions
    ReturnCode Connect(std::string id);
    ReturnCode Disconnect();
    bool       isConnected() const;

    ReturnCode Setup(const dlp::Parameters &settings);
    ReturnCode GetSetup(dlp::Parameters* settings)const;

    ReturnCode ProjectSolidWhitePattern();
    ReturnCode ProjectSolidBlackPattern();

    ReturnCode PreparePatternSequence(const dlp::Pattern::Sequence &pattern_sequence);
    ReturnCode StartPatternSequence(const unsigned int &start, const unsigned int &patterns, const bool &repeat);
    ReturnCode DisplayPatternInSequence(const unsigned int &pattern_index, const bool &repeat);
    ReturnCode StopPatternSequence();


    // LightCrafter 6500 Specific methods
    ReturnCode PatternSettingsValid(dlp::Pattern &arg_pattern);

    ReturnCode UploadFirmware(std::string firmware_filename);
    ReturnCode CreateFirmware(const std::string              &new_firmware_filename,
                              const std::vector<std::string> &image_filenames);

    bool FirmwareUploadInProgress();

    long long GetFirmwareUploadPercentComplete();
    long long GetFirmwareFlashEraseComplete();

    ReturnCode CreateFirmwareImages(const dlp::Pattern::Sequence    &arg_pattern_sequence,
                                    const std::string               &arg_image_filename_base,
                                    dlp::Pattern::Sequence          &ret_pattern_sequence,
                                    std::vector<std::string>        &ret_image_filename_list );

    bool       UploadImages_InProgress();
    float      UploadImages_PercentComplete();

    bool StartPatternImageStorage(const unsigned char &bitplane_position,
                                  const unsigned char &mono_bpp,
                                        unsigned char &ret_pattern_number);



private:


    ReturnCode ConvertSequenceToLut(const dlp::Pattern::Sequence &pattern_sequence, std::vector<LCR6500_LUT_Entry> &sequence_LUT);
    ReturnCode SendLutWithImages(const std::vector<LCR6500_LUT_Entry> &dlpc900_lut , const bool &repeat);
    ReturnCode UploadPrestoredCompressedImages(const unsigned char &start_index, const unsigned char &last_index);
    ReturnCode UploadCompressedImage(const unsigned char &fw_image_index, unsigned char *compressed_image_byte_array, const int &compressed_image_data_size);
    ReturnCode SavePatternIntImageAsRGBfile(Image &image_int, const std::string &filename);

    // Pattern Sequence related methods
    static int DlpPatternColorToLCr6500Led(const dlp::Pattern::Color &color);
    static int DlpPatternBitdepthToLCr6500Bitdepth(const dlp::Pattern::Bitdepth &depth);

    // Firmware upload related methods
    bool ProcessFlashParamsLine(const std::string &line);
    int  GetSectorNum(unsigned int Addr);


    // Setting members
    Parameters::FlagUseDefault      use_default_;
    //Parameters::PowerStandyMode     power_standby_;
    //Parameters::OperatingMode       display_mode_;
    Parameters::InvertData          invert_data_;
    Parameters::ImageFlipShortAxis  short_axis_flip_;
    Parameters::ImageFlipLongAxis   long_axis_flip_;

    Parameters::VideoInputSource        input_source_;
    Parameters::VideoParallelPortWidth  parallel_port_width_;
    //Parameters::VideoParallelClockPort  parallel_port_clock_;
    //Parameters::VideoSwapDataParallel   parallel_data_swap_;

    Parameters::VideoTestPatternForegroundRed   test_pattern_foreground_red;
    Parameters::VideoTestPatternForegroundGreen test_pattern_foreground_green;
    Parameters::VideoTestPatternForegroundBlue  test_pattern_foreground_blue;
    Parameters::VideoTestPatternBackgroundRed   test_pattern_background_red;
    Parameters::VideoTestPatternBackgroundGreen test_pattern_background_green;
    Parameters::VideoTestPatternBackgroundBlue  test_pattern_background_blue;


    Parameters::LED_SequenceAutomatic   led_sequence_mode_;
    Parameters::LED_EnableRed           led_red_enable_;
    Parameters::LED_EnableGreen         led_green_enable_;
    Parameters::LED_EnableBlue          led_blue_enable_;

    Parameters::LED_InvertPWM           led_invert_pwm_;
    Parameters::LED_CurrentRed          led_red_current_;
    Parameters::LED_CurrentGreen        led_green_current_;
    Parameters::LED_CurrentBlue         led_blue_current_;

    Parameters::LED_DelayRisingRed      led_red_edge_delay_rising_;
    Parameters::LED_DelayRisingGreen    led_green_edge_delay_rising_;
    Parameters::LED_DelayRisingBlue     led_blue_edge_delay_rising_;

    Parameters::LED_DelayFallingRed     led_red_edge_delay_falling_;
    Parameters::LED_DelayFallingGreen   led_green_edge_delay_falling_;
    Parameters::LED_DelayFallingBlue    led_blue_edge_delay_falling_;

    //Parameters::TriggerSource           trigger_source_;
    Parameters::TriggerIn1Delay         trigger_in_1_delay_;
    Parameters::TriggerIn1Invert        trigger_in_1_invert_;
    Parameters::TriggerIn2Invert        trigger_in_2_invert_;



    Parameters::TriggerOut1Invert       trigger_out_1_invert_;
    Parameters::TriggerOut2Invert       trigger_out_2_invert_;
    Parameters::TriggerOut1DelayRising  trigger_out_1_rising_;
    Parameters::TriggerOut1DelayFalling trigger_out_1_falling_;
    Parameters::TriggerOut2DelayRising  trigger_out_2_rising_;
    Parameters::TriggerOut2DelayFalling trigger_out_2_falling_;

    Parameters::WhiteBmpFilename image_file_white_;
    Parameters::BlackBmpFilename image_file_black_;

    Parameters::PatternWaitForTrigger pattern_wait_for_trigger_;



    FlashDevice myFlashDevice;
    std::string firmwarePath;

    bool        previous_command_in_progress;

    bool        firmware_upload_restart_needed;


    std::atomic_flag        firmware_upload_in_progress = ATOMIC_FLAG_INIT;
    std::atomic <long long> firmware_upload_percent_erased_;
    std::atomic <long long> firmware_upload_percent_complete_;



    std::atomic_flag        image_upload_in_progress = ATOMIC_FLAG_INIT;
    std::atomic<long long>  image_upload_total_data_size_;
    std::atomic<long long>  image_upload_total_data_size_sent_;
    std::atomic<float>      image_upload_percent_complete_;
    int image_previous_first_;
    int image_previous_last_;

    std::vector<COMPRESSED_BITMAPIMAGES> compressed_images_;
    COMPRESSED_BITMAPIMAGES pattern_image_white_;
    COMPRESSED_BITMAPIMAGES pattern_image_black_;

    ReturnCode CompressImageFile(const std::string image_file, COMPRESSED_BITMAPIMAGES* compressed_bitmaps );


    bool                pattern_sequence_prepared_;
    dlp::Pattern::Sequence   pattern_sequence_;





};

}

#endif // DLP_SDK_LCR6500_HPP
