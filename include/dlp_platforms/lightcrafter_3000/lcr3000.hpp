/** \file       lcr3000.hpp
 *  @brief      Contains definitions for the DLP SDK LightCrafter 3000 class
 *  \copyright  2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#ifndef LCR3000_HPP
#define LCR3000_HPP

#endif // LCR3000_HPP

#include <common/returncode.hpp>
#include <common/parameters.hpp>
#include <common/pattern/pattern.hpp>
#include <common/image/image.hpp>

#include <dlp_platforms/dlp_platform.hpp>
#include <dlp_platforms/lightcrafter_3000/lcr3000_definitions.hpp>

#include <stdint.h>

#define ASIO_STANDALONE
#include <asio.hpp>

#define DLP_LIGHTCRAFTER_3000_SYSTEM_BUSY "DLP_LIGHTCRAFTER_3000_SYSTEM_BUSY"


#define LCR3000_NOT_CONNECTED   "LCR3000_NOT_CONNECTED"

#define LCR3000_INVALID_IP_PORT_NUM "LCR3000_INVALID_IP_PORT_NUM"
#define LCR3000_FAIL "LCR3000_FAIL"
#define LCR3000_SEQUENCE_NOT_PREPARED   "LCR3000_SEQUENCE_NOT_PREPARED"
#define LCR3000_INVALID_REVISION "LCR3000_INVALID_REVISION"
#define LCR3000_INVALID_DISPLAY_MODE "LCR3000_INVALID_DISPLAY_MODE"
#define LCR3000_SEND_COMMAND_FAILED "LCR3000_SEND_COMMAND_FAILED"
#define LCR3000_OUT_OF_RESOURCE "LCR3000_OUT_OF_RESOURCE"
#define LCR3000_INVALID_PARAM "LCR3000_INVALID_PARAM"
#define LCR3000_NULL_PTR "LCR3000_NULL_PTR"
#define LCR3000_NOT_INITIALIZED "LCR3000_NOT_INITIALIZED"
#define LCR3000_DEVICE_FAIL "LCR3000_DEVICE_FAIL"
#define LCR3000_DEVICE_BUSY "LCR3000_DEVICE_BUSY"
#define LCR3000_PACKET_FORMAT_ERROR "LCR3000_PACKET_FORMAT_ERROR"
#define LCR3000_TIMEOUT "LCR3000_TIMEOUT"
#define LCR3000_NOT_SUPPORTED "LCR3000_NOT_SUPPORTED"
#define LCR3000_NOT_FOUND "LCR3000_NOT_FOUND"
#define LCR3000_TEST_PATTER_OUT_OF_BOUNDS "LCR3000_TEST_PATTER_OUT_OF_BOUNDS"
#define LCR3000_COULD_NOT_CONNECT "LCR3000_COULD_NOT_CONNECT"
#define LCR3000_HEADER_READ_FAIL "LCR3000_HEADER_READ_FAIL"
#define LCR3000_CHECKSUM_RESPONSE_FAILED "LCR3000_CHECKSUM_RESPONSE_FAILED"
#define LCR3000_CMD_NOT_EXCECUTED_REASON_BUSY "LCR3000_CMD_NOT_EXCECUTED_REASON_BUSY"
#define LCR3000_PACKET_CONTAINS_ERROR "LCR3000_PACKET_CONTAINS_ERROR"
#define LCR3000_CMD_FAILED "LCR3000_CMD_FAILED"
#define LCR3000_UNSUPPORTED_COMMAND "LCR3000_UNSUPPORTED_COMMAND"
#define LCR3000_CHECKSUM_FAILED "LCR3000_CHECKSUM_FAILED"
#define LCR3000_COMMAND_CONTINUE_FAILED "LCR3000_COMMAND_CONTINUE_FAILED"
#define LCR3000_UNDOCUMENTED_ERROR "LCR3000_UNDOCUMENTED_ERROR"
#define LCR3000_RESPONSE_WRITE_OR_READ "LCR3000_RESPONSE_WRITE_OR_READ"
#define LCR3000_UNKNOWN_PACKET_TYPE "LCR3000_UNKNOWN_PACKET_TYPE"

#define LCR3000_PATTERN_SEQUENCE_APPENDED_WITH_BLACK_PATTERNS   "LCR3000_PATTERN_SEQUENCE_APPENDED_WITH_BLACK_PATTERNS"
#define LCR3000_IMAGE_RESOLUTION_INVALID    "LCR3000_IMAGE_RESOLUTION_INVALID"


namespace dlp{


    /** \class LCr3000
     *  \ingroup DLP_Platforms
     *  @brief Contains classes for LightCrafter 3000 projector settings and operations
     *
     * The LCr3000 class contains several classes used for setting projector parameters,
     *  and controlling projector operation.
     *
     *  The LCr3000 class is specific to the DLP LightCrafter 3000 and should not be used
     *  with other DLP platforms.
     * */
    class LCr3000 : public DLP_Platform{
    public:
        //from lcr_cmd
        typedef enum
        {
            REV_DM365 = 0x00,
            REV_FPGA = 0x10,
            REV_MSP430 = 0x20,

        }
        LCR_Revision_t;

        typedef enum
        {
            PWR_NORMAL,
            PWR_STANDBY,
        } LCR_PowerMode_t;

        typedef enum
        {
            LED_RED,
            LED_GREEN,
            LED_BLUE,
            LED_DEFAULT, //Only used for running custom sequence referred in Demo-8
        } LCR_LEDSelect_t;

        typedef enum
        {
            SOL_DELETE,
            SOL_LOAD,
            SOL_SET_DEFAULT,
        } LCR_SolutionCommand_t;

        typedef enum
        {
            DISP_MODE_IMAGE,        /* Static Image */
            DISP_MODE_TEST_PTN,     /* Internal Test pattern */
            DISP_MODE_VIDEO,        /* HDMI Video */
            DISP_MODE_VIDEO_INT_PTN,    /* Interleaved pattern */
            DISP_MODE_PTN_SEQ,      /* Pattern Sequence */
            DISP_NUM_MODES,
        } LCR_DisplayMode_t;

        typedef enum
        {
            TEST_PTN_FINE_CHECKER,      /* 0x0 - Fine Checkerboard */
            TEST_PTN_SOLID_BLACK,       /* 0x1 - Solid black */
            TEST_PTN_SOLID_WHITE,       /* 0x2 - Solid white */
            TEST_PTN_SOLID_GREEN,       /* 0x3 - Solid green */
            TEST_PTN_SOLID_BLUE,        /* 0x4 - Solid blue */
            TEST_PTN_SOLID_RED,         /* 0x5 - Solid red */
            TEST_PTN_VERTICAL_LINES,    /* 0x6 - Vertical lines (1-white, 7-black) */
            TEST_PTN_HORIZONTAL_LINES,  /* 0x7 - Horizontal lines (1-white, 7-black) */
            TEST_PTN_FINE_VERTICAL_LINES, /* 0x8 - Vertical lines (1-white, 1-black) */
            TEST_PTN_FILE_HORIZONTAL_LINES, /* 0x9 - Horizontal lines (1-white, 1-black) */
            TEST_PTN_DIAG_LINES,        /* 0xA - Diagonal lines */
            TEST_PTN_VERTICAL_RAMP,     /* 0xB - Vertical Gray Ramps */
            TEST_PTN_HORIZONTAL_RAMP,   /* 0xC - Horizontal Gray Ramps */
            TEST_PTN_ANXI_CHECKER,      /* 0xD - ANSI 4x4 Checkerboard */
            NUM_TEST_PTNS
        } LCR_TestPattern_t;


        typedef enum
        {
            TRIGGER_TYPE_SW,
            TRIGGER_TYPE_AUTO,
            TRIGGER_TYPE_EXTRNAL,
            TRIGGER_TYPE_EXTRNAL_INV,
            TRIGGER_TYPE_CAMERA,
            TRIGGER_TYPE_CAMERA_INV,
            /* External trigger + exposure on the same line */
            TRIGGER_TYPE_TRIG_EXP,
            NUM_TRIGGER_TYPES
        } LCR_TriggerType_t;


        typedef enum
        {
            TRIGGER_EDGE_POS,
            TRIGGER_EDGE_NEG
        } LCR_TriggerEdge_t;

        typedef enum
        {
            CAPTURE_STOP,
            CAPTURE_SINGLE,
            CAPTURE_STREAM,
        } LCR_CaptureMode_t;

        typedef enum
        {
            PTN_TYPE_NORMAL,
            PTN_TYPE_INVERTED,
            PTN_TYPE_HW,
        } LCR_PatternType_t;

        typedef uint16 LCR_PatternCount_t;

        typedef struct
        {
            uint8 BitDepth;
            LCR_PatternCount_t NumPatterns;
            LCR_PatternType_t  PatternType;
            LCR_TriggerType_t InputTriggerType;
            uint32 InputTriggerDelay;
            uint32 AutoTriggerPeriod;
            uint32 ExposureTime;
            LCR_LEDSelect_t LEDSelect;
            uint8 Repeat;
        } LCR_PatternSeqSetting_t;

        typedef struct
        {
            uint8 Number;
            uint8 Invert;
        } LCR_HWPattern_t;

        typedef struct
        {
            uint8 index; //0-31
            uint8 numOfPatn; //Number of patterns from the start index
            LCR_HWPattern_t hwPatArray[32];
        } LCR_HWPatternSeqDef_t;

        typedef struct
        {
            uint16 ResolutionX;
            uint16 ResolutionY;
            uint16 FirstPix;
            uint16 FirstLine;
            uint16 ActiveWidth;
            uint16 ActiveHeight;
        } LCR_VideoSetting_t;

        typedef struct
        {
            uint8 FrameRate;
            uint8 BitDepth;
            uint8 RGB;
        } LCR_VideoModeSetting_t;

        typedef struct
        {
            uint8 Rotate;
            uint8 LongAxisFlip;
            uint8 ShortAxisFlip;
        } LCR_DisplaySetting_t;

        typedef struct
        {
            uint16 Red;
            uint16 Green;
            uint16 Blue;
        } LCR_LEDCurrent_t;

        typedef struct
        {
            uint8 Enable;
            uint8 Source;
            uint8 Polarity;
            uint32 Delay;
            uint32 PulseWidth;
            uint8 Reserved[12];
        } LCR_CamTriggerSetting_t;

        typedef struct
        {
            LCR_DisplayMode_t DisplayMode;
            LCR_DisplaySetting_t Display;
            LCR_LEDCurrent_t LEDCurrent;
            LCR_TestPattern_t TestPattern;
            LCR_VideoSetting_t Video;
            LCR_PatternSeqSetting_t PatternSeq;
            LCR_CamTriggerSetting_t CamTrigger;
            LCR_VideoModeSetting_t VideoMode;
            uint32 StaticColor;
            uint8 Reserved[32];
        } LCR_Setting_t;

        //from packetizer
        enum LCR_PacketType {
            PKT_TYPE_BUSY,
            PKT_TYPE_ERROR,
            PKT_TYPE_WRITE,
            PKT_TYPE_WRITE_RESP,
            PKT_TYPE_READ,
            PKT_TYPE_READ_RESP
        };

        typedef enum
        {
            LCR_CMD_PKT_TYPE_READ,
            LCR_CMD_PKT_TYPE_WRITE,
        } LCR_CommandType_t;

        //from error
        typedef enum
        {
            SUCCESS = 0,
            FAIL,
            ERR_OUT_OF_RESOURCE,
            ERR_INVALID_PARAM,
            ERR_NULL_PTR,
            ERR_NOT_INITIALIZED,
            ERR_DEVICE_FAIL,
            ERR_DEVICE_BUSY,
            ERR_FORMAT_ERROR,
            ERR_TIMEOUT,
            ERR_NOT_SUPPORTED,
            ERR_NOT_FOUND
        } ErrorCode_t;



        class ImageFlip{
        public:
            enum Enum{
                FLIP   = 1,     /**< Flip image enabled  */
                NORMAL = 0      /**< Flip image disabled */
            };
        };

        class OutputTriggerPolarity{
        public:
            enum Enum{
                POSITIVE = 0,
                NEGATIVE = 1
            };
        };


        class InputTriggerType{
        public:
            enum Enum{
                COMMAND        = 0,
                AUTO           = 1,
                EXTRNAL        = 2,
                EXTRNAL_INV    = 3,
                CAMERA         = 4,
                CAMERA_INV     = 5,
                TRIG_EXP       = 6
            };
        };


        class Parameters{
        public:
            DLP_NEW_PARAMETERS_ENTRY(LED_CurrentRed,        "LCR3000_PARAMETERS_LED_CURRENT_RED",         unsigned int,   274);
            DLP_NEW_PARAMETERS_ENTRY(LED_CurrentGreen,      "LCR3000_PARAMETERS_LED_CURRENT_GREEN",       unsigned int,   274);
            DLP_NEW_PARAMETERS_ENTRY(LED_CurrentBlue,       "LCR3000_PARAMETERS_LED_CURRENT_BLUE",        unsigned int,   274);

            DLP_NEW_PARAMETERS_ENTRY(ImageFlipShortAxis,    "LCR3000_PARAMETERS_IMAGE_FLIP_SHORT", dlp::LCr3000::ImageFlip::Enum, dlp::LCr3000::ImageFlip::NORMAL);
            DLP_NEW_PARAMETERS_ENTRY(ImageFlipLongAxis,     "LCR3000_PARAMETERS_IMAGE_FLIP_LONG",  dlp::LCr3000::ImageFlip::Enum, dlp::LCr3000::ImageFlip::NORMAL);

            DLP_NEW_PARAMETERS_ENTRY(TriggerInputType,      "LCR3000_PARAMETERS_INPUT_TRIGGER_TYPE", dlp::LCr3000::InputTriggerType::Enum, dlp::LCr3000::InputTriggerType::AUTO);
            DLP_NEW_PARAMETERS_ENTRY(TriggerInputDelay,     "LCR3000_PARAMETERS_INPUT_TRIGGER_DELAY", unsigned int, 0);

            DLP_NEW_PARAMETERS_ENTRY(TriggerOutputEnable,       "LCR3000_PARAMETERS_OUTPUT_TRIGGER_ENABLE",                 bool, true);
            DLP_NEW_PARAMETERS_ENTRY(TriggerOutputPolarity,     "LCR3000_PARAMETERS_OUTPUT_TRIGGER_POLARITY",dlp::LCr3000::OutputTriggerPolarity::Enum,dlp::LCr3000::OutputTriggerPolarity::POSITIVE);
            DLP_NEW_PARAMETERS_ENTRY(TriggerOutputPulseWidth,   "LCR3000_PARAMETERS_OUTPUT_TRIGGER_PULSE_WIDTH_US", unsigned int, 100);
            DLP_NEW_PARAMETERS_ENTRY(TriggerOutputDelay,        "LCR3000_PARAMETERS_OUTPUT_TRIGGER_DELAY_US",       unsigned int, 0);

            DLP_NEW_PARAMETERS_ENTRY(PatternLEDSelect,          "LCR3000_PARAMETERS_PATTERN_LED_SELECT",dlp::Pattern::Color, dlp::Pattern::Color::GREEN);
        };


        class ImageBuffer{
        public:
            ImageBuffer();
            ~ImageBuffer();

            void Clear();
            bool Add(const unsigned int &xCol,      const unsigned int &yRow,
                            const unsigned int &bitplane_offset,
                            const unsigned int &bitdepth,
                            const unsigned int &value );

            ReturnCode SaveImages(std::string basename, std::vector<std::string> &ret_names);
        private:
            std::vector<dlp::Image> images_;
            unsigned int rows_;
            unsigned int columns_;
            unsigned int total_bitplanes_;
            unsigned int image_bitplanes_;
        };



        LCr3000();
        ~LCr3000();

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


        static bool ValidSequenceLength(const unsigned char &bitdepth, const unsigned char &pattern_count);
        static unsigned char ConvertDlpPatternBitdepthToLCr3000Bitdetph(dlp::Pattern::Bitdepth bitdepth);


        ReturnCode LCR_CMD_Open(); //open a connection the the LCR3000
        ReturnCode LCR_CMD_Close(); //close the connection to the LCR3000

        ReturnCode LCR_CMD_GetRevision(LCR_Revision_t Which, char *VersionStr); //get the revision number, return in VersionStr

        ReturnCode LCR_CMD_SetDisplayMode(LCR_DisplayMode_t Mode); //set the display mode
        LCR_DisplayMode_t LCR_CMD_GetDisplayMode(); //get the display mode

        ReturnCode LCR_CMD_SetPowerMode(LCR_PowerMode_t Mode);
        LCR_PowerMode_t LCR_CMD_GetPowerMode();

        ReturnCode LCR_CMD_SetTestPattern(LCR_TestPattern_t TestPtn);
        LCR_TestPattern_t LCR_CMD_GetTestPattern();

        ReturnCode LCR_CMD_SetLEDCurrent(LCR_LEDCurrent_t *LEDSetting);
        ReturnCode LCR_CMD_GetLEDCurrent(LCR_LEDCurrent_t *LEDSetting);

        ReturnCode LCR_CMD_DisplayStaticImage(char const *fileNameWithPath);

        ReturnCode LCR_CMD_DisplayStaticColor(uint32_t Color);

        ReturnCode LCR_CMD_SetDisplaySetting(LCR_DisplaySetting_t const *Setting);
        ReturnCode LCR_CMD_GetDisplaySetting(LCR_DisplaySetting_t *Setting);

        ReturnCode LCR_CMD_SetVideoSetting(LCR_VideoSetting_t const *Setting);
        ReturnCode LCR_CMD_GetVideoSetting(LCR_VideoSetting_t *Setting);

        ReturnCode LCR_CMD_SetVideoMode(LCR_VideoModeSetting_t *Setting);
        ReturnCode LCR_CMD_GetVideoMode(LCR_VideoModeSetting_t *Setting);

        ReturnCode LCR_CMD_SetPatternSeqSetting(LCR_PatternSeqSetting_t const *Setting);
        ReturnCode LCR_CMD_GetPatternSeqSetting(LCR_PatternSeqSetting_t *Setting);

        ReturnCode LCR_CMD_DefinePatternBMP(LCR_PatternCount_t PatternNum, char const *fileNameWithPath);

        ReturnCode LCR_CMD_ReadPattern(LCR_PatternCount_t PatternNum, char *fileName);

        ReturnCode LCR_CMD_StartPatternSeq(uint8_t Start);

        ReturnCode LCR_CMD_AdvancePatternSeq();

        ReturnCode LCR_CMD_GetCamTriggerSetting(LCR_CamTriggerSetting_t *Setting);
        ReturnCode LCR_CMD_SetCamTriggerSetting(LCR_CamTriggerSetting_t *Setting);

        ReturnCode LCR_CMD_SaveSolution(char *SolutionName);
        ReturnCode LCR_CMD_GetSolutionNames(uint8_t *Count, uint8_t *DefaultSolution, char *SolutionName);
        ReturnCode LCR_CMD_ManageSolution(LCR_SolutionCommand_t Cmd, char *SolutionName);

        ReturnCode LCR_CMD_DefineHWPatSequence(LCR_HWPatternSeqDef_t *hwPatSeqDef);

        ReturnCode LCR_CMD_LoadCustomSequence(char *seqBinFileName);
        ReturnCode LCR_CMD_SetupCustomSequencevectors(uint8_t startVector, uint8_t numOfvectors);



    private:

        unsigned int previous_sequence_start_;
        unsigned int previous_sequence_patterns_;
        bool         previous_sequence_repeat_;

        Parameters::LED_CurrentRed      led_current_red_;
        Parameters::LED_CurrentGreen    led_current_green_;
        Parameters::LED_CurrentBlue     led_current_blue_;

        Parameters::ImageFlipShortAxis  image_flip_short_axis_;
        Parameters::ImageFlipLongAxis   image_flip_long_axis_;

        Parameters::TriggerInputType    trigger_input_type_;
        Parameters::TriggerInputDelay   trigger_input_delay_;

        Parameters::TriggerOutputEnable         trigger_output_enable_;
        Parameters::TriggerOutputPolarity       trigger_output_polarity_;
        Parameters::TriggerOutputPulseWidth     trigger_output_pulse_width_;
        Parameters::TriggerOutputDelay          trigger_output_delay_;

        Parameters::PatternLEDSelect    pattern_led_select_;
        LCR_LEDSelect_t lcr3000_led_select_;

        dlp::Pattern::Sequence prepared_sequence_;

        std::string ip; //string that contains the IP of this LCR3000
        unsigned long int port; //port of the LCR3000
        asio::ip::tcp::tcp::socket* LCR_PKT_Socket;

        bool is_connected_;

        asio::io_service io_service;

        //ex globals from legacy program
        uint8_t packetBuffer[HEADER_SIZE + MAX_PACKET_SIZE + CHECKSUM_SIZE];
        uint8_t * const packetData = packetBuffer + HEADER_SIZE;
        uint8_t LCR_PacketType;
        uint8_t contFlag;
        uint8_t recvFlag;
        uint16_t commandId;
        uint16_t dataLength;
        uint16_t parseIndex;



        /* ****** end of ex globals ****/


        //methods from packetizer
        ReturnCode LCR_CMD_PKT_ConnectToLCR();
        ReturnCode LCR_CMD_PKT_DisconnectLCR();

        int LCR_CMD_PKT_CommandInit(LCR_CommandType_t cmdType, uint16_t cmd);
        int LCR_CMD_PKT_PutData(uint8_t *data, unsigned long int size);
        int LCR_CMD_PKT_PutInt(uint32_t value, unsigned int size);
        int LCR_CMD_PKT_GetData(uint8_t *data, unsigned long int size);
        uint32_t LCR_CMD_PKT_GetInt(unsigned int size);
        int LCR_CMD_PKT_PutFile(char const *fileName);
        int LCR_CMD_PKT_GetFile(char const *fileName,uint32_t size);
        int LCR_CMD_PKT_SendCommand();

        int LCR_CMD_PKT_ReceivePacket(bool firstPkt);//, ReturnCode& ret);
        int LCR_CMD_PKT_SendPacket(bool more);

        uint8_t LCR_CMD_PKT_CalcChecksum();

        int LCR_CMD_ReadData(uint8_t *data, uint32_t size);
        int LCR_CMD_WriteData(uint8_t *data, uint32_t size);

        //methods from tcp_client
        asio::ip::tcp::tcp::socket* TCP_Connect(std::string host, unsigned long int  port);
        int TCP_Send(asio::ip::tcp::tcp::socket* sock,  unsigned char *buffer, int length);
        int TCP_Receive(asio::ip::tcp::tcp::socket* sock, unsigned char *buffer, int length);
        int TCP_Disconnect(asio::ip::tcp::tcp::socket* sock);
    };

    namespace String{
    template <> dlp::LCr3000::ImageFlip::Enum ToNumber( const std::string &text, unsigned int base );
    template <> dlp::LCr3000::InputTriggerType::Enum ToNumber( const std::string &text, unsigned int base );
    template <> dlp::LCr3000::OutputTriggerPolarity::Enum ToNumber( const std::string &text, unsigned int base );
    }

    namespace Number{
    template <> std::string ToString<dlp::LCr3000::ImageFlip::Enum>( dlp::LCr3000::ImageFlip::Enum value );
    template <> std::string ToString<dlp::LCr3000::InputTriggerType::Enum>( dlp::LCr3000::InputTriggerType::Enum value );
    template <> std::string ToString<dlp::LCr3000::OutputTriggerPolarity::Enum>( dlp::LCr3000::OutputTriggerPolarity::Enum value );
    }
}
