/*
 * Common.h
 *
 * This module provides flash related definitions.
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
*/

#ifndef __FLASHDEVICE_H
#define __FLASHDEVICE_H

#include <string>

#define FLASH_FILE_VERSION_LONG_ID 2 

typedef struct _FlashDevice
{
    std::string             Mfg;             // Company name.
    unsigned short          Mfg_ID;          // Manufacturer ID stored in part.
    unsigned long long int  LMfg_ID;         // Long Manufacturer ID stored in part.

    std::string             Dev;             // Part number from data sheet.
    unsigned short          Dev_ID;          // Device ID stored in part.
    unsigned long long int  LDev_ID;         // Long Device ID stored in part.

    unsigned int            Size_MBit;       // 4, 8, or 16 MBit.
    unsigned char           Type;            // A, B or C algothrim (0, 1, 2)
    unsigned int            numSectors;
    unsigned int            SectorArr[256];       // List of sector addresses.
}FlashDevice;

#endif  //__FLASHDEVICE_H
