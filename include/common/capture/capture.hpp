/*! @file       capture.hpp
 *  @ingroup    group_Common
 *  @brief      Defines \ref dlp::Capture and \ref dlp::Capture::Sequence classes
 *  @copyright  2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#ifndef DLP_SDK_CAPTURE_HPP
#define DLP_SDK_CAPTURE_HPP

// DLP Structured Light SDK header files
#include <common/debug.hpp>                     // Adds dlp::Debug
#include <common/other.hpp>                     // Adds dlp::CmdLine, Time, File, String, Number namespaces
#include <common/returncode.hpp>                // Adds dlp::ReturnCode
#include <common/image/image.hpp>               // Adds dlp::Image
#include <common/parameters.hpp>                // Adds dlp::Parameter

// C++ standard header files
#include <string>                               // Adds std::string
#include <vector>                               // Adds std::vector

#define CAPTURE_TYPE_INVALID                    "CAPTURE_TYPE_INVALID"
#define CAPTURE_SEQUENCE_EMPTY                  "CAPTURE_SEQUENCE_EMPTY"
#define CAPTURE_SEQUENCE_TOO_LONG               "CAPTURE_SEQUENCE_TOO_LONG"
#define CAPTURE_SEQUENCE_INDEX_OUT_OF_RANGE     "CAPTURE_SEQUENCE_INDEX_OUT_OF_RANGE"
#define CAPTURE_SEQUENCE_TYPES_NOT_EQUAL        "CAPTURE_SEQUENCE_TYPES_NOT_EQUAL"


/** @brief  Contains all DLP SDK classes, functions, etc. */
namespace dlp{


/** @class      Capture
 *  @ingroup    group_Common
 *  @brief      Container class for image data or file name for transfer between \ref dlp::Calibration,
 *              \ref dlp::Camera, and \ref dlp::StructuredLight classes.
 */
class Capture{
public:


    /** @class DataType
     *  @brief Dictates if the \ref dlp::Capture contains image data or an image filename
     */
    enum class DataType{
        IMAGE_FILE,     /**< Capture data stored in \ref dlp::Image with \ref image_data */
        IMAGE_DATA,     /**< Capture data stored in image file with \ref image_file      */
        INVALID
    };

    /** @class      Sequence
     *  @ingroup    group_Common
     *  @brief      Container class used to group multiple \ref dlp::Capture objects
     */
    class Sequence{
    public:

        Sequence();
        ~Sequence();
        Sequence(const Capture  &capture);
        Sequence(const Sequence &capture_seq);
        Sequence& operator=(const Sequence& capture_seq);

        unsigned int GetCount() const;
        void Clear();

        ReturnCode Add( const Capture  &new_capture);
        ReturnCode Add( const Sequence &sequence);
        ReturnCode Get(    const unsigned int &index, Capture* ret_capture) const;
        ReturnCode Set(    const unsigned int &index, Capture &arg_capture);
        ReturnCode Remove( const unsigned int &index);

        bool EqualDataTypes() const;

        Parameters parameters;  /*!< Nonrequired member to store any addition information required about the \ref dlp::Capture::Sequence */
    private:
        std::vector<Capture> captures_;
    };


    Capture();
    ~Capture();

    // Capture Information
    int camera_id;          /*!< Nonrequired member to notate which camera created the Capture */
    int pattern_id;         /*!< Nonrequired member to notate which projected pattern image the Capture contains */

    // Capture Data
    DataType    data_type;     /*!< \b Required member to notate if Capture contains image data or an image filename */
    dlp::Image  image_data;     /*!< \ref dlp::Image member. Empty when Capture instance is constructed. */
    std::string image_file;     /*!< \ref std::string to store an image filename. Empty when Capture instance is constructed. */
};

namespace Number{
template <> std::string ToString<dlp::Capture::DataType>( dlp::Capture::DataType data_type );
}

namespace String{
template <> dlp::Capture::DataType ToNumber( const std::string &text, unsigned int base );
}

}


#endif // DLP_SDK_CAPTURE_HPP
