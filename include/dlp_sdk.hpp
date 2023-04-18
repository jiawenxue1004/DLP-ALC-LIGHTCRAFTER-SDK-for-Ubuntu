/** @file       dlp_sdk.hpp
 *  @brief      Header file that includes all DLP 3D Scanner SDK header files
 *  @copyright  2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

/** @mainpage DLP Advanced Light Control Software Development Kit
 *
 * The DLP® Advanced Light Control Software Development Kit (SDK) is a tool designed to enable faster development with
 * DLP technology platforms. The SDK includes pre-built modules that perform specialized tasks using easy
 * to understand methods. The initial goal of the SDK was to create a scaleable platform for 3D machine vision
 * applications, but is not limited to that specific application. The SDK framework is also suitable for 3D printing,
 * spectroscopy, and any other application that requires the use of DLP technology in a simple to use interface.
 *
 *
 * For help, please refer to the <a href="http://e2e.ti.com/support/dlp__mems_micro-electro-mechanical_systems/">DLP and MEMS TI E2E Community Support Forums</a>
 *
 */


#ifndef DLP_STRUCTURED_LIGHT_SDK_HPP
#define DLP_STRUCTURED_LIGHT_SDK_HPP

// Include SDK headers
#include <common/returncode.hpp>
#include <common/debug.hpp>
#include <common/other.hpp>
#include <common/image/image.hpp>
#include <common/parameters.hpp>
#include <common/capture/capture.hpp>
#include <common/pattern/pattern.hpp>
#include <common/point_cloud/point_cloud.hpp>
#include <common/module.hpp>

#include <camera/camera.hpp>
#include <camera/opencv_cam/opencv_cam.hpp>
#include <camera/pg_flycap2/pg_flycap2_c.hpp>

#include <structured_light/structured_light.hpp>
#include <structured_light/gray_code/gray_code.hpp>
#include <structured_light/three_phase/three_phase.hpp>

#include <dlp_platforms/dlp_platform.hpp>
// #include <dlp_platforms/lightcrafter_3000/lcr3000.hpp>
#include <dlp_platforms/lightcrafter_4500/lcr4500.hpp>
// #include <dlp_platforms/lightcrafter_6500/lcr6500.hpp>

#include <calibration/calibration.hpp>
#include <geometry/geometry.hpp>

//new added 
#include <dlp_platforms/lightcrafter_4500/common.hpp>
#include <dlp_platforms/lightcrafter_4500/dlpc350_api.hpp>
#include <dlp_platforms/lightcrafter_4500/dlpc350_usb.hpp>


/** @defgroup   group_Common Common
 *  @brief      Contains common functions relating to strings, numbers, and time
 */


#endif  //#ifndef DLP_STRUCTURED_LIGHT_SDK_HPP


