/** @file   dlpc900_usb.cpp
 *  @brief  Contains methods to communicate with DLP LightCrafter 6500 EVM via USB HID interface
 *  @copyright 2014 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */


#include <cstdio>
#include <cstdlib>

#include <hidapi.h>
#include <dlp_platforms/lightcrafter_6500/dlpc900_usb.hpp>

/***************************************************
*                  GLOBAL VARIABLES
****************************************************/
static hid_device *DeviceHandle;	//Handle to write
//In/Out buffers equal to HID endpoint size + 1
//First byte is for Windows internal use and it is always 0
unsigned char OutputBuffer[USB_MAX_PACKET_SIZE+1];
unsigned char InputBuffer[USB_MAX_PACKET_SIZE+1];


static int USBConnected = 0;      //Boolean true when device is connected

int DLPC900_USB_IsConnected()
{
    return USBConnected;
}

int DLPC900_USB_Init(void)
{
    return hid_init();
}

int DLPC900_USB_Exit(void)
{
    return hid_exit();
}

int DLPC900_USB_Open()
{
    // Open the device using the VID, PID,
    // and optionally the Serial number.
    DeviceHandle = hid_open(MY_VID, MY_PID, NULL);

    if(DeviceHandle == NULL)
    {
        USBConnected = 0;
        return -1;
    }
    USBConnected = 1;
    return 0;
}

int DLPC900_USB_Write()
{
    if(DeviceHandle == NULL)
        return -1;

    return hid_write(DeviceHandle, OutputBuffer, USB_MIN_PACKET_SIZE+1);

}

int DLPC900_USB_Read()
{
    if(DeviceHandle == NULL)
        return -1;

    return hid_read_timeout(DeviceHandle, InputBuffer, USB_MIN_PACKET_SIZE+1, 2000);
}

int DLPC900_USB_Close()
{
    hid_close(DeviceHandle);
    USBConnected = 0;

    return 0;
}

