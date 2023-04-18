/**
 * @file    capture.cpp
 * @ingroup group_Common
 * @brief   Contains \ref dlp::Capture methods
 * @copyright 2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

// DLP Structured Light SDK header files
#include <common/debug.hpp>                     // Adds dlp::Debug
#include <common/other.hpp>                     // Adds dlp::CmdLine, Time, File, String, Number namespaces
#include <common/returncode.hpp>                // Adds dlp::ReturnCode
#include <common/image/image.hpp>               // Adds dlp::Image
#include <common/parameters.hpp>                // Adds dlp::Parameter
#include <common/capture/capture.hpp>           // Adds dlp::Capture

// C++ standard header files
#include <string>                               // Adds std::string
#include <vector>                               // Adds std::vector

namespace dlp{

/** @brief Constructs empty object */
Capture::Capture(){
    this->camera_id   = 0;
    this->pattern_id  = 0;
    this->data_type   = DataType::INVALID;
    this->image_file  = "";
    this->image_data.Clear();
}

/** @brief Destructs object and deallocates memory */
Capture::~Capture(){
    this->image_data.Clear();
}

namespace Number{
template <> std::string ToString<dlp::Capture::DataType>( dlp::Capture::DataType data_type ){
    switch(data_type){
    case dlp::Capture::DataType::INVALID:       return "INVALID";
    case dlp::Capture::DataType::IMAGE_FILE:    return "IMAGE_FILE";
    case dlp::Capture::DataType::IMAGE_DATA:    return "IMAGE_DATA";
    }
    return "INVALID";
}
}

namespace String{
template <> dlp::Capture::DataType ToNumber(const std::string &text , unsigned int base){
    // Ignore base variable
    if (text.compare("IMAGE_FILE") == 0){
        return dlp::Capture::DataType::IMAGE_FILE;
    }
    else if (text.compare("IMAGE_DATA") == 0){
        return dlp::Capture::DataType::IMAGE_DATA;
    }
    else{
        return dlp::Capture::DataType::INVALID;
    }
}
}

}

