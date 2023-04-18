/*
 * API.h
 *
 * This module provides C callable APIs for each of the command supported by LightCrafter4500 platform and detailed in the programmer's guide.
 *
 * Copyright (C) {2013} Texas Instruments Incorporated - http://www.ti.com/ 
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#ifndef DLPC900_API_H
#define DLPC900_API_H

/* Bit masks. */
#define BIT0        0x01
#define BIT1        0x02
#define BIT2        0x04
#define BIT3        0x08
#define BIT4        0x10
#define BIT5        0x20
#define BIT6        0x40
#define BIT7        0x80
#define BIT8      0x0100
#define BIT9      0x0200
#define BIT10     0x0400
#define BIT11     0x0800
#define BIT12     0x1000
#define BIT13     0x2000
#define BIT14     0x4000
#define BIT15     0x8000
#define BIT16 0x00010000
#define BIT17 0x00020000
#define BIT18 0x00040000
#define BIT19 0x00080000
#define BIT20 0x00100000
#define BIT21 0x00200000
#define BIT22 0x00400000
#define BIT23 0x00800000
#define BIT24 0x01000000
#define BIT25 0x02000000
#define BIT26 0x04000000
#define BIT27 0x08000000
#define BIT28 0x10000000
#define BIT29 0x20000000
#define BIT30 0x40000000
#define BIT31 0x80000000

#define STAT_BIT_FLASH_BUSY         	BIT3
#define HID_MESSAGE_MAX_SIZE        	 512
#define MAX_PAT_LUT_ENTRIES         	 512
#define MAX_IMAGE_ENTRIES_ON_THE_FLY	  18
#define MAX_IMAGE_ENTRIES_STORED    	 256

typedef struct _hidmessageStruct
{
    struct _hidhead
    {
        struct _packetcontrolStruct
        {
            unsigned char dest		:3; /* 0 - ProjCtrl; 1 - RFC; 7 - Debugmsg */
            unsigned char reserved	:2;
            unsigned char nack		:1; /* Command Handler Error */
            unsigned char reply	:1; /* Host wants a reply from device */
            unsigned char rw		:1; /* Write = 0; Read = 1 */
        }flags;
        unsigned char seq;
        unsigned short length;
    }head;
    union
    {
        unsigned short cmd;
        unsigned char data[HID_MESSAGE_MAX_SIZE];
    }text;
}hidMessageStruct;

typedef struct _readCmdData
{
    unsigned char I2CCMD;
    unsigned char CMD2;
    unsigned char CMD3;
    bool batchUpdateEnable;
    unsigned short len;
    char *name;
}CmdFormat;

typedef struct _rectangle
{
    unsigned short firstPixel;
    unsigned short firstLine;
    unsigned short pixelsPerLine;
    unsigned short linesPerFrame;
}rectangle;

typedef enum
{   
    SOURCE_SEL,
    PIXEL_FORMAT,
    CLK_SEL,
    CHANNEL_SWAP,
    FPD_MODE,
    POWER_CONTROL,
    FLIP_LONG,
    FLIP_SHORT,
    TPG_SEL,
    PWM_INVERT,
    LED_ENABLE,
    GET_VERSION,
    SW_RESET,
    STATUS_HW,
    STATUS_SYS,
    STATUS_MAIN,
    PWM_ENABLE,
    PWM_SETUP,
    PWM_CAPTURE_CONFIG,
    GPIO_CONFIG,
    LED_CURRENT,
    DISP_CONFIG,
    DISP_MODE,
    TRIG_OUT1_CTL,
    TRIG_OUT2_CTL,
    RED_LED_ENABLE_DLY,
    GREEN_LED_ENABLE_DLY,
    BLUE_LED_ENABLE_DLY,
    PAT_START_STOP,
    TRIG_IN1_CTL,
    TRIG_IN2_CTL,
    INVERT_DATA,
    PAT_CONFIG,
    MBOX_ADDRESS,
    MBOX_CONTROL,
    MBOX_DATA,
    SPLASH_LOAD,
    GPCLK_CONFIG,
    TPG_COLOR,
    PWM_CAPTURE_READ,
    I2C_PASSTHRU,
    PATMEM_LOAD_INIT_MASTER,
    PATMEM_LOAD_DATA_MASTER,
    PATMEM_LOAD_INIT_SLAVE,
    PATMEM_LOAD_DATA_SLAVE,
    BATCHFILE_NAME,
    BATCHFILE_EXECUTE,
    DELAY,
    DEBUG,
    I2C_CONFIG,
    CURTAIN_COLOR,
    VIDEO_CONT_SEL,
    READ_ERROR_CODE,
    READ_ERROR_MSG,
    BL_STATUS,
    BL_SPL_MODE,
    BL_GET_MANID,
    BL_GET_DEVID,
    BL_GET_CHKSUM,
    BL_SET_SECTADDR,
    BL_SECT_ERASE,
    BL_SET_DNLDSIZE,
    BL_DNLD_DATA,
    BL_FLASH_TYPE,
    BL_CALC_CHKSUM,
    BL_PROG_MODE,
    BL_MASTER_SLAVE,
}DLPC900_CMD;

int DLPC900_SetInputSource(unsigned int source, unsigned int portWidth);
int DLPC900_GetInputSource(unsigned int *pSource, unsigned int *portWidth);
int DLPC900_SetPixelFormat(unsigned int format);
int DLPC900_GetPixelFormat(unsigned int *pFormat);
int DLPC900_SetPortClock(unsigned int clock);
int DLPC900_GetPortClock(unsigned int *pClock);
int DLPC900_SetDataChannelSwap(unsigned int port, unsigned int swap);
int DLPC900_GetDataChannelSwap(unsigned int *pPort, unsigned int *pSwap);
int DLPC900_SetFPD_Mode_Field(unsigned int PixelMappingMode, bool SwapPolarity, unsigned int FieldSignalSelect);
int DLPC900_GetFPD_Mode_Field(unsigned int *pPixelMappingMode, bool *pSwapPolarity, unsigned int *pFieldSignalSelect);
int DLPC900_SetPowerMode(unsigned char);
int DLPC900_GetPowerMode(bool *Standby);
int DLPC900_SetLongAxisImageFlip(bool);
bool DLPC900_GetLongAxisImageFlip();
int DLPC900_SetShortAxisImageFlip(bool);
bool DLPC900_GetShortAxisImageFlip();
int DLPC900_SetTPGSelect(unsigned int pattern);
int DLPC900_GetTPGSelect(unsigned int *pPattern);
int DLPC900_SetLEDPWMInvert(bool invert);
int DLPC900_GetLEDPWMInvert(bool *inverted);
int DLPC900_SetLedEnables(bool SeqCtrl, bool Red, bool Green, bool Blue);
int DLPC900_GetLedEnables(bool *pSeqCtrl, bool *pRed, bool *pGreen, bool *pBlue);
int DLPC900_GetVersion(unsigned int *pApp_ver, unsigned int *pAPI_ver, unsigned int *pSWConfig_ver, unsigned int *pSeqConfig_ver);
int DLPC900_SoftwareReset(void);
int DLPC900_GetStatus(unsigned char *pHWStatus, unsigned char *pSysStatus, unsigned char *pMainStatus);
int DLPC900_SetPWMEnable(unsigned int channel, bool Enable);
int DLPC900_GetPWMEnable(unsigned int channel, bool *pEnable);
int DLPC900_SetPWMConfig(unsigned int channel, unsigned int pulsePeriod, unsigned int dutyCycle);
int DLPC900_GetPWMConfig(unsigned int channel, unsigned int *pPulsePeriod, unsigned int *pDutyCycle);
int DLPC900_SetPWMCaptureConfig(unsigned int channel, bool enable, unsigned int sampleRate);
int DLPC900_GetPWMCaptureConfig(unsigned int channel, bool *pEnabled, unsigned int *pSampleRate);
int DLPC900_SetGPIOConfig(unsigned int pinNum, bool dirOutput, bool outTypeOpenDrain, bool pinState);
int DLPC900_GetGPIOConfig(unsigned int pinNum, bool *pDirOutput, bool *pOutTypeOpenDrain, bool *pState);
int DLPC900_GetLedCurrents(unsigned char *pRed, unsigned char *pGreen, unsigned char *pBlue);
int DLPC900_SetLedCurrents(unsigned char RedCurrent, unsigned char GreenCurrent, unsigned char BlueCurrent);
int DLPC900_SetDisplay(rectangle croppedArea, rectangle displayArea);
int DLPC900_GetDisplay(rectangle *pCroppedArea, rectangle *pDisplayArea);
int DLPC900_MemRead(unsigned int addr, unsigned int *readWord);
int DLPC900_MemWrite(unsigned int addr, unsigned int data);
int DLPC900_ValidatePatLutData(unsigned int *pStatus);
int DLPC900_SetPatternDisplayMode(bool external);
int DLPC900_GetPatternDisplayMode(bool *external);
int DLPC900_SetTrigOutConfig(unsigned int trigOutNum, bool invert, short rising, short falling);
int DLPC900_GetTrigOutConfig(unsigned int trigOutNum, bool *pInvert,short *pRising, short *pFalling);
int DLPC900_SetRedLEDStrobeDelay(short rising, short falling);
int DLPC900_SetGreenLEDStrobeDelay(short rising, short falling);
int DLPC900_SetBlueLEDStrobeDelay(short rising, short falling);
int DLPC900_GetRedLEDStrobeDelay(short *, short *);
int DLPC900_GetGreenLEDStrobeDelay(short *, short *);
int DLPC900_GetBlueLEDStrobeDelay(short *, short *);
int DLPC900_EnterProgrammingMode(void);
int DLPC900_ExitProgrammingMode(void);
int DLPC900_GetProgrammingMode(bool *ProgMode);
int DLPC900_EnableMasterSlave(void);
int DLPC900_DisableMasterUpdate(void);
int DLPC900_DisableSlaveUpdate(void);
int DLPC900_GetFlashManID(unsigned short *manID);
int DLPC900_GetFlashDevID(unsigned long long *devID);
int DLPC900_GetBLStatus(unsigned char *BL_Status);
int DLPC900_SetFlashAddr(unsigned int Addr);
int DLPC900_FlashSectorErase(void);
int DLPC900_SetDownloadSize(unsigned int dataLen);
int DLPC900_DownloadData(unsigned char *pByteArray, unsigned int dataLen);
void DLPC900_WaitForFlashReady(void);
int DLPC900_SetFlashType(unsigned char Type);
int DLPC900_CalculateFlashChecksum(void);
int DLPC900_GetFlashChecksum(unsigned int*checksum);
int DLPC900_SetMode(int SLmode);
int DLPC900_GetMode(int *pMode);
int DLPC900_LoadSplash(unsigned int index);
int DLPC900_GetSplashIndex(unsigned int *pIndex);
int DLPC900_SetTPGColor(unsigned short redFG, unsigned short greenFG, unsigned short blueFG, unsigned short redBG, unsigned short greenBG, unsigned short blueBG);
int DLPC900_GetTPGColor(unsigned short *pRedFG, unsigned short *pGreenFG, unsigned short *pBlueFG, unsigned short *pRedBG, unsigned short *pGreenBG, unsigned short *pBlueBG);
int DLPC900_ClearPatLut(void);
int DLPC900_AddToPatLut(int patNum, int ExpUs, bool ClearPat, int BitDepth, int LEDSelect, bool WaitForTrigger, int DarkTime, bool TrigOut2, int SplashIndex, int BitIndex);
int DLPC900_SendPatLut(void);
int DLPC900_SendSplashLut(unsigned char *lutEntries, unsigned int numEntries);
int DLPC900_GetPatLut(int numEntries);
int DLPC900_GetSplashLut(unsigned char *pLut, int numEntries);
int DLPC900_SetPatternTriggerMode(bool);
int DLPC900_GetPatternTriggerMode(bool *);
int DLPC900_PatternDisplay(int Action);
int DLPC900_SetPatternConfig(unsigned int numLutEntries, unsigned int repeat);
int DLPC900_GetPatternConfig(unsigned int *pNumLutEntries, bool *pRepeat, unsigned int *pNumPatsForTrigOut2, unsigned int *pNumSplash);
int DLPC900_SetTrigIn1Config(bool invert, unsigned int trigDelay);
int DLPC900_GetTrigIn1Config(bool *pInvert, unsigned int *pTrigDelay);
int DLPC900_SetTrigIn1Delay(unsigned int Delay);
int DLPC900_GetTrigIn1Delay(unsigned int *pDelay);
int DLPC900_SetTrigIn2Config(bool invert);
int DLPC900_GetTrigIn2Config(bool *pInvert);
int DLPC900_SetInvertData(bool invert);
int DLPC900_GetInvertData(bool *pInvert);
int DLPC900_PWMCaptureRead(unsigned int channel, unsigned int *pLowPeriod, unsigned int *pHighPeriod);
int DLPC900_SetGeneralPurposeClockOutFreq(unsigned int clkId, bool enable, unsigned int clkDivider);
int DLPC900_GetGeneralPurposeClockOutFreq(unsigned int clkId, bool *pEnabled, unsigned int *pClkDivider);
int DLPC900_MeasureSplashLoadTiming(unsigned int startIndex, unsigned int numSplash);
int DLPC900_ReadSplashLoadTiming(unsigned int *pTimingData);
int DLPC900_SetI2CPassThrough(unsigned int port, unsigned int addm, unsigned int clk, unsigned int devadd, unsigned char* wdata, unsigned int nwbytes);
int DLPC900_GetI2CPassThrough(unsigned int port, unsigned int addm, unsigned int clk, unsigned int devadd, unsigned char* wdata, unsigned int nwbytes, unsigned int nrbytes, unsigned char* rdata);
int DLPC900_WriteI2CPassThrough(unsigned int port, unsigned int devadd, unsigned char* wdata, unsigned int nwbytes);
int DLPC900_ReadI2CPassThrough(unsigned int port, unsigned int devadd, unsigned char* wdata, unsigned int nwbytes, unsigned int nrbytes, unsigned char* rdata);
int DLPC900_I2CConfigure(unsigned int port, unsigned int addm, unsigned int clk);
int DLPC900_SetPixelMode(unsigned int index);
int DLPC900_GetPixelMode(unsigned int *index);
int DLPC900_pattenMemLoad(bool master, unsigned char *pByteArray, int size);
int DLPC900_InitPatternMemLoad(bool master, unsigned short imageNum, unsigned int size);
int DLPC900_getBatchFileName(unsigned char id, char *batchFileName);
int DLPC900_executeBatchFile(unsigned char id);
int DLPC900_enableDebug();
int DLPC900_GetPortConfig(unsigned int *pDataPort,unsigned int *pPixelClock,unsigned int *pDataEnable,unsigned int *pSyncSelect);
int DLPC900_SetPortConfig(unsigned int dataPort,unsigned int pixelClock,unsigned int dataEnable,unsigned int syncSelect);
int DLPC900_executeRawCommand(unsigned char *rawCommand, int count);
int API_getI2CCommand(char *command, unsigned char *i2cCommand);
int API_getUSBCommand(char *command, unsigned char *usbCommand);
int API_getCommandLength(char *command, int *len);
int API_getCommandName(unsigned char i2cCommand, char **command);
int API_getBatchFilePatternDetails(unsigned char *batchBuffer, int size, unsigned short *patternImageList, int *patternImageCount);
int API_changeImgNoinBatchFile(unsigned char *buffer, int size, int curId, int changeId);
int DLPC900_SetCurtainColor(unsigned int red,unsigned int green, unsigned int blue);
int DLPC900_GetCurtainColor(unsigned int *pRed, unsigned int *pGreen, unsigned int *pBlue);
int DLPC900_SetIT6535PowerMode(unsigned int powerMode);
int DLPC900_GetIT6535PowerMode(unsigned int *pPowerMode);
int DLPC900_ReadErrorCode(unsigned int *pCode);
int DLPC900_ReadErrorString(char *errStr);

#endif // API_H
