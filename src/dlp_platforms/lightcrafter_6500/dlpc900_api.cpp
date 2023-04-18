/** @file   dlpc350_api.cpp
 *  @brief  Contains methods to control the DLPC350
 *  @copyright 2014 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#include <stdlib.h>
#include <string>

#include <dlp_platforms/lightcrafter_6500/common.hpp>
#include <dlp_platforms/lightcrafter_6500/dlpc900_api.hpp>
#include <dlp_platforms/lightcrafter_6500/dlpc900_usb.hpp>

extern unsigned char OutputBuffer[];
extern unsigned char InputBuffer[];

CmdFormat CmdList[] =
{
    {   0x00, 0x1A,  0x00,  true, 0x01, "SOURCE_SEL" },      //SOURCE_SEL,
    {   0x02, 0x1A,  0x02,  true, 0x01, "PIXEL_FORMAT"   },      //PIXEL_FORMAT,
    {   0x03, 0x1A,  0x03,  true, 0x01, "CLK_SEL"   },      //CLK_SEL,
    {   0x04, 0x1A,  0x37,  true, 0x01, "CHANNEL_SWAP"   },      //CHANNEL_SWAP,
    {   0x05, 0x1A,  0x04,  true, 0x01, "FPD_MODE"   },      //FPD_MODE,
    {   0x07, 0x02,  0x00,  true, 0x01, "POWER_CONTROL"  },      //POWER_CONTROL,
    {   0x08, 0x10,  0x08,  true, 0x01 , "FLIP_LONG"  },      //FLIP_LONG,
    {   0x09, 0x10,  0x09,  true, 0x01, "FLIP_SHORT"   },      //FLIP_SHORT,
    {   0x0A, 0x12,  0x03,  true, 0x01, "TPG_SEL"   },      //TPG_SEL,
    {   0x0B, 0x1A,  0x05,  true, 0x01, "PWM_INVERT"   },      //PWM_INVERT,
    {   0x10, 0x1A,  0x07,  true, 0x01, "LED_ENABLE"   },      //LED_ENABLE,
    {   0x11, 0x02,  0x05,  false, 0x00, "GET_VERSION"   },      //GET_VERSION,
    {   0x13, 0x08,  0x02,  true, 0x01, "SW_RESET"   },      //SW_RESET,
    {   0x20, 0x1A,  0x0A,  false, 0x00, "STATUS_HW"   },      //STATUS_HW,
    {   0x21, 0x1A,  0x0B,  false, 0x00, "STATUS_SYS"   },      //STATUS_SYS,
    {   0x22, 0x1A,  0x0C,  false, 0x00, "STATUS_MAIN"   },      //STATUS_MAIN,
    {   0x40, 0x1A,  0x10,  true, 0x01, "PWM_ENABLE"   },      //PWM_ENABLE,
    {   0x41, 0x1A,  0x11,  true, 0x06, "PWM_SETUP"   },      //PWM_SETUP,
    {   0x43, 0x1A,  0x12,  true, 0x05, "PWM_CAPTURE_CONFIG"   },      //PWM_CAPTURE_CONFIG,
    {   0x44, 0x1A,  0x38,  true, 0x02, "GPIO_CONFIG"   },      //GPIO_CONFIG,
    {   0x4B, 0x0B,  0x01,  true, 0x03, "LED_CURRENT"   },      //LED_CURRENT,
    {   0x7E, 0x10,  0x00,  true, 0x10 , "DISP_CONFIG"  },      //DISP_CONFIG,
    {   0x69, 0x1A,  0x1B,  true, 0x01, "DISP_MODE"   },      //DISP_MODE,
    {   0x6A, 0x1A,  0x1D,  true, 0x05, "TRIG_OUT1_CTL"   },      //TRIG_OUT1_CTL,
    {   0x6B, 0x1A,  0x1E,  true, 0x05, "TRIG_OUT2_CTL"   },      //TRIG_OUT2_CTL,
    {   0x6C, 0x1A,  0x1F,  true, 0x04, "RED_LED_ENABLE_DLY"   },  //RED_LED_ENABLE_DLY,
    {   0x6D, 0x1A,  0x20,  true, 0x04, "GREEN_LED_ENABLE_DLY"   }, //GREEN_LED_ENABLE_DLY,
    {   0x6E, 0x1A,  0x21,  true, 0x04, "BLUE_LED_ENABLE_DLY"   }, //BLUE_LED_ENABLE_DLY,
    {   0x65, 0x1A,  0x24,  true, 0x01, "PAT_START_STOP"   },      //PAT_START_STOP,
    {   0x79, 0x1A,  0x35,  true, 0x03, "TRIG_IN1_CTL"},      //TRIG_IN1_CTL,
    {   0x7A, 0x1A,  0x36,  true, 0x01, "TRIG_IN2_CTL"},      //TRIG_IN2_CTL,
    {   0x74, 0x1A,  0x30,  true, 0x01,  "INVERT_DATA"  },      //INVERT_DATA,
    {   0x75, 0x1A,  0x31,  true, 0x06, "PAT_CONFIG"   },      //PAT_CONFIG,
    {   0x76, 0x1A,  0x32,  true, 0x02, "MBOX_ADDRESS"   },      //MBOX_ADDRESS,
    {   0x77, 0x1A,  0x33,  true, 0x01, "MBOX_CONTROL"   },      //MBOX_CONTROL,
    {   0x78, 0x1A,  0x34,  true, 0x0C, "MBOX_DATA"    },      //MBOX_DATA,
    {   0x7F, 0x1A,  0x39,  true, 0x01, "SPLASH_LOAD"   },      //SPLASH_LOAD,
    {   0x48, 0x08,  0x07,  true, 0x03, "GPCLK_CONFIG"   },      //GPCLK_CONFIG,
    {   0x1A, 0x12,  0x04,  true, 0x0C, "TPG_COLOR"   },      //TPG_COLOR,
    {   0x4E, 0x1A,  0x13,  true, 0x05, "PWM_CAPTURE_READ"   },     //PWM_CAPTURE_READ,
    {   0x4F, 0x1A,  0x4F,  true, 0x05, "I2C_PASSTHRU"},     //I2C_PASSTHRU,
    {	0x2A, 0x1A,  0x2A,  false, 0x6, "PATMEM_LOAD_INIT_MASTER"},//PATMEM_LOAD_INIT_MASTER
    {   0x2B, 0x1A,  0x2B,  false, 0x00, "PATMEM_LOAD_DATA_MASTER"},//PATMEM_LOAD_DATA_MASTER
    {   0x2C, 0x1A,  0x2C,  false, 0x6, "PATMEM_LOAD_INIT_SLAVE"},//PATMEM_LOAD_INIT_SLAVE
    {   0x2D, 0x1A,  0x2D,  false, 0x00, "PATMEM_LOAD_DATA_SLAVE" },//PATMEM_LOAD_DATA_SLAVE
    {   0x3A, 0x1A,  0x14,  false, 0x1, "BATCHFILE_NAME"}, //BATCHFILE_NAME
    {   0x3B, 0x1A,  0x15,  false, 0x1,  "BATCHFILE_EXECUTE"}, // BATCHFILE_EXECUTE
    {   0x3C, 0x1A,  0x16,  false, 0x4,  "DELAY"}, //DELAY
    {   0x00, 0x1A,  0x5B,  false, 0x05, "DEBUG"}, //DEBUG
    {   0x45, 0x1A,  0x4E,  true, 0x05, "I2C_CONFIG"}, //I2C_CONFIG
    {   0x06, 0x11,  0x00,  true, 0x06, "CURTAIN_COLOR"},//CURTAIN_COLOR
    {   0x0C, 0x1A,  0x01,  true, 0x01, "VIDEO_CONT_SEL"},//VIDEO_CONT_SEL(IT6535_POWER_MODE)
    {   0x32, 0x01,  0x00,  false, 0x01, "READ_ERROR_CODE"},//READ_ERROR_CODE
    {   0x33, 0x01,  0x01,  false, 0x80, "READ_ERROR_MSG"},//READ_ERROR_MSG
    {   0x01, 0x00,  0x00,  false, 0x00, "BL_STATUS"   },     //BL_STATUS
    {   0x23, 0x00,  0x23,  false, 0x01, "BL_SPL_MODE"   },     //BL_SPL_MODE
    {   0x0C, 0x00,  0x15,  false, 0x01, "BL_GET_MANID"   },     //BL_GET_MANID,
    {   0x0D, 0x00,  0x15,  false, 0x01, "BL_GET_DEVID"   },     //BL_GET_DEVID,
    {   0x00, 0x00,  0x15,  false, 0x01, "BL_GET_CHKSUM"   },     //BL_GET_CHKSUM,
    {   0x29, 0x00,  0x29,  false, 0x04, "BL_SET_SECTADDR"   },     //BL_SET_SECTADDR,
    {   0x28, 0x00,  0x28,  false, 0x00, "BL_SECT_ERASE"   },     //BL_SECT_ERASE,
    {   0x2C, 0x00,  0x2C,  false, 0x04, "BL_SET_DNLDSIZE"   },     //BL_SET_DNLDSIZE,
    {   0x25, 0x00,  0x25,  false, 0x00, "BL_DNLD_DATA"   },     //BL_DNLD_DATA,
    {   0x2F, 0x00,  0x2F,  false, 0x01, "BL_FLASH_TYPE"   },     //BL_FLASH_TYPE,
    {   0x26, 0x00,  0x26,  false, 0x00, "BL_CALC_CHKSUM"   },     //BL_CALC_CHKSUM,
    {   0x30, 0x00,  0x30,  false, 0x01, "BL_PROG_MODE"   },     //BL_PROG_MODE,
    {   0x31, 0x00,  0x31,  false, 0x01, "BL_MASTER_SLAVE"   },     // BL_MASTER_SLAVE,
    {   0x00, 0x00, 0x00, false, 0x0, "END"},	// Keep this to identify end of command list.
};

static unsigned char seqNum=0;
static unsigned char PatLut[12][512];
static unsigned int PatLutIndex = 0;

int DLPC900_Write(bool ackRequired)
{
    if (!ackRequired)
        return DLPC900_USB_Write();

    int ret = DLPC900_USB_Write();
    if (ret > 0)
    {
        DLPC900_USB_Read();
        hidMessageStruct *pMsg = (hidMessageStruct *)InputBuffer;
        if (pMsg->head.flags.nack)
            return -2;

        return ret;
    }
    return -1;
}

int DLPC900_Read()
/**
 * This function is private to this file. This function is called to write the read control command and then read back 64 bytes over USB
 * to InputBuffer.
 *
 * @return  number of bytes read
 *          -2 = nack from target
 *          -1 = error reading
 *
 */
{
    int ret_val;
    hidMessageStruct *pMsg = (hidMessageStruct *)InputBuffer;
    if(DLPC900_USB_Write() > 0)
    {
        ret_val =  DLPC900_USB_Read();

        if((pMsg->head.flags.nack == 1) || (pMsg->head.length == 0))
            return -2;
        else
            return ret_val;
    }
    return -1;
}

int DLPC900_ContinueRead()
{
    return DLPC900_USB_Read();
}

int DLPC900_SendMsg(hidMessageStruct *pMsg)
/**
 * This function is private to this file. This function is called to send a message over USB; in chunks of 64 bytes.
 *
 * @return  number of bytes sent
 *          -1 = FAIL
 *
 */
{
    int maxDataSize = USB_MAX_PACKET_SIZE-sizeof(pMsg->head);
    int dataBytesSent = MIN(pMsg->head.length, maxDataSize);    //Send all data or max possible
    bool ackRequired = true;

    /* Disable Read back for BL_DNLD_DATA, BL_CALC_CHKSUM, PATMEM_LOAD_DATAcommands */
    if ((pMsg->text.cmd == 0x25) || (pMsg->text.cmd == 0x26) ||  (pMsg->text.cmd == 0x1A2B) || (pMsg->text.cmd == 0x1A2D))
    {
        ackRequired = false;
        pMsg->head.flags.reply = 0;
    }
    
    OutputBuffer[0]=0; // First byte is the report number
    memcpy(&OutputBuffer[1], pMsg, (sizeof(pMsg->head) + dataBytesSent));


    if(DLPC900_Write(ackRequired) < 0)
        return -1;

    //dataBytesSent = maxDataSize;

    while(dataBytesSent < pMsg->head.length)
    {
        memcpy(&OutputBuffer[1], &pMsg->text.data[dataBytesSent], USB_MAX_PACKET_SIZE);
        if(DLPC900_Write(ackRequired) < 0)
            return -1;
        dataBytesSent += USB_MAX_PACKET_SIZE;
    }

    return dataBytesSent+sizeof(pMsg->head);
}

int DLPC900_PrepReadCmd(DLPC900_CMD cmd)
/**
 * This function is private to this file. Prepares the read-control command packet for the given command code and copies it to OutputBuffer.
 *
 * @param   cmd  - I - USB command code.
 *
 * @return  0 = PASS
 *          -1 = FAIL
 *
 */
{
    hidMessageStruct msg;

    msg.head.flags.rw = 1; //Read
    msg.head.flags.reply = 1; //Host wants a reply from device
    msg.head.flags.dest = 0; //Projector Control Endpoint
    msg.head.flags.reserved = 0;
    msg.head.flags.nack = 0;
    msg.head.seq = 0;

    msg.text.cmd = (CmdList[cmd].CMD2 << 8) | CmdList[cmd].CMD3;
    msg.head.length = 2;

    if(cmd == BL_GET_MANID)
    {
        msg.text.data[2] = 0x0C;
        msg.head.length += 1;
    }
    else if (cmd == BL_GET_DEVID)
    {
        msg.text.data[2] = 0x0D;
        msg.head.length += 1;
    }
    else if (cmd == BL_GET_CHKSUM)
    {
        msg.text.data[2] = 0x00;
        msg.head.length += 1;
    }

    OutputBuffer[0]=0; // First byte is the report number
    memcpy(&OutputBuffer[1], &msg, (sizeof(msg.head)+sizeof(msg.text.cmd) + msg.head.length));
    return 0;
}

int DLPC900_PrepReadCmdWithParam(DLPC900_CMD cmd, unsigned char param)
/**
 * This function is private to this file. Prepares the read-control command packet for the given command code and parameter and copies it to OutputBuffer.
 *
 * @param   cmd  - I - USB command code.
 * @param   param - I - parameter to be used for tis read command.
 *
 * @return  0 = PASS
 *          -1 = FAIL
 *
 */
{
    hidMessageStruct msg;

    msg.head.flags.rw = 1; //Read
    msg.head.flags.reply = 1; //Host wants a reply from device
    msg.head.flags.dest = 0; //Projector Control Endpoint
    msg.head.flags.reserved = 0;
    msg.head.flags.nack = 0;
    msg.head.seq = 0;

    msg.text.cmd = (CmdList[cmd].CMD2 << 8) | CmdList[cmd].CMD3;
    msg.head.length = 3;

    msg.text.data[2] = param;

    OutputBuffer[0]=0; // First byte is the report number
    memcpy(&OutputBuffer[1], &msg, (sizeof(msg.head)+sizeof(msg.text.cmd) + msg.head.length));
    return 0;
}

int DLPC900_PrepWriteCmd(hidMessageStruct *pMsg, DLPC900_CMD cmd)
/**
 * This function is private to this file. Prepares the write command packet with given command code in the message structure pointer passed.
 *
 * @param   cmd  - I - USB command code.
 * @param   pMsg - I - Pointer to the message.
 *
 * @return  0 = PASS
 *          -1 = FAIL
 *
 */
{
    pMsg->head.flags.rw = 0; //Write
    pMsg->head.flags.reply = 1; //Host wants a reply from device
    pMsg->head.flags.dest = 0; //Projector Control Endpoint
    pMsg->head.flags.reserved = 0;
    pMsg->head.flags.nack = 0;
    pMsg->head.seq = seqNum++;

    pMsg->text.cmd = (CmdList[cmd].CMD2 << 8) | CmdList[cmd].CMD3;
    pMsg->head.length = CmdList[cmd].len + 2;

    return 0;
}

int DLPC900_GetVersion(unsigned int *pApp_ver, unsigned int *pAPI_ver, unsigned int *pSWConfig_ver, unsigned int *pSeqConfig_ver)
/**
 * This command reads the version information of the DLPC350 firmware.
 * (I2C: 0x11)
 * (USB: CMD2: 0x02, CMD3: 0x05)
 *
 * @param   pApp_ver  - O - Application Software Revision BITS 0:15 PATCH NUMBER, BITS 16:23 MINOR REVISION, BIS 24:31 MAJOR REVISION
 * @param   pAPI_ver  - O - API Software Revision BITS 0:15 PATCH NUMBER, BITS 16:23 MINOR REVISION, BIS 24:31 MAJOR REVISION
 * @param   pSWConfig_ver  - O - Software Configuration Revision BITS 0:15 PATCH NUMBER, BITS 16:23 MINOR REVISION, BIS 24:31 MAJOR REVISION
 * @param   pSeqConfig_ver  - O - Sequence Configuration Revision BITS 0:15 PATCH NUMBER, BITS 16:23 MINOR REVISION, BIS 24:31 MAJOR REVISION
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(GET_VERSION);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);

        *pApp_ver = *(unsigned int *)&msg.text.data[0];
        *pAPI_ver = *(unsigned int *)&msg.text.data[4];
        *pSWConfig_ver = *(unsigned int *)&msg.text.data[8];
        *pSeqConfig_ver = *(unsigned int *)&msg.text.data[12];
        return 0;
    }
    return -1;
}

int DLPC900_GetLedEnables(bool *pSeqCtrl, bool *pRed, bool *pGreen, bool *pBlue)
/**
 * This command reads back the state of LED control method as well as the enabled/disabled status of all LEDs.
 * (I2C: 0x10)
 * (USB: CMD2: 0x1A, CMD3: 0x07)
 *
 * @param   pSeqCtrl  - O - 1 - All LED enables are controlled by the Sequencer and ignore the other LED enable settings.
 *                          0 - All LED enables are controlled by pRed, pGreen and pBlue seetings and ignore Sequencer control
 * @param   pRed  - O - 0 - Red LED is disabled
 *                      1 - Red LED is enabled
 * @param   pGreen  - O - 0 - Green LED is disabled
 *                      1 - Green LED is enabled
 * @param   pBlue  - O - 0 - Blue LED is disabled
 *                      1 - Blue LED is enabled]
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(LED_ENABLE);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);

        if(msg.text.data[0] & BIT0)
            *pRed = true;
        else
            *pRed = false;

        if(msg.text.data[0] & BIT1)
            *pGreen = true;
        else
            *pGreen = false;

        if(msg.text.data[0] & BIT2)
            *pBlue = true;
        else
            *pBlue = false;

        if(msg.text.data[0] & BIT3)
            *pSeqCtrl = true;
        else
            *pSeqCtrl = false;
        return 0;
    }
    return -1;
}


int DLPC900_SetLedEnables(bool SeqCtrl, bool Red, bool Green, bool Blue)
/**
 * This command sets the state of LED control method as well as the enabled/disabled status of all LEDs.
 * (I2C: 0x10)
 * (USB: CMD2: 0x1A, CMD3: 0x07)
 *
 * @param   pSeqCtrl  - I - 1 - All LED enables are controlled by the Sequencer and ignore the other LED enable settings.
 *                          0 - All LED enables are controlled by pRed, pGreen and pBlue seetings and ignore Sequencer control
 * @param   pRed  - I - 0 - Red LED is disabled
 *                      1 - Red LED is enabled
 * @param   pGreen  - I - 0 - Green LED is disabled
 *                      1 - Green LED is enabled
 * @param   pBlue  - I - 0 - Blue LED is disabled
 *                      1 - Blue LED is enabled]
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;
    unsigned char Enable=0;

    if(SeqCtrl)
        Enable |= BIT3;
    if(Red)
        Enable |= BIT0;
    if(Green)
        Enable |= BIT1;
    if(Blue)
        Enable |= BIT2;

    msg.text.data[2] = Enable;
    DLPC900_PrepWriteCmd(&msg, LED_ENABLE);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_SetIT6535PowerMode(unsigned int powerMode)
{
    hidMessageStruct msg;
    msg.text.data[2] = powerMode;
    DLPC900_PrepWriteCmd(&msg, VIDEO_CONT_SEL);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetIT6535PowerMode(unsigned int *pPowerMode)
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(VIDEO_CONT_SEL);
    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);

        *pPowerMode = msg.text.data[0];

        return 0;
    }
    return -1;
}

int DLPC900_GetLedCurrents(unsigned char *pRed, unsigned char *pGreen, unsigned char *pBlue)
/**
 * (I2C: 0x4B)
 * (USB: CMD2: 0x0B, CMD3: 0x01)
 * This parameter controls the pulse duration of the specific LED PWM modulation output pin. The resolution
 * is 8 bits and corresponds to a percentage of the LED current. The PWM value can be set from 0 to 100%
 * in 256 steps . If the LED PWM polarity is set to normal polarity, a setting of 0xFF gives the maximum
 * PWM current. The LED current is a function of the specific LED driver design.
 *
 * @param   pRed  - O - Red LED PWM current control Valid range, assuming normal polarity of PWM signals, is:
 *                      0x00 (0% duty cycle → Red LED driver generates no current
 *                      0xFF (100% duty cycle → Red LED driver generates maximum current))
 *                      The current level corresponding to the selected PWM duty cycle is a function of the specific LED driver design and thus varies by design.
 * @param   pGreen  - O - Green LED PWM current control Valid range, assuming normal polarity of PWM signals, is:
 *                      0x00 (0% duty cycle → Red LED driver generates no current
 *                      0xFF (100% duty cycle → Red LED driver generates maximum current))
 *                      The current level corresponding to the selected PWM duty cycle is a function of the specific LED driver design and thus varies by design.
 * @param   pBlue  - O - Blue LED PWM current control Valid range, assuming normal polarity of PWM signals, is:
 *                      0x00 (0% duty cycle → Red LED driver generates no current
 *                      0xFF (100% duty cycle → Red LED driver generates maximum current))
 *                      The current level corresponding to the selected PWM duty cycle is a function of the specific LED driver design and thus varies by design.
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(LED_CURRENT);
    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);

        *pRed = msg.text.data[0];
        *pGreen = msg.text.data[1];
        *pBlue = msg.text.data[2];

        return 0;
    }
    return -1;
}


int DLPC900_SetLedCurrents(unsigned char RedCurrent, unsigned char GreenCurrent, unsigned char BlueCurrent)
/**
 * (I2C: 0x4B)
 * (USB: CMD2: 0x0B, CMD3: 0x01)
 * This parameter controls the pulse duration of the specific LED PWM modulation output pin. The resolution
 * is 8 bits and corresponds to a percentage of the LED current. The PWM value can be set from 0 to 100%
 * in 256 steps . If the LED PWM polarity is set to normal polarity, a setting of 0xFF gives the maximum
 * PWM current. The LED current is a function of the specific LED driver design.
 *
 * @param   RedCurrent  - I - Red LED PWM current control Valid range, assuming normal polarity of PWM signals, is:
 *                      0x00 (0% duty cycle → Red LED driver generates no current
 *                      0xFF (100% duty cycle → Red LED driver generates maximum current))
 *                      The current level corresponding to the selected PWM duty cycle is a function of the specific LED driver design and thus varies by design.
 * @param   GreenCurrent  - I - Green LED PWM current control Valid range, assuming normal polarity of PWM signals, is:
 *                      0x00 (0% duty cycle → Red LED driver generates no current
 *                      0xFF (100% duty cycle → Red LED driver generates maximum current))
 *                      The current level corresponding to the selected PWM duty cycle is a function of the specific LED driver design and thus varies by design.
 * @param   BlueCurrent  - I - Blue LED PWM current control Valid range, assuming normal polarity of PWM signals, is:
 *                      0x00 (0% duty cycle → Red LED driver generates no current
 *                      0xFF (100% duty cycle → Red LED driver generates maximum current))
 *                      The current level corresponding to the selected PWM duty cycle is a function of the specific LED driver design and thus varies by design.
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = RedCurrent;
    msg.text.data[3] = GreenCurrent;
    msg.text.data[4] = BlueCurrent;

    DLPC900_PrepWriteCmd(&msg, LED_CURRENT);

    return DLPC900_SendMsg(&msg);
}

bool DLPC900_GetLongAxisImageFlip(void)
/**
 * (I2C: 0x08)
 * (USB: CMD2: 0x10, CMD3: 0x08)
 * The Long-Axis Image Flip defines whether the input image is flipped across the long axis of the DMD. If
 * this parameter is changed while displaying a still image, the input still image should be re-sent. If the
 * image is not re-sent, the output image might be slightly corrupted. In Structured Light mode, the image
 * flip will take effect on the next bit-plane, image, or video frame load.
 *
 * @return  TRUE = Image flipped along long axis    <BR>
 *          FALSE = Image not flipped  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(FLIP_LONG);
    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);

        if ((msg.text.data[0] & BIT0) == BIT0)
            return true;
        else
            return false;
    }
    return false;
}

bool DLPC900_GetShortAxisImageFlip(void)
/**
 * (I2C: 0x09)
 * (USB: CMD2: 0x10, CMD3: 0x09)
 * The Short-Axis Image Flip defines whether the input image is flipped across the short axis of the DMD. If
 * this parameter is changed while displaying a still image, the input still image should be re-sent. If the
 * image is not re-sent, the output image might be slightly corrupted. In Structured Light mode, the image
 * flip will take effect on the next bit-plane, image, or video frame load.
 *
 * @return  TRUE = Image flipped along short axis    <BR>
 *          FALSE = Image not flipped  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(FLIP_SHORT);
    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);

        if ((msg.text.data[0] & BIT0) == BIT0)
            return true;
        else
            return false;
    }
    return false;
}


int DLPC900_SetLongAxisImageFlip(bool Flip)
/**
 * (I2C: 0x08)
 * (USB: CMD2: 0x10, CMD3: 0x08)
 * The Long-Axis Image Flip defines whether the input image is flipped across the long axis of the DMD. If
 * this parameter is changed while displaying a still image, the input still image should be re-sent. If the
 * image is not re-sent, the output image might be slightly corrupted. In Structured Light mode, the image
 * flip will take effect on the next bit-plane, image, or video frame load.
 *
 * @param   Flip -I TRUE = Image flipped along long axis enable    <BR>
 *                  FALSE = Do not flip image <BR>
 *
 * @return >=0 PASS <BR>
 *         <0 FAIL <BR>
 *
 */
{
    hidMessageStruct msg;

    if(Flip)
        msg.text.data[2] = BIT0;
    else
        msg.text.data[2] = 0;

    DLPC900_PrepWriteCmd(&msg, FLIP_LONG);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_SetShortAxisImageFlip(bool Flip)
/**
 * (I2C: 0x09)
 * (USB: CMD2: 0x10, CMD3: 0x09)
 * The Long-Axis Image Flip defines whether the input image is flipped across the long axis of the DMD. If
 * this parameter is changed while displaying a still image, the input still image should be re-sent. If the
 * image is not re-sent, the output image might be slightly corrupted. In Structured Light mode, the image
 * flip will take effect on the next bit-plane, image, or video frame load.
 *
 * @param   Flip -I TRUE = Image flipped along long axis enable    <BR>
 *                  FALSE = Do not flip image <BR>
 *
 * @return >=0 PASS <BR>
 *         <0 FAIL <BR>
 *
 */
{
    hidMessageStruct msg;

    if(Flip)
        msg.text.data[2] = BIT0;
    else
        msg.text.data[2] = 0;

    DLPC900_PrepWriteCmd(&msg, FLIP_SHORT);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_EnterProgrammingMode()
/**
 * This function is to be called to put the unit in programming mode. Only programming mode APIs will work once
 * in this mode.
 *
 * @return >=0 PASS <BR>
 *         <0 FAIL <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = 1;

    DLPC900_PrepWriteCmd(&msg, BL_PROG_MODE);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_ExitProgrammingMode(void)
/**
 * This function works only in prorgamming mode.
 * This function is to be called to exit programming mode and resume normal operation with the new downloaded firmware.
 *
 * @return >=0 PASS <BR>
 *         <0 FAIL <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = 2;
    DLPC900_PrepWriteCmd(&msg, BL_PROG_MODE);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_EnableMasterSlave(void)
{
    hidMessageStruct msg;

    msg.text.data[2] = 0;
    DLPC900_PrepWriteCmd(&msg, BL_MASTER_SLAVE);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_DisableMasterUpdate(void)
{
    hidMessageStruct msg;

    msg.text.data[2] = 1;
    DLPC900_PrepWriteCmd(&msg, BL_MASTER_SLAVE);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_DisableSlaveUpdate(void)
{
    hidMessageStruct msg;

    msg.text.data[2] = 2;
    DLPC900_PrepWriteCmd(&msg, BL_MASTER_SLAVE);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetFlashManID(unsigned short *pManID)
/**
 * This function works only in prorgamming mode.
 * This function returns the manufacturer ID of the flash part interfaced with the controller.
 *
 * @param pManID - O - Manufacturer ID of the flash part
 *
 * @return 0 PASS <BR>
 *         -1 FAIL <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(BL_GET_MANID);
    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);

        *pManID = msg.text.data[6];
        *pManID |= (unsigned short)msg.text.data[7] << 8;
        return 0;
    }
    return -1;
}

int DLPC900_GetFlashDevID(unsigned long long *pDevID)
/**
 * This function works only in prorgamming mode.
 * This function returns the device ID of the flash part interfaced with the controller.
 *
 * @param pDevID - O - Device ID of the flash part
 *
 * @return 0 PASS <BR>
 *         -1 FAIL <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(BL_GET_DEVID);
    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);

        *pDevID = msg.text.data[6];
        *pDevID |= (unsigned long long)msg.text.data[7] << 8;
        *pDevID |= (unsigned long long)msg.text.data[8] << 16;
        *pDevID |= (unsigned long long)msg.text.data[9] << 24;
        *pDevID |= (unsigned long long)msg.text.data[12] << 32;
        *pDevID |= (unsigned long long)msg.text.data[13] << 40;
        *pDevID |= (unsigned long long)msg.text.data[14] << 48;
        *pDevID |= (unsigned long long)msg.text.data[15] << 56;
        return 0;
    }
    return -1;
}

int DLPC900_SetFlashAddr(unsigned int Addr)
/**
 * This function works only in prorgamming mode.
 * This function is to be called to set the address prior to calling DLPC900_FlashSectorErase or DLPC900_DownloadData APIs.
 *
 * @param Addr - I - 32-bit absolute address.
 *
 * @return >=0 PASS <BR>
 *         <0 FAIL <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = Addr;
    msg.text.data[3] = Addr >> 8;
    msg.text.data[4] = Addr >> 16;
    msg.text.data[5] = Addr >> 24;

    DLPC900_PrepWriteCmd(&msg, BL_SET_SECTADDR);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_FlashSectorErase(void)
/**
  * This function works only in prorgamming mode.
  * This function is to be called to erase a sector of flash. The address of the sector to be erased
  * is to be set by using the DLPC900_SetFlashAddr() API
  *
  * @return >=0 PASS <BR>
  *         <0 FAIL <BR>
  *
  */
{
    hidMessageStruct msg;

    DLPC900_PrepWriteCmd(&msg, BL_SECT_ERASE);
    return DLPC900_SendMsg(&msg);
}

int DLPC900_SetDownloadSize(unsigned int dataLen)
/**
 * This function works only in prorgamming mode.
 * This function is to be called to set the payload size of data to be sent using DLPC900_DownloadData API.
 *
 * @param dataLen -I - length of download data payload in bytes.
 *
 * @return >=0 PASS <BR>
 *         <0 FAIL <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = dataLen;
    msg.text.data[3] = dataLen >> 8;
    msg.text.data[4] = dataLen >> 16;
    msg.text.data[5] = dataLen >> 24;

    DLPC900_PrepWriteCmd(&msg, BL_SET_DNLDSIZE);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_DownloadData(unsigned char *pByteArray, unsigned int dataLen)
/**
 * This function works only in prorgamming mode.
 * This function sends one payload of data to the controller at a time. takes the total size of payload
 * in the parameter dataLen and returns the actual number of bytes that was sent in the return value.
 * This function needs to be called multiple times until all of the desired bytes are sent.
 *
 * @param pByteArray - I - Pointer to where the data to be downloaded is to be fetched from
 * @param dataLen -I - length in bytes of the total payload data to download.
 *
 * @return number of bytes actually downloaded <BR>
 *         <0 FAIL <BR>
 *
 */
{
    hidMessageStruct msg;
    int retval;
    unsigned int sendSize;

    sendSize = HID_MESSAGE_MAX_SIZE - sizeof(msg.head)- sizeof(msg.text.cmd) - 2;//The last -2 is to workaround a bug in bootloader.

    if(dataLen > sendSize)
        dataLen = sendSize;

    CmdList[BL_DNLD_DATA].len = dataLen;
    memcpy(&msg.text.data[2], pByteArray, dataLen);

    DLPC900_PrepWriteCmd(&msg, BL_DNLD_DATA);

    retval = DLPC900_SendMsg(&msg);
    if(retval > 0)
        return dataLen;

    return -1;
}

int DLPC900_GetBLStatus(unsigned char *BL_Status)
/**
 * This function works only in prorgamming mode.
 * This function returns the device ID of the flash part interfaced with the controller.
 *
 * @param BL_Status - O - BIT3 of the status byte when set indicates that the program is busy
 *                        with exectuing the previous command. When BIT3 is reset, it means the
 *                        program is ready for the next command.
 *
 * @return 0 PASS <BR>
 *         -1 FAIL <BR>
 *
 */
{
    hidMessageStruct msg;

    /* For some reason BL_STATUS readback is not working properly.
     * However, after going through the bootloader code, I have ascertained that any
     * readback is fine - Byte 0 is always the bootloader status */
    DLPC900_PrepReadCmd(BL_STATUS);
    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);

        *BL_Status = msg.text.data[0];
        return 0;
    }
    return -1;
}

void DLPC900_WaitForFlashReady()
/**
 * This function works only in prorgamming mode.
 * This function polls the status bit and returns only when the controller is ready for next command.
 *
 */
{
    unsigned char BLstatus=STAT_BIT_FLASH_BUSY;

    do
    {
        DLPC900_GetBLStatus(&BLstatus);
    }
    while((BLstatus & STAT_BIT_FLASH_BUSY) == STAT_BIT_FLASH_BUSY);//Wait for flash busy flag to go off
}

int DLPC900_SetFlashType(unsigned char Type)
/**
 * This function works only in prorgamming mode.
 * This function is to be used to set the programming type of the flash device attached to the controller.
 *
 * @param Type - I - Type of the flash device.
 *
 * @return >=0 PASS <BR>
 *         <0 FAIL <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = Type;

    DLPC900_PrepWriteCmd(&msg, BL_FLASH_TYPE);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_CalculateFlashChecksum(void)
/**
 * This function works only in prorgamming mode.
 * This function is to be issued to instruct the controller to calculate the flash checksum.
 * DLPC900_WaitForFlashReady() is then to be called to ensure that the controller is done and then call
 * DLPC900_GetFlashChecksum() API to retrieve the actual checksum from the controller.
 *
 * @return 0 = PASS <BR>
 *         -1 = FAIL <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepWriteCmd(&msg, BL_CALC_CHKSUM);

    if(DLPC900_SendMsg(&msg) <= 0)
        return -1;

    return 0;

}

int DLPC900_GetFlashChecksum(unsigned int*checksum)
/**
 * This function works only in prorgamming mode.
 * This function is to be used to retrieve the flash checksum from the controller.
 * DLPC900_CalculateFlashChecksum() and DLPC900_WaitForFlashReady() must be called before using this API.
 *
 * @param checksum - O - variable in which the flash checksum is to be returned
 *
 * @return >=0 PASS <BR>
 *         <0 FAIL <BR>
 *
 */
{
    hidMessageStruct msg;
#if 0
    DLPC900_PrepWriteCmd(&msg, BL_CALC_CHKSUM);

    if(DLPC900_SendMsg(&msg) <= 0)
        return -1;

    DLPC900_WaitForFlashReady();
#endif
    DLPC900_PrepReadCmd(BL_GET_CHKSUM);
    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);

        *checksum = msg.text.data[6];
        *checksum |= (unsigned int)msg.text.data[7] << 8;
        *checksum |= (unsigned int)msg.text.data[8] << 16;
        *checksum |= (unsigned int)msg.text.data[9] << 24;
        return 0;
    }
    return -1;
}

int DLPC900_GetStatus(unsigned char *pHWStatus, unsigned char *pSysStatus, unsigned char *pMainStatus)
/**
 * This function is to be used to check the various status indicators from the controller.
 * Refer to DLPC350 Programmer's guide section 2.1 "DLPC350 Status Commands" for detailed description of each byte.
 *
 * @param pHWStatus - O - provides status information on the DLPC350's sequencer, DMD controller and initialization.
 * @param pSysStatus - O - provides DLPC350 status on internal memory tests..
 * @param pMainStatus - O - provides DMD park status and DLPC350 sequencer, frame buffer, and gamma correction status.
 *
 * @return 0 PASS <BR>
 *         -1 FAIL <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(STATUS_HW);
    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);

        *pHWStatus = msg.text.data[0];
    }
    else
        return -1;

    DLPC900_PrepReadCmd(STATUS_SYS);
    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);

        *pSysStatus = msg.text.data[0];
    }
    else
        return -1;

    DLPC900_PrepReadCmd(STATUS_MAIN);
    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);

        *pMainStatus = msg.text.data[0];
    }
    else
        return -1;

    return 0;
}

int DLPC900_SoftwareReset(void)
/**
 * Use this API to reset the controller
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = 0x01;
    DLPC900_PrepWriteCmd(&msg, SW_RESET);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_SetMode(int SLmode)
/**
 * The Display Mode Selection Command enables the DLPC350 internal image processing functions for
 * video mode or bypasses them for pattern display mode. This command selects between video or pattern
 * display mode of operation.
 *
 * @param   SLmode  - I - TRUE = Pattern Display mode. Assumes a 1-bit through 8-bit image with a pixel
 *                              resolution of 912 x 1140 and bypasses all the image processing functions of DLPC350
 *                          FALSE = Video Display mode. Assumes streaming video image from the 30-bit
 *                              RGB or FPD-link interface with a pixel resolution of up to 1280 x 800 up to 120 Hz.
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = SLmode;
    DLPC900_PrepWriteCmd(&msg, DISP_MODE);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetMode(int *pMode)
/**
 * The Display Mode Selection Command enables the DLPC350 internal image processing functions for
 * video mode or bypasses them for pattern display mode. This command selects between video or pattern
 * display mode of operation.
 *
 * @param   SLmode  - O - TRUE = Pattern Display mode. Assumes a 1-bit through 8-bit image with a pixel
 *                              resolution of 912 x 1140 and bypasses all the image processing functions of DLPC350
 *                        FALSE = Video Display mode. Assumes streaming video image from the 30-bit
 *                              RGB or FPD-link interface with a pixel resolution of up to 1280 x 800 up to 120 Hz.
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(DISP_MODE);
    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pMode = msg.text.data[0];
        return 0;
    }
    return -1;
}

int DLPC900_GetPowerMode(bool *Standby)
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(POWER_CONTROL);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *Standby = (msg.text.data[0] != 0);
        return 0;
    }
    return -1;
}

int DLPC900_SetPowerMode(unsigned char Mode)
/**
 * (I2C: 0x07)
 * (USB: CMD2: 0x02, CMD3: 0x00)
 * The Power Control places the DLPC350 in a low-power state and powers down the DMD interface.
 * Standby mode should only be enabled after all data for the last frame to be displayed has been
 * transferred to the DLPC350. Standby mode must be disabled prior to sending any new data.
 *
 * @param   Standby  - I - TRUE = Standby mode. Places DLPC350 in low power state and powers down the DMD interface
 *                         FALSE = Normal operation. The selected external source will be displayed
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    if (Mode > 0x2)
        return -1;
    msg.text.data[2] = Mode;
    DLPC900_PrepWriteCmd(&msg, POWER_CONTROL);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_SetRedLEDStrobeDelay(short rising, short falling)
/**
 * (I2C: 0x6C)
 * (USB: CMD2: 0x1A, CMD3: 0x1F)
 * The Red LED Enable Delay Control command sets the rising and falling edge delay of the Red LED enable signal.
 *
 * @param   rising  - I - Red LED enable rising edge delay control. Each bit adds 107.2 ns.
 *                        0x00 = -20.05 μs, 0x01 = -19.9428 μs, ......0xBB=0.00 μs, ......, 0xFE = +7.1828 μs, 0xFF = +7.29 μs
 * @param   falling  - I - Red LED enable falling edge delay control. Each bit adds 107.2 ns.
 *                        0x00 = -20.05 μs, 0x01 = -19.9428 μs, ......0xBB=0.00 μs, ......, 0xFE = +7.1828 μs, 0xFF = +7.29 μs
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = rising & 0xFF;
    msg.text.data[3] = (rising >> 8) & 0xFF;
    msg.text.data[4] = falling & 0xFF;
    msg.text.data[5] = (falling >> 8) & 0xFF;

    DLPC900_PrepWriteCmd(&msg, RED_LED_ENABLE_DLY);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_SetGreenLEDStrobeDelay(short rising, short falling)
/**
 * (I2C: 0x6D)
 * (USB: CMD2: 0x1A, CMD3: 0x20)
 * The Green LED Enable Delay Control command sets the rising and falling edge delay of the Green LED enable signal.
 *
 * @param   rising  - I - Green LED enable rising edge delay control. Each bit adds 107.2 ns.
 *                        0x00 = -20.05 μs, 0x01 = -19.9428 μs, ......0xBB=0.00 μs, ......, 0xFE = +7.1828 μs, 0xFF = +7.29 μs
 * @param   falling  - I - Green LED enable falling edge delay control. Each bit adds 107.2 ns.
 *                        0x00 = -20.05 μs, 0x01 = -19.9428 μs, ......0xBB=0.00 μs, ......, 0xFE = +7.1828 μs, 0xFF = +7.29 μs
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = rising & 0xFF;
    msg.text.data[3] = (rising >> 8) & 0xFF;
    msg.text.data[4] = falling & 0xFF;
    msg.text.data[5] = (falling >> 8) & 0xFF;

    DLPC900_PrepWriteCmd(&msg, GREEN_LED_ENABLE_DLY);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_SetBlueLEDStrobeDelay(short rising, short falling)
/**
 * (I2C: 0x6E)
 * (USB: CMD2: 0x1A, CMD3: 0x21)
 * The Blue LED Enable Delay Control command sets the rising and falling edge delay of the Blue LED enable signal.
 *
 * @param   rising  - I - Blue LED enable rising edge delay control. Each bit adds 107.2 ns.
 *                        0x00 = -20.05 μs, 0x01 = -19.9428 μs, ......0xBB=0.00 μs, ......, 0xFE = +7.1828 μs, 0xFF = +7.29 μs
 * @param   falling  - I - Blue LED enable falling edge delay control. Each bit adds 107.2 ns.
 *                        0x00 = -20.05 μs, 0x01 = -19.9428 μs, ......0xBB=0.00 μs, ......, 0xFE = +7.1828 μs, 0xFF = +7.29 μs
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = rising & 0xFF;
    msg.text.data[3] = (rising >> 8) & 0xFF;
    msg.text.data[4] = falling & 0xFF;
    msg.text.data[5] = (falling >> 8) & 0xFF;

    DLPC900_PrepWriteCmd(&msg, BLUE_LED_ENABLE_DLY);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetRedLEDStrobeDelay(short *pRising, short *pFalling)
/**
 * (I2C: 0x6C)
 * (USB: CMD2: 0x1A, CMD3: 0x1F)
 * This command reads back the rising and falling edge delay of the Red LED enable signal.
 *
 * @param   pRising  - O - Red LED enable rising edge delay value. Each bit adds 107.2 ns.
 *                        0x00 = -20.05 μs, 0x01 = -19.9428 μs, ......0xBB=0.00 μs, ......, 0xFE = +7.1828 μs, 0xFF = +7.29 μs
 * @param   pFalling  - O - Red LED enable falling edge delay value. Each bit adds 107.2 ns.
 *                        0x00 = -20.05 μs, 0x01 = -19.9428 μs, ......0xBB=0.00 μs, ......, 0xFE = +7.1828 μs, 0xFF = +7.29 μs
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(RED_LED_ENABLE_DLY);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pRising = msg.text.data[0] | (msg.text.data[1] << 8);
        *pFalling = msg.text.data[2] | (msg.text.data[3] << 8);
        return 0;
    }
    return -1;
}

int DLPC900_GetGreenLEDStrobeDelay(short *pRising, short *pFalling)
/**
 * (I2C: 0x6D)
 * (USB: CMD2: 0x1A, CMD3: 0x20)
 * This command reads back the rising and falling edge delay of the Green LED enable signal.
 *
 * @param   pRising  - O - Green LED enable rising edge delay value. Each bit adds 107.2 ns.
 *                        0x00 = -20.05 μs, 0x01 = -19.9428 μs, ......0xBB=0.00 μs, ......, 0xFE = +7.1828 μs, 0xFF = +7.29 μs
 * @param   pFalling  - O - Green LED enable falling edge delay value. Each bit adds 107.2 ns.
 *                        0x00 = -20.05 μs, 0x01 = -19.9428 μs, ......0xBB=0.00 μs, ......, 0xFE = +7.1828 μs, 0xFF = +7.29 μs
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(GREEN_LED_ENABLE_DLY);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pRising = msg.text.data[0] | (msg.text.data[1] << 8);
        *pFalling = msg.text.data[2] | (msg.text.data[3] << 8);
        return 0;
    }
    return -1;
}

int DLPC900_GetBlueLEDStrobeDelay(short *pRising, short *pFalling)
/**
 * (I2C: 0x6E)
 * (USB: CMD2: 0x1A, CMD3: 0x21)
 * This command reads back the rising and falling edge delay of the Blue LED enable signal.
 *
 * @param   pRising  - O - Blue LED enable rising edge delay value. Each bit adds 107.2 ns.
 *                        0x00 = -20.05 μs, 0x01 = -19.9428 μs, ......0xBB=0.00 μs, ......, 0xFE = +7.1828 μs, 0xFF = +7.29 μs
 * @param   pFalling  - O - Blue LED enable falling edge delay value. Each bit adds 107.2 ns.
 *                        0x00 = -20.05 μs, 0x01 = -19.9428 μs, ......0xBB=0.00 μs, ......, 0xFE = +7.1828 μs, 0xFF = +7.29 μs
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(BLUE_LED_ENABLE_DLY);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pRising = msg.text.data[0] | (msg.text.data[1] << 8);
        *pFalling = msg.text.data[2] | (msg.text.data[3] << 8);
        return 0;
    }
    return -1;
}

int DLPC900_SetInputSource(unsigned int source, unsigned int portWidth)
/**
 * (I2C: 0x00)
 * (USB: CMD2: 0x1A, CMD3: 0x00)
 * The Input Source Selection command selects the input source to be displayed by the DLPC350: 30-bit
 * Parallel Port, Internal Test Pattern, Flash memory, or FPD-link interface.
 *
 * @param   source  - I - Select the input source and interface mode:
 *                        0 = Parallel interface with 8-bit, 16-bit, 20-bit, 24-bit, or 30-bit RGB or YCrCb data formats
 *                        1 = Internal test pattern; Use DLPC900_SetTPGSelect() API to select pattern
 *                        2 = Flash. Images are 24-bit single-frame, still images stored in flash that are uploaded on command.
 *                        3 = FPD-link interface
 * @param   portWidth  - I - Parallel Interface bit depth
 *                           0 = 30-bits
 *                           1 = 24-bits
 *                           2 = 20-bits
 *                           3 = 16-bits
 *                           4 = 10-bits
 *                           5 = 8-bits
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = source;
    msg.text.data[2] |= portWidth << 3;
    DLPC900_PrepWriteCmd(&msg, SOURCE_SEL);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_SetCurtainColor(unsigned int red,unsigned int green, unsigned int blue)
{
    hidMessageStruct msg;
    msg.text.data[2] = red & 0xff;
    msg.text.data[3] = (red >> 8) & 0x03;
    msg.text.data[4] = green & 0xff;
    msg.text.data[5] = (green >> 8) & 0x03;
    msg.text.data[6] = blue & 0xff;
    msg.text.data[7] = (blue >> 8) & 0x03;

    DLPC900_PrepWriteCmd(&msg, CURTAIN_COLOR);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetCurtainColor(unsigned int *pRed,unsigned int *pGreen, unsigned int *pBlue)
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(CURTAIN_COLOR);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pRed   = msg.text.data[0] | msg.text.data[1]<<8;
        *pGreen = msg.text.data[2] | msg.text.data[3]<<8;
        *pBlue  = msg.text.data[4] | msg.text.data[5]<<8;

        return 0;
    }
    return -1;
}

int DLPC900_GetInputSource(unsigned int *pSource, unsigned int *pPortWidth)
/**
 * (I2C: 0x00)
 * (USB: CMD2: 0x1A, CMD3: 0x00)
 * Thisn command reads back the input source to be displayed by the DLPC350
 *
 * @param   pSource  - O - Input source and interface mode:
 *                        0 = Parallel interface with 8-bit, 16-bit, 20-bit, 24-bit, or 30-bit RGB or YCrCb data formats
 *                        1 = Internal test pattern; Use DLPC900_SetTPGSelect() API to select pattern
 *                        2 = Flash. Images are 24-bit single-frame, still images stored in flash that are uploaded on command.
 *                        3 = FPD-link interface
 * @param   pPortWidth  - O - Parallel Interface bit depth
 *                           0 = 30-bits
 *                           1 = 24-bits
 *                           2 = 20-bits
 *                           3 = 16-bits
 *                           4 = 10-bits
 *                           5 = 8-bits
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(SOURCE_SEL);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pSource = msg.text.data[0] & (BIT0 | BIT1 | BIT2);
        *pPortWidth = msg.text.data[0] >> 3;
        return 0;
    }
    return -1;
}

int DLPC900_SetPixelFormat(unsigned int format)
/**
 * (I2C: 0x02)
 * (USB: CMD2: 0x1A, CMD3: 0x02)
 * This API defines the pixel data format input into the DLPC350.Refer to programmer's guide for supported pixel formats
 * for each source type.
 *
 * @param   format  - I - Select the pixel data format:
 *                        0 = RGB 4:4:4 (30-bit)
 *                        1 = YCrCb 4:4:4 (30-bit)
 *                        2 = YCrCb 4:2:2
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = format;
    DLPC900_PrepWriteCmd(&msg, PIXEL_FORMAT);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetPixelFormat(unsigned int *pFormat)
/**
 * (I2C: 0x02)
 * (USB: CMD2: 0x1A, CMD3: 0x02)
 * This API returns the defined the pixel data format input into the DLPC350.Refer to programmer's guide for supported pixel formats
 * for each source type.
 *
 * @param   pFormat  - O - Pixel data format:
 *                        0 = RGB 4:4:4 (30-bit)
 *                        1 = YCrCb 4:4:4 (30-bit)
 *                        2 = YCrCb 4:2:2
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(PIXEL_FORMAT);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pFormat = msg.text.data[0] & (BIT0 | BIT1 | BIT2);
        return 0;
    }
    return -1;
}

int DLPC900_SetPortConfig(unsigned int dataPort,unsigned int pixelClock,unsigned int dataEnable,unsigned int syncSelect)
/**
 * (I2C: 0x03)
 * (USB: CMD2: 0x1A, CMD3: 0x03)
 * This command sets on which port the RGB data is on and which pixel clock, data enable,
 * and syncs to use.
 *
 * @param   dataPort     0 - Data Port 1 (Valid when bit 0 = 0)Single Pixel mode
 *                       1 - Data Port 2 (Valid when bit 0 = 0)Single Pixel mode
 *                       2 - Data Port 1-2 (Valid when bit 0 = 1. Even pixel on port 1, Odd pixel on port 2)Dual Pixel mode
 *                       3 - Data Port 2-1 (Valid when bit 0 = 1. Even pixel on port 2, Odd pixel on port 1)Dual Pixel mode
 * @param   pixelClock   0 - Pixel Clock 1
 *                       1 - Pixel Clock 2
 *                       2 - Pixel Clock 3
 * @param   dataEnable   0 - Data Enable 1
 *                       1 - Data Enable 2
 * @param   syncSelect   0 - P1 VSync and P1 HSync
 *                       1 - P2 VSync and P2 HSync
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    unsigned int data = 0;
    data = dataPort | (pixelClock << 2) | (dataEnable << 4) | (syncSelect << 5);

    hidMessageStruct msg;

    msg.text.data[2] = data;
    DLPC900_PrepWriteCmd(&msg, CLK_SEL);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetPortConfig(unsigned int *pDataPort,unsigned int *pPixelClock,unsigned int *pDataEnable,unsigned int *pSyncSelect)
/**
 * (I2C: 0x03)
 * (USB: CMD2: 0x1A, CMD3: 0x03)
 * This command reads on which port the RGB data is on and which pixel clock, data enable,
 * and syncs is used.
 *
 * @param   pDataPort     0 - Data Port 1 (Valid when bit 0 = 0)Single Pixel mode
 *                       1 - Data Port 2 (Valid when bit 0 = 0)Single Pixel mode
 *                       2 - Data Port 1-2 (Valid when bit 0 = 1. Even pixel on port 1, Odd pixel on port 2)Dual Pixel mode
 *                       3 - Data Port 2-1 (Valid when bit 0 = 1. Even pixel on port 2, Odd pixel on port 1)Dual Pixel mode
 * @param   pPixelClock   0 - Pixel Clock 1
 *                       1 - Pixel Clock 2
 *                       2 - Pixel Clock 3
 * @param   pDataEnable   0 - Data Enable 1
 *                       1 - Data Enable 2
 * @param   pSyncSelect   0 - P1 VSync and P1 HSync
 *                       1 - P2 VSync and P2 HSync
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(CLK_SEL);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pDataPort = (msg.text.data[0] & (BIT0 | BIT1));
        *pPixelClock = (msg.text.data[0] & (BIT2 | BIT3)) >> 2;
        *pDataEnable = (msg.text.data[0] & BIT4) >> 4;
        *pSyncSelect = (msg.text.data[0] & BIT5) >> 5;

        return 0;
    }
    return -1;
}

int DLPC900_SetDataChannelSwap(unsigned int port, unsigned int swap)
/**
 * (I2C: 0x04)
 * (USB: CMD2: 0x1A, CMD3: 0x37)
 * This API configures the specified input data port and map the data subchannels.
 * The DLPC350 interprets Channel A as Green, Channel B as Red, and Channel C as Blue.
 *
 * @param   port  - I - Selects the port:
 *                        0 = Port1, parallel interface
 *                        1 = Port2, FPD-link interface
 * @param   swap - I - Swap Data Sub-Channel:
 *                     0 - ABC = ABC, No swapping of data sub-channels
 *                     1 - ABC = CAB, Data sub-channels are right shifted and circularly rotated
 *                     2 - ABC = BCA, Data sub-channels are left shifted and circularly rotated
 *                     3 - ABC = ACB, Data sub-channels B and C are swapped
 *                     4 - ABC = BAC, Data sub-channels A and B are swapped
 *                     5 - ABC = CBA, Data sub-channels A and C are swapped
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = port << 7;
    msg.text.data[2] |= swap & (BIT0 | BIT1 | BIT2);
    DLPC900_PrepWriteCmd(&msg, CHANNEL_SWAP);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetDataChannelSwap(unsigned int *pPort, unsigned int *pSwap)
/**
 * (I2C: 0x04)
 * (USB: CMD2: 0x1A, CMD3: 0x37)
 * This API reads the data subchannel mapping for the specified input data port and map the data subchannels
 *
 * @param   *pPort  - O - Selected port:
 *                        0 = Port1, parallel interface
 *                        1 = Port2, FPD-link interface
 * @param   *pSwap - O - Swap Data Sub-Channel:
 *                     0 - ABC = ABC, No swapping of data sub-channels
 *                     1 - ABC = CAB, Data sub-channels are right shifted and circularly rotated
 *                     2 - ABC = BCA, Data sub-channels are left shifted and circularly rotated
 *                     3 - ABC = ACB, Data sub-channels B and C are swapped
 *                     4 - ABC = BAC, Data sub-channels A and B are swapped
 *                     5 - ABC = CBA, Data sub-channels A and C are swapped
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(CHANNEL_SWAP);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pSwap = msg.text.data[0] & (BIT0 | BIT1 | BIT2);
        if(msg.text.data[0] & BIT7)
            *pPort = 1;
        else
            *pPort = 0;
        return 0;
    }
    return -1;
}

int DLPC900_SetFPD_Mode_Field(unsigned int PixelMappingMode, bool SwapPolarity, unsigned int FieldSignalSelect)
/**
 * (I2C: 0x05)
 * (USB: CMD2: 0x1A, CMD3: 0x04)
 * The FPD-Link Mode and Field Select command configures the FPD-link pixel map, polarity, and signal select.
 *
 * @param   PixelMappingMode  - I - FPD-link Pixel Mapping Mode: See table 2-21 in programmer's guide for more details
 *                                  0 = Mode 1
 *                                  1 = Mode 2
 *                                  2 = Mode 3
 *                                  3 = Mode 4
 * @param   SwapPolarity - I - Polarity select
 *                             true = swap polarity
 *                             false = do not swap polarity
 *
 * @param   FieldSignalSelect -I -  Field Signal Select
 *                              0 - Map FPD-Link output from CONT1 onto Field Signal for FPD-link interface port
 *                              1 - Map FPD-Link output from CONT2 onto Field Signal for FPD-link interface port
 *                              2 - Force 0 onto Field Signal for FPD-link interface port
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = PixelMappingMode << 6;
    msg.text.data[2] |= FieldSignalSelect & (BIT0 | BIT1 | BIT2);
    if(SwapPolarity)
        msg.text.data[2] |= BIT3;
    DLPC900_PrepWriteCmd(&msg, FPD_MODE);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetFPD_Mode_Field(unsigned int *pPixelMappingMode, bool *pSwapPolarity, unsigned int *pFieldSignalSelect)
/**
 * (I2C: 0x05)
 * (USB: CMD2: 0x1A, CMD3: 0x04)
 * This command reads back the configuration of FPD-link pixel map, polarity, and signal select.
 *
 * @param   PixelMappingMode  - O - FPD-link Pixel Mapping Mode: See table 2-21 in programmer's guide for more details
 *                                  0 = Mode 1
 *                                  1 = Mode 2
 *                                  2 = Mode 3
 *                                  3 = Mode 4
 * @param   SwapPolarity - O - Polarity select
 *                             true = swap polarity
 *                             false = do not swap polarity
 *
 * @param   FieldSignalSelect - O -  Field Signal Select
 *                              0 - Map FPD-Link output from CONT1 onto Field Signal for FPD-link interface port
 *                              1 - Map FPD-Link output from CONT2 onto Field Signal for FPD-link interface port
 *                              2 - Force 0 onto Field Signal for FPD-link interface port
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(FPD_MODE);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pFieldSignalSelect = msg.text.data[0] & (BIT0 | BIT1 | BIT2);
        if(msg.text.data[0] & BIT3)
            *pSwapPolarity = 1;
        else
            *pSwapPolarity = 0;
        *pPixelMappingMode = msg.text.data[0] >> 6;
        return 0;
    }
    return -1;
}

int DLPC900_SetTPGSelect(unsigned int pattern)
/**
 * (I2C: 0x0A)
 * (USB: CMD2: 0x12, CMD3: 0x03)
 * When the internal test pattern is the selected input, the Internal Test Patterns Select defines the test
 * pattern displayed on the screen. These test patterns are internally generated and injected into the
 * beginning of the DLPC350 image processing path. Therefore, all image processing is performed on the
 * test images. All command registers should be set up as if the test images are input from an RGB 8:8:8
 * external source.
 *
 * @param   pattern  - I - Selects the internal test pattern:
 *                         0x0 = Solid Field
 *                         0x1 = Horizontal Ramp
 *                         0x2 = Vertical Ramp
 *                         0x3 = Horizontal Lines
 *                         0x4 = Diagonal Lines
 *                         0x5 = Vertical Lines
 *                         0x6 = Grid
 *                         0x7 = Checkerboard
 *                         0x8 = RGB Ramp
 *                         0x9 = Color Bars
 *                         0xA = Step Bars
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = pattern;
    DLPC900_PrepWriteCmd(&msg, TPG_SEL);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetTPGSelect(unsigned int *pPattern)
/**
 * (I2C: 0x0A)
 * (USB: CMD2: 0x12, CMD3: 0x03)
 * This command reads back the selected internal test pattern.
 *
 * @param   pattern  - O - Selected internal test pattern:
 *                         0x0 = Solid Field
 *                         0x1 = Horizontal Ramp
 *                         0x2 = Vertical Ramp
 *                         0x3 = Horizontal Lines
 *                         0x4 = Diagonal Lines
 *                         0x5 = Vertical Lines
 *                         0x6 = Grid
 *                         0x7 = Checkerboard
 *                         0x8 = RGB Ramp
 *                         0x9 = Color Bars
 *                         0xA = Step Bars
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(TPG_SEL);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pPattern = msg.text.data[0] & (BIT0 | BIT1 | BIT2 | BIT3);
        return 0;
    }
    return -1;
}

int DLPC900_LoadSplash(unsigned int index)
/**
 * (I2C: 0x7F)
 * (USB: CMD2: 0x1A, CMD3: 0x39)
 * This command loads an image from flash memory and then performs a buffer swap to display the loaded
 * image on the DMD.
 *
 * @param   index  - I - Image Index. Loads the image at this index from flash.
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = index;
    DLPC900_PrepWriteCmd(&msg, SPLASH_LOAD);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetSplashIndex(unsigned int *pIndex)
/**
 * (I2C: 0x7F)
 * (USB: CMD2: 0x1A, CMD3: 0x39)
 * This command loads reads back the index that was loaded most recently via DLPC900_LoadSplash() API.
 *
 * @param   *pIndex  - O - Image Index. Image at this index is loaded from flash.
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(SPLASH_LOAD);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pIndex = msg.text.data[0];
        return 0;
    }
    return -1;
}

int DLPC900_SetDisplay(rectangle croppedArea, rectangle displayArea)
/**
 * (I2C: 0x7E)
 * (USB: CMD2: 0x10, CMD3: 0x00)
 * The Input Display Resolution command defines the active input resolution and active output (displayed)
 * resolution. The maximum supported input and output resolutions for the DLP4500 0.45 WXGA DMD is
 * 1280 pixels (columns) by 800 lines (rows). This command provides the option to define a subset of active
 * input frame data using pixel (column) and line (row) counts relative to the source-data enable signal
 * (DATEN). In other words, this feature allows the source image to be cropped as the first step in the
 * processing chain.
 *
 * @param croppedArea - I - The rectagle structure contains the following
 *          parameters to describe the area to be cropped:
 *              - FirstPixel <BR>
 *              - FirstLine <BR>
 *              - PixelsPerLine <BR>
 *              - LinesPerFrame <BR>
 * @param displayArea - I - The rectagle structure contains the following
 *          parameters to describe the display area:
 *              - FirstPixel <BR>
 *              - FirstLine <BR>
 *              - PixelsPerLine <BR>
 *              - LinesPerFrame <BR>
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = croppedArea.firstPixel & 0xFF;
    msg.text.data[3] = croppedArea.firstPixel >> 8;
    msg.text.data[4] = croppedArea.firstLine & 0xFF;
    msg.text.data[5] = croppedArea.firstLine >> 8;
    msg.text.data[6] = croppedArea.pixelsPerLine & 0xFF;
    msg.text.data[7] = croppedArea.pixelsPerLine >> 8;
    msg.text.data[8] = croppedArea.linesPerFrame & 0xFF;
    msg.text.data[9] = croppedArea.linesPerFrame >> 8;
    msg.text.data[10] = displayArea.firstPixel & 0xFF;
    msg.text.data[11] = displayArea.firstPixel >> 8;
    msg.text.data[12] = displayArea.firstLine & 0xFF;
    msg.text.data[13] = displayArea.firstLine >> 8;
    msg.text.data[14] = displayArea.pixelsPerLine & 0xFF;
    msg.text.data[15] = displayArea.pixelsPerLine >> 8;
    msg.text.data[16] = displayArea.linesPerFrame & 0xFF;
    msg.text.data[17] = displayArea.linesPerFrame >> 8;

    DLPC900_PrepWriteCmd(&msg, DISP_CONFIG);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetDisplay(rectangle *pCroppedArea, rectangle *pDisplayArea)
/**
 * (I2C: 0x7E)
 * (USB: CMD2: 0x10, CMD3: 0x00)
 * This command reads back the active input resolution and active output (displayed) resolution.
 *
 * @param *pCroppedArea - O - The rectagle structure contains the following
 *          parameters to describe the area to be cropped:
 *              - FirstPixel <BR>
 *              - FirstLine <BR>
 *              - PixelsPerLine <BR>
 *              - LinesPerFrame <BR>
 * @param *pDisplayArea - O - The rectagle structure contains the following
 *          parameters to describe the display area:
 *              - FirstPixel <BR>
 *              - FirstLine <BR>
 *              - PixelsPerLine <BR>
 *              - LinesPerFrame <BR>
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(DISP_CONFIG);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        pCroppedArea->firstPixel = msg.text.data[0] | msg.text.data[1] << 8;
        pCroppedArea->firstLine = msg.text.data[2] | msg.text.data[3] << 8;
        pCroppedArea->pixelsPerLine = msg.text.data[4] | msg.text.data[5] << 8;
        pCroppedArea->linesPerFrame = msg.text.data[6] | msg.text.data[7] << 8;
        pDisplayArea->firstPixel = msg.text.data[8] | msg.text.data[9] << 8;
        pDisplayArea->firstLine = msg.text.data[10] | msg.text.data[11] << 8;
        pDisplayArea->pixelsPerLine = msg.text.data[12] | msg.text.data[13] << 8;
        pDisplayArea->linesPerFrame = msg.text.data[14] | msg.text.data[15] << 8;

        return 0;
    }
    return -1;
}

int DLPC900_SetTPGColor(unsigned short redFG, unsigned short greenFG, unsigned short blueFG, unsigned short redBG, unsigned short greenBG, unsigned short blueBG)
/**
 * (I2C: 0x1A)
 * (USB: CMD2: 0x12, CMD3: 0x04)
 * When the internal test pattern is the selected input, the Internal Test Patterns Color Control defines the
 * colors of the test pattern displayed on the screen. The foreground color setting affects all test patterns. The background color
 * setting affects those test patterns that have a foreground and background component, such as, Horizontal
 * Lines, Diagonal Lines, Vertical Lines, Grid, and Checkerboard.
 *
 * @param   redFG  - I - Red Foreground Color intensity in a scale from 0 to 1023
 *                       0x0 = No Red Foreground color intensity
 *                       0x3FF = Full Red Foreground color intensity
 * @param   greenFG  - I - Green Foreground Color intensity in a scale from 0 to 1023
 *                       0x0 = No Green Foreground color intensity
 *                       0x3FF = Full Green Foreground color intensity
 * @param   blueFG  - I - Blue Foreground Color intensity in a scale from 0 to 1023
 *                       0x0 = No Blue Foreground color intensity
 *                       0x3FF = Full Blue Foreground color intensity
 * @param   redBG  - I - Red Foreground Color intensity in a scale from 0 to 1023
 *                       0x0 = No Red Foreground color intensity
 *                       0x3FF = Full Red Foreground color intensity
 * @param   greenBG  - I - Green Foreground Color intensity in a scale from 0 to 1023
 *                       0x0 = No Green Foreground color intensity
 *                       0x3FF = Full Red Foreground color intensity
 * @param   blueBG  - I - Red Foreground Color intensity in a scale from 0 to 1023
 *                       0x0 = No Blue Foreground color intensity
 *                       0x3FF = Full Blue Foreground color intensity
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = (char)redFG;
    msg.text.data[3] = (char)(redFG >> 8);
    msg.text.data[4] = (char)greenFG;
    msg.text.data[5] = (char)(greenFG >> 8);
    msg.text.data[6] = (char)blueFG;
    msg.text.data[7] = (char)(blueFG >> 8);
    msg.text.data[8] = (char)redBG;
    msg.text.data[9] = (char)(redBG >> 8);
    msg.text.data[10] = (char)greenBG;
    msg.text.data[11] = (char)(greenBG >> 8);
    msg.text.data[12] = (char)blueBG;
    msg.text.data[13] = (char)(blueBG >> 8);

    DLPC900_PrepWriteCmd(&msg, TPG_COLOR);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetTPGColor(unsigned short *pRedFG, unsigned short *pGreenFG, unsigned short *pBlueFG, unsigned short *pRedBG, unsigned short *pGreenBG, unsigned short *pBlueBG)
/**
 * (I2C: 0x1A)
 * (USB: CMD2: 0x12, CMD3: 0x04)
 * When the internal test pattern is the selected input, the Internal Test Patterns Color Control defines the
 * colors of the test pattern displayed on the screen. The foreground color setting affects all test patterns. The background color
 * setting affects those test patterns that have a foreground and background component, such as, Horizontal
 * Lines, Diagonal Lines, Vertical Lines, Grid, and Checkerboard.
 *
 * @param   *pRedFG  - O - Red Foreground Color intensity in a scale from 0 to 1023
 *                       0x0 = No Red Foreground color intensity
 *                       0x3FF = Full Red Foreground color intensity
 * @param   *pGreenFG  - O - Green Foreground Color intensity in a scale from 0 to 1023
 *                       0x0 = No Green Foreground color intensity
 *                       0x3FF = Full Green Foreground color intensity
 * @param   *pBlueFG  - O - Blue Foreground Color intensity in a scale from 0 to 1023
 *                       0x0 = No Blue Foreground color intensity
 *                       0x3FF = Full Blue Foreground color intensity
 * @param   *pRedBG  - O - Red Foreground Color intensity in a scale from 0 to 1023
 *                       0x0 = No Red Foreground color intensity
 *                       0x3FF = Full Red Foreground color intensity
 * @param   *pGreenBG  - O - Green Foreground Color intensity in a scale from 0 to 1023
 *                       0x0 = No Green Foreground color intensity
 *                       0x3FF = Full Red Foreground color intensity
 * @param   *pBlueBG  - O - Red Foreground Color intensity in a scale from 0 to 1023
 *                       0x0 = No Blue Foreground color intensity
 *                       0x3FF = Full Blue Foreground color intensity
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(TPG_COLOR);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pRedFG = msg.text.data[0] | msg.text.data[1] << 8;
        *pGreenFG = msg.text.data[2] | msg.text.data[3] << 8;
        *pBlueFG = msg.text.data[4] | msg.text.data[5] << 8;
        *pRedBG = msg.text.data[6] | msg.text.data[7] << 8;
        *pGreenBG = msg.text.data[8] | msg.text.data[9] << 8;
        *pBlueBG = msg.text.data[10] | msg.text.data[11] << 8;

        return 0;
    }
    return -1;
}

int DLPC900_ClearPatLut(void)
/**
 * This API does not send any commands to the controller.It clears the locally (in the GUI program) stored pattern LUT.
 * See table 2-65 in programmer's guide for detailed desciprtion of pattern LUT entries.
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    PatLutIndex = 0;
    return 0;
}

int DLPC900_AddToPatLut(int patNum, int ExpUs, bool ClearPat, int BitDepth, int LEDSelect, bool WaitForTrigger, int DarkTime, bool TrigOut2, int SplashIndex, int BitIndex)
/**
 * This API does not send any commands to the controller.
 * It makes an entry (appends) in the locally stored (in the GUI program) pattern LUT as per the input arguments passed to this function.
 * See table 2-65 in programmer's guide for detailed desciprtion of pattern LUT entries.
 *
 * @param   TrigType  - I - Select the trigger type for the pattern
 *                          0 = Internal trigger
 *                          1 = External positive
 *                          2 = External negative
 *                          3 = No Input Trigger (Continue from previous; Pattern still has full exposure time)
 *                       0x3FF = Full Red Foreground color intensity
 * @param   PatNum  - I - Pattern number (0 based index). For pattern number 0x3F, there is no
 *                          pattern display. The maximum number supported is 24 for 1 bit-depth
 *                          patterns. Setting the pattern number to be 25, with a bit-depth of 1 will insert
 *                          a white-fill pattern. Inverting this pattern will insert a black-fill pattern. These w
 *                          patterns will have the same exposure time as defined in the Pattern Display
 *                          Exposure and Frame Period command. Table 2-66 in the programmer's guide illustrates which bit
 *                          planes are illuminated by each pattern number.
 * @param   BitDepth  - I - Select desired bit-depth
 *                          0 = Reserved
 *                          1 = 1-bit
 *                          2 = 2-bit
 *                          3 = 3-bit
 *                          4 = 4-bit
 *                          5 = 5-bit
 *                          6 = 6-bit
 *                          7 = 7-bit
 *                          8 = 8-bit
 * @param   LEDSelect  - I -  Choose the LEDs that are on: b0 = Red, b1 = Green, b2 = Blue
 *                          0 = No LED (Pass Through)
 *                          1 = Red
 *                          2 = Green
 *                          3 = Yellow (Green + Red)
 *                          4 = Blue
 *                          5 = Magenta (Blue + Red)
 *                          6 = Cyan (Blue + Green)
 *                          7 = White (Red + Blue + Green)
 * @param   InvertPat  - I - true = Invert pattern
 *                           false = do not invert pattern
 * @param   InsertBlack  - I - true = Insert black-fill pattern after current pattern. This setting requires 230 μs
 *                                      of time before the start of the next pattern
 *                           false = do not insert any post pattern
 * @param   BufSwap  - I - true = perform a buffer swap
 *                           false = do not perform a buffer swap
 * @param   trigOutPrev  - I - true = Trigger Out 1 will continue to be high. There will be no falling edge
 *                                       between the end of the previous pattern and the start of the current pattern. w
 *                                       Exposure time is shared between all patterns defined under a common
 *                                       trigger out). This setting cannot be combined with the black-fill pattern
 *                           false = Trigger Out 1 has a rising edge at the start of a pattern, and a falling
 *                                       edge at the end of the pattern
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    unsigned char lutByte = 0;
    unsigned short lutShort = 0;

    if( (BitDepth > 8) || (BitDepth < 1))
        return -1;

    if(LEDSelect > 7)
        return -1;

    PatLut[0][PatLutIndex] = patNum & 0xFF;
    PatLut[1][PatLutIndex] = (patNum >> 8) & 0xFF;
    PatLut[2][PatLutIndex] = ExpUs & 0xFF;
    PatLut[3][PatLutIndex] = (ExpUs >> 8) & 0xFF;
    PatLut[4][PatLutIndex] = (ExpUs >> 16) & 0xFF;

    if (ClearPat)
        lutByte = BIT0;
    lutByte |= ((BitDepth - 1) & 0x7) << 1;
    lutByte |= (LEDSelect & 0x7) << 4;
    if(WaitForTrigger)
        lutByte |= BIT7;

    PatLut[5][PatLutIndex] = lutByte;
    PatLut[6][PatLutIndex] = DarkTime & 0xFF;
    PatLut[7][PatLutIndex] = (DarkTime >> 8) & 0xFF;
    PatLut[8][PatLutIndex] = (DarkTime >> 16) & 0xFF;

    lutByte = 0;
    if (!TrigOut2)
        lutByte = BIT0;

    PatLut[9][PatLutIndex] = lutByte;

    lutShort = SplashIndex & 0x7FF;
    lutShort |= (BitIndex & 0x1F) << 11;

    PatLut[10][PatLutIndex] = lutShort & 0xFF;
    PatLut[11][PatLutIndex] = (lutShort >> 8) & 0xFF;

    PatLutIndex++;

    return 0;
}


int DLPC900_OpenMailbox(int MboxNum)
/**
 * (I2C: 0x77)
 * (USB: CMD2: 0x1A, CMD3: 0x33)
 * This API opens the specified Mailbox within the DLPC350 controller. This API must be called
 * before sending data to the mailbox/LUT using DLPC900_SendPatLut() or DLPC900_SendSplashLut() APIs.
 *
 * @param MboxNum - I - 1 = Open the mailbox for image index configuration
 *                      2 = Open the mailbox for pattern definition.
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = MboxNum;
    DLPC900_PrepWriteCmd(&msg, MBOX_CONTROL);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_CloseMailbox(void)
/**
 * (I2C: 0x77)
 * (USB: CMD2: 0x1A, CMD3: 0x33)
 * This API is internally used by other APIs within this file. There is no need for user application to
 * call this API separately.
 * This API closes all the Mailboxes within the DLPC350 controller.
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = 0;
    DLPC900_PrepWriteCmd(&msg, MBOX_CONTROL);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_MailboxSetAddr(int Addr)
/**
 * (I2C: 0x76)
 * (USB: CMD2: 0x1A, CMD3: 0x32)
 * This API defines the offset location within the DLPC350 mailboxes to write data into or to read data from
 *
 * @param Addr - I - 0-127 - Defines the offset within the selected (opened) LUT to write/read data to/from.
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    if(Addr > 127)
        return -1;

    msg.text.data[2] = Addr;
    msg.text.data[3] = 0x0;
    DLPC900_PrepWriteCmd(&msg, MBOX_ADDRESS);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_SendPatLut(void)
/**
 * (I2C: 0x78)
 * (USB: CMD2: 0x1A, CMD3: 0x34)
 * This API sends the pattern LUT created by calling DLPC900_AddToPatLut() API to the DLPC350 controller.
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    unsigned int i;
    int ret;

    for(i=0; i<PatLutIndex; i++)
    {
        hidMessageStruct msg;

        CmdList[MBOX_DATA].len = 12;
        DLPC900_PrepWriteCmd(&msg, MBOX_DATA);

        for (int j = 0; j < 12; j++)
            msg.text.data[2 + j] = PatLut[j][i];

        if (DLPC900_SendMsg(&msg) < 0)
            return -2;
    }

    return 0;
}

int DLPC900_PatternDisplay(int Action)
/**
 * (I2C: 0x65)
 * (USB: CMD2: 0x1A, CMD3: 0x24)
 * This API starts or stops the programmed patterns sequence.
 *
 * @param   Action - I - Pattern Display Start/Stop Pattern Sequence
 *                          0 = Stop Pattern Display Sequence. The next "Start" command will
 *                              restart the pattern sequence from the beginning.
 *                          1 = Pause Pattern Display Sequence. The next "Start" command will
 *                              start the pattern sequence by re-displaying the current pattern in the sequence.
 *                          2 = Start Pattern Display Sequence
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = Action;
    DLPC900_PrepWriteCmd(&msg, PAT_START_STOP);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_SetPatternConfig(unsigned int numLutEntries, unsigned int repeat)
/* (I2C: 0x75)
 * (USB: CMD2: 0x1A, CMD3: 0x31)
 * This API controls the execution of patterns stored in the lookup table.
 * Before using this API, stop the current pattern sequence using DLPC900_PatternDisplay() API
 * After calling this API, send the Validation command using the API DLPC900_ValidatePatLutData() before starting the pattern sequence
 *
 * @param   numLutEntries - I - Number of LUT entries
 * @param   repeat - I - 0 = execute the pattern sequence once; 1 = repeat the pattern sequnce.
 * @param   numPatsForTrigOut2 - I - Number of patterns to display(range 1 through 256).
 *                                   If in repeat mode, then this value dictates how often TRIG_OUT_2 is generated.
 * @param   numSplash - I - Number of Image Index LUT Entries(range 1 through 64).
 *                          This Field is irrelevant for Pattern Display Data Input Source set to a value other than internal.
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = numLutEntries & 0xFF;
    msg.text.data[3] = (numLutEntries >> 8) & 0xFF;
    msg.text.data[4] = repeat & 0xFF;
    msg.text.data[5] = (repeat >> 8) & 0xFF;
    msg.text.data[6] = (repeat >> 16) & 0xFF;
    msg.text.data[7] = (repeat >> 24) & 0xFF;
    DLPC900_PrepWriteCmd(&msg, PAT_CONFIG);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetPatternConfig(unsigned int *pNumLutEntries, bool *pRepeat, unsigned int *pNumPatsForTrigOut2, unsigned int *pNumSplash)
/**
 * (I2C: 0x75)
 * (USB: CMD2: 0x1A, CMD3: 0x31)
 * This API controls the execution of patterns stored in the lookup table.
 * Before using this API, stop the current pattern sequence using DLPC900_PatternDisplay() API
 * After calling this API, send the Validation command using the API DLPC900_ValidatePatLutData() before starting the pattern sequence
 *
 * @param   *pNumLutEntries - O - Number of LUT entries
 * @param   *pRepeat - O - 0 = execute the pattern sequence once; 1 = repeat the pattern sequnce.
 * @param   *pNumPatsForTrigOut2 - O - Number of patterns to display(range 1 through 256).
 *                                   If in repeat mode, then this value dictates how often TRIG_OUT_2 is generated.
 * @param   *pNumSplash - O - Number of Image Index LUT Entries(range 1 through 64).
 *                          This Field is irrelevant for Pattern Display Data Input Source set to a value other than internal.
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(PAT_CONFIG);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pNumLutEntries = msg.text.data[0] + 1; /* +1 because the firmware gives 0-based indices (0 means 1) */
        *pRepeat = (msg.text.data[1] != 0);
        *pNumPatsForTrigOut2 = msg.text.data[2]+1;    /* +1 because the firmware gives 0-based indices (0 means 1) */
        *pNumSplash = msg.text.data[3]+1;     /* +1 because the firmware gives 0-based indices (0 means 1) */
        return 0;
    }
    return -1;
}

int DLPC900_SetTrigIn1Config(bool invert, unsigned int trigDelay)
/**
 * (I2C: 0x79)
 * (USB: CMD2: 0x1A, CMD3: 0x35)
 * The Trigger In1 command sets the rising edge delay of the DLPC900's TRIG_IN_1 signal compared to
 * when the pattern is displayed on the DMD. The polarity of TRIG_IN_1 is set in the lookup table of the
 * pattern sequence. Before executing this command, stop the current pattern sequence.
 *
 * @param   invert - I - Pattern advance on rising/falling edge
 * @param   trigDelay - I - Trigger 1 delay in micro seconds.
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = trigDelay & 0xFF;
    msg.text.data[3] = (trigDelay>>8) & 0xFF;
    msg.text.data[4] = invert;
    DLPC900_PrepWriteCmd(&msg, TRIG_IN1_CTL);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetTrigIn1Config(bool *pInvert, unsigned int *pTrigDelay)
/**
 * (I2C: 0x79)
 * (USB: CMD2: 0x1A, CMD3: 0x35)
 * The Trigger In1 command sets the rising edge delay of the DLPC900's TRIG_IN_1 signal compared to
 * when the pattern is displayed on the DMD. The polarity of TRIG_IN_1 is set in the lookup table of the
 * pattern sequence. Before executing this command, stop the current pattern sequence.
 *
 * @param   pInvert - I - Pattern advance on rising/falling edge
 * @param   pTrigDelay - I - Trigger 1 delay in micro seconds.
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(TRIG_IN1_CTL);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        
        *pTrigDelay = msg.text.data[0] | msg.text.data[1] << 8;
        *pInvert = msg.text.data[2];
        return 0;
    }
    return -1;
}

int DLPC900_SetTrigIn2Config(bool invert)
/**
 * (I2C: 0x7A)
 * (USB: CMD2: 0x1A, CMD3: 0x36)
 * In Video Pattern and Pre-Stored Pattern modes, TRIG_IN_2 acts as a start or stop signal. If the sequence
 * was not already started already by a software command, the rising edge on TRIG_IN_2 signal input will
 * start or resume the pattern sequence. If the pattern sequence is active, the falling edge on TRIG_IN_2
 * signal input stops the pattern sequence. Before executing this command, stop the current pattern
 * sequence.
 *
 * @param   invert - 0 – Pattern started on rising edge stopped on falling edge
 *                   1 – Pattern started on falling edge stopped on rising edge
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = invert;
    DLPC900_PrepWriteCmd(&msg, TRIG_IN2_CTL);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetTrigIn2Config(bool *pInvert)
/**
 * (I2C: 0x7A)
 * (USB: CMD2: 0x1A, CMD3: 0x36)
 * In Video Pattern and Pre-Stored Pattern modes, TRIG_IN_2 acts as a start or stop signal. If the sequence
 * was not already started already by a software command, the rising edge on TRIG_IN_2 signal input will
 * start or resume the pattern sequence. If the pattern sequence is active, the falling edge on TRIG_IN_2
 * signal input stops the pattern sequence. Before executing this command, stop the current pattern
 * sequence.
 *
 * @param   invert - 0 – Pattern started on rising edge stopped on falling edge
 *                   1 – Pattern started on falling edge stopped on rising edge
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(TRIG_IN2_CTL);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);

        *pInvert = msg.text.data[0];
        return 0;
    }
    return -1;
}

int DLPC900_SetTrigOutConfig(unsigned int trigOutNum, bool invert, short rising, short falling)
/**
 * (I2C: 0x6A)
 * (USB: CMD2: 0x1A, CMD3: 0x1D)
 * This API sets the polarity, rising edge delay, and falling edge delay of the DLPC350's TRIG_OUT_1 or TRIG_OUT_2 signal.
 * The delays are compared to when the pattern is displayed on the DMD. Before executing this command,
 * stop the current pattern sequence. After executing this command, call DLPC900_ValidatePatLutData() API before starting the pattern sequence.
 *
 * @param   trigOutNum - I - 1 = TRIG_OUT_1; 2 = TRIG_OUT_2
 * @param   invert - I - 0 = active High signal; 1 = Active Low signal
 * @param   rising - I - rising edge delay control. Each bit adds 107.2 ns
 *                      0x00 = -20.05 μs, 0x01 = -19.9428 μs, ......0xBB=0.00 μs, ......, 0xD4 = +2.68 μs, 0xD5 = +2.787 μs
 * @param   falling- I - falling edge delay control. Each bit adds 107.2 ns (This field is not applcable for TRIG_OUT_2)
 *                      0x00 = -20.05 μs, 0x01 = -19.9428 μs, ......0xBB=0.00 μs, ......, 0xD4 = +2.68 μs, 0xD5 = +2.787 μs
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = invert & 0x1;
    msg.text.data[3] = rising & 0xFF;
    msg.text.data[4] = (rising >> 8) & 0xFF;
    msg.text.data[5] = falling & 0xFF;
    msg.text.data[6] = (falling >> 8) & 0xFF;

    if(trigOutNum == 1)
        DLPC900_PrepWriteCmd(&msg, TRIG_OUT1_CTL);
    else if(trigOutNum==2)
        DLPC900_PrepWriteCmd(&msg, TRIG_OUT2_CTL);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetTrigOutConfig(unsigned int trigOutNum, bool *pInvert, short *pRising, short *pFalling)
/**
 * (I2C: 0x6A)
 * (USB: CMD2: 0x1A, CMD3: 0x1D)
 * This API readsback the polarity, rising edge delay, and falling edge delay of the DLPC350's TRIG_OUT_1 or TRIG_OUT_2 signal.
 * The delays are compared to when the pattern is displayed on the DMD.
 *
 * @param   trigOutNum - I - 1 = TRIG_OUT_1; 2 = TRIG_OUT_2
 * @param   *pInvert - O - 0 = active High signal; 1 = Active Low signal
 * @param   *pRising - O - rising edge delay control. Each bit adds 107.2 ns
 *                      0x00 = -20.05 μs, 0x01 = -19.9428 μs, ......0xBB=0.00 μs, ......, 0xD4 = +2.68 μs, 0xD5 = +2.787 μs
 * @param   *pFalling- O - falling edge delay control. Each bit adds 107.2 ns (This field is not applcable for TRIG_OUT_2)
 *                      0x00 = -20.05 μs, 0x01 = -19.9428 μs, ......0xBB=0.00 μs, ......, 0xD4 = +2.68 μs, 0xD5 = +2.787 μs
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    if(trigOutNum == 1)
        DLPC900_PrepReadCmd(TRIG_OUT1_CTL);
    else if(trigOutNum==2)
        DLPC900_PrepReadCmd(TRIG_OUT2_CTL);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pInvert = (msg.text.data[0] != 0);
        *pRising = msg.text.data[1] | (msg.text.data[2] << 8);//| (0xff << 16) | (0xff << 24);
        *pFalling = msg.text.data[3] | (msg.text.data[4] << 8);// | (0xff << 16) | (0xff << 24);

        return 0;
    }
    return -1;
}


int DLPC900_SetInvertData(bool invert)
/**
 * (I2C: 0x74)
 * (USB: CMD2: 0x1A, CMD3: 0x30)
 * This API dictates how the DLPC350 interprets a value of 0 or 1 to control mirror position for displayed patterns.
 * Before executing this command, stop the current pattern sequence. After executing this command, call
 * DLPC900_ValidatePatLutData() API before starting the pattern sequence.
 *
 * @param   invert - I - Pattern Display Invert Data
 *                      0 = Normal operation. A data value of 1 will flip the mirror to output light,
 *                          while a data value of 0 will flip the mirror to block light
 *                      1 = Inverted operation. A data value of 0 will flip the mirror to output light,
 *                          while a data value of 1 will flip the mirror to block light
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = invert;
    DLPC900_PrepWriteCmd(&msg, INVERT_DATA);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetInvertData(bool *pInvert)
/**
 * (I2C: 0x74)
 * (USB: CMD2: 0x1A, CMD3: 0x30)
 * This API dictates how the DLPC350 interprets a value of 0 or 1 to control mirror position for displayed patterns.
 * Before executing this command, stop the current pattern sequence. After executing this command, call
 * DLPC900_ValidatePatLutData() API before starting the pattern sequence.
 *
 * @param   invert - I - Pattern Display Invert Data
 *                      0 = Normal operation. A data value of 1 will flip the mirror to output light,
 *                          while a data value of 0 will flip the mirror to block light
 *                      1 = Inverted operation. A data value of 0 will flip the mirror to output light,
 *                          while a data value of 1 will flip the mirror to block light
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;
    DLPC900_PrepReadCmd(INVERT_DATA);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pInvert = msg.text.data[0];
        return 0;
    }
    return -1;
}

int DLPC900_SetPWMConfig(unsigned int channel, unsigned int pulsePeriod, unsigned int dutyCycle)
/**
 * (I2C: 0x41)
 * (USB: CMD2: 0x1A, CMD3: 0x11)
 * This API sets the clock period and duty cycle of the specified PWM channel. The PWM
 * frequency and duty cycle is derived from an internal 18.67MHz clock. To calculate the desired PWM
 * period, divide the desired clock frequency from the internal 18.67Mhz clock. For example, a PWM
 * frequency of 2kHz, requires pulse period to be set to 18666667 / 2000 = 9333.
 *
 * @param   channel - I - PWM Channel Select
 *                      0 - PWM channel 0 (GPIO_0)
 *                      1 - Reserved
 *                      2 - PWM channel 2 (GPIO_2)
 *
 * @param   pulsePeriod - I - Clock Period in increments of 53.57ns. Clock Period = (value + 1) * 53.5ns
 *
 * @param   dutyCycle - I - Duty Cycle = (value + 1)% Value range is 1%-99%
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = channel;
    msg.text.data[3] = pulsePeriod;
    msg.text.data[4] = pulsePeriod >> 8;
    msg.text.data[5] = pulsePeriod >> 16;
    msg.text.data[6] = pulsePeriod >> 24;
    msg.text.data[7] = dutyCycle;

    DLPC900_PrepWriteCmd(&msg, PWM_SETUP);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetPWMConfig(unsigned int channel, unsigned int *pPulsePeriod, unsigned int *pDutyCycle)
/**
 * (I2C: 0x41)
 * (USB: CMD2: 0x1A, CMD3: 0x11)
 * This API reads back the clock period and duty cycle of the specified PWM channel. The PWM
 * frequency and duty cycle is derived from an internal 18.67MHz clock. To calculate the desired PWM
 * period, divide the desired clock frequency from the internal 18.67Mhz clock. For example, a PWM
 * frequency of 2kHz, requires pulse period to be set to 18666667 / 2000 = 9333.
 *
 * @param   channel - I - PWM Channel Select
 *                      0 - PWM channel 0 (GPIO_0)
 *                      1 - Reserved
 *                      2 - PWM channel 2 (GPIO_2)
 *
 * @param   *pPulsePeriod - O - Clock Period in increments of 53.57ns. Clock Period = (value + 1) * 53.5ns
 *
 * @param   *pDutyCycle - O - Duty Cycle = (value + 1)% Value range is 1%-99%
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmdWithParam(PWM_SETUP, (unsigned char)channel);
    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pPulsePeriod = msg.text.data[1] | msg.text.data[2] << 8 | msg.text.data[3] << 16 | msg.text.data[4] << 24;
        *pDutyCycle = msg.text.data[5];
        return 0;
    }
    return -1;
}

int DLPC900_SetPWMEnable(unsigned int channel, bool Enable)
/**
 * (I2C: 0x40)
 * (USB: CMD2: 0x1A, CMD3: 0x10)
 * After the PWM Setup command configures the clock period and duty cycle, the PWM Enable command
 * activates the PWM signals.
 *
 * @param   channel - I - PWM Channel Select
 *                      0 - PWM channel 0 (GPIO_0)
 *                      1 - PWM channel 1 (GPIO_1)
 *                      2 - PWM channel 2 (GPIO_2)
 *                      3 - PWM channel 3 (GPIO_3)
 *
 * @param   Enable - I - PWM Channel enable 0=disable; 1=enable
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;
    unsigned char value = 0;

    if (channel > 3)
        return -1;
    else
        value = channel;

    if(Enable)
        value |= BIT7;

    msg.text.data[2] = value;
    DLPC900_PrepWriteCmd(&msg, PWM_ENABLE);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetPWMEnable(unsigned int channel, bool *pEnable)
/**
 * (I2C: 0x40)
 * (USB: CMD2: 0x1A, CMD3: 0x10)
 * Reads back the enabled/disabled status of the given PWM channel.
 *
 * @param   channel - I - PWM Channel Select
 *                      0 - PWM channel 0 (GPIO_0)
 *                      1 - PWM channel 1 (GPIO_1)
 *                      2 - PWM channel 2 (GPIO_2)
 *                      3 - PWM channel 3 (GPIO_3)
 *
 * @param   *pEnable - O - PWM Channel enable 0=disable; 1=enable
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmdWithParam(PWM_ENABLE, (unsigned char)channel);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        if(msg.text.data[0] & BIT7)
            *pEnable =  true;
        else
            *pEnable = false;

        return 0;
    }
    return -1;
}

int DLPC900_SetPWMCaptureConfig(unsigned int channel, bool enable, unsigned int sampleRate)
/**
 * (I2C: 0x43)
 * (USB: CMD2: 0x1A, CMD3: 0x12)
 * This API samples the specified PWM input signals and returns the PWM clock period.
 *
 * @param   channel - I - PWM Capture Port
 *                      0 - PWM input channel 0 (GPIO_5)
 *                      1 - PWM input channel 1 (GPIO_6)
 *
 * @param   enable - I - PWM Channel enable 0=disable; 1=enable
 *
 * @param   sampleRate - I - PWM Sample Rate (285 Hz to 18,666,667 Hz) - Sample Rate = Pulse Frequency / Duty Cycle
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;
    unsigned char value = 0;

    value = channel & 1;

    if(enable)
        value |= BIT7;

    msg.text.data[2] = value;
    msg.text.data[3] = sampleRate;
    msg.text.data[4] = sampleRate >> 8;
    msg.text.data[5] = sampleRate >> 16;
    msg.text.data[6] = sampleRate >> 24;
    DLPC900_PrepWriteCmd(&msg, PWM_CAPTURE_CONFIG);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetPWMCaptureConfig(unsigned int channel, bool *pEnabled, unsigned int *pSampleRate)
/**
 * (I2C: 0x43)
 * (USB: CMD2: 0x1A, CMD3: 0x12)
 * This API reads back the configuration of the specified PWM capture channel.
 *
 * @param   channel - I - PWM Capture Port
 *                      0 - PWM input channel 0 (GPIO_5)
 *                      1 - PWM input channel 1 (GPIO_6)
 *
 * @param   *pEnabled - O - PWM Channel enable 0=disable; 1=enable
 *
 * @param   *pSampleRate - O - PWM Sample Rate (285 Hz to 18,666,667 Hz) - Sample Rate = Pulse Frequency / Duty Cycle
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmdWithParam(PWM_CAPTURE_CONFIG, (unsigned char)channel);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        if(msg.text.data[0] & BIT7)
            *pEnabled =  true;
        else
            *pEnabled = false;

        *pSampleRate = msg.text.data[1] | msg.text.data[2] << 8 | msg.text.data[3] << 16 | msg.text.data[4] << 24;

        return 0;
    }
    return -1;
}

int DLPC900_PWMCaptureRead(unsigned int channel, unsigned int *pLowPeriod, unsigned int *pHighPeriod)
/**
 * (I2C: 0x4E)
 * (USB: CMD2: 0x1A, CMD3: 0x13)
 * This API returns both the number of clock cycles the signal was low and high.
 *
 * @param   channel - I - PWM Capture Port
 *                      0 - PWM input channel 0 (GPIO_5)
 *                      1 - PWM input channel 1 (GPIO_6)
 *
 * @param   *pLowPeriod - O - indicates how many samples were taken during a low signal
 *
 * @param   *pHighPeriod - O - indicates how many samples were taken during a high signal
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmdWithParam(PWM_CAPTURE_READ, (unsigned char)channel);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pLowPeriod = msg.text.data[1] | msg.text.data[2] << 8;
        *pHighPeriod = msg.text.data[3] | msg.text.data[4] << 8;
        return 0;
    }
    return -1;
}

int DLPC900_SetGPIOConfig(unsigned int pinNum, bool dirOutput, bool outTypeOpenDrain, bool pinState)
/**
 * (I2C: 0x44)
 * (USB: CMD2: 0x1A, CMD3: 0x38)
 *
 * This API enables GPIO functionality on a specific set of DLPC350 pins. The
 * command sets their direction, output buffer type, and output state.
 *
 * @param   pinNum - I - GPIO selection. See Table 2-38 in the programmer's guide for description of available pins
 *
 * @param   dirOutput - I - 0=input; 1=output
 *
 * @param   outTypeOpenDrain - I - 0=Standard buffer (drives high or low); 1=open drain buffer (drives low only)
 *
 * @param   pinState - I - 0=LOW; 1=HIGH
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;
    unsigned char value = 0;

    if(dirOutput)
        value |= BIT1;
    if(outTypeOpenDrain)
        value |= BIT2;
    if(pinState)
        value |= BIT0;

    msg.text.data[2] = pinNum;
    msg.text.data[3] = value;
    DLPC900_PrepWriteCmd(&msg, GPIO_CONFIG);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetGPIOConfig(unsigned int pinNum, bool *pDirOutput, bool *pOutTypeOpenDrain, bool *pState)
/**
 * (I2C: 0x44)
 * (USB: CMD2: 0x1A, CMD3: 0x38)
 *
 * This API reads back the GPIO configuration on a specific set of DLPC350 pins. The
 * command reads back their direction, output buffer type, and  state.
 *
 * @param   pinNum - I - GPIO selection. See Table 2-38 in the programmer's guide for description of available pins
 *
 * @param   *pDirOutput - O - 0=input; 1=output
 *
 * @param   *pOutTypeOpenDrain - O - 0=Standard buffer (drives high or low); 1=open drain buffer (drives low only)
 *
 * @param   *pState - O - 0=LOW; 1=HIGH
 *
 * @return  0 = PASS    <BR>
 *          -1 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmdWithParam(GPIO_CONFIG, (unsigned char)pinNum);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pDirOutput = ((msg.text.data[0] & BIT1) == BIT1);
        *pOutTypeOpenDrain = ((msg.text.data[0] & BIT2) == BIT2);
        *pState = ((msg.text.data[0] & BIT0) == BIT0);

        return 0;
    }
    return -1;
}

int DLPC900_SetGeneralPurposeClockOutFreq(unsigned int clkId, bool enable, unsigned int clkDivider)
/**
 * (I2C: 0x48)
 * (USB: CMD2: 0x08, CMD3: 0x07)
 *
 * DLPC350 supports two pins with clock output capabilities: GPIO_11 and GPIO_12.
 * This API enables the clock output functionality and sets the clock frequency.
 *
 * @param   clkId - I - Clock selection. 1=GPIO_11; 2=GPIO_12
 *
 * @param   enable - I - 0=disable clock functionality on selected pin; 1=enable clock functionality on selected pin
 *
 * @param   clkDivider - I - Allowed values in the range of 2 to 127. Output frequency = 96MHz / (Clock Divider)
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = clkId;
    msg.text.data[3] = enable;
    msg.text.data[4] = clkDivider;
    DLPC900_PrepWriteCmd(&msg, GPCLK_CONFIG);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetGeneralPurposeClockOutFreq(unsigned int clkId, bool *pEnabled, unsigned int *pClkDivider)
/**
 * (I2C: 0x48)
 * (USB: CMD2: 0x08, CMD3: 0x07)
 *
 * DLPC350 supports two pins with clock output capabilities: GPIO_11 and GPIO_12.
 * This API reads back the clock output enabled status and the clock frequency.
 *
 * @param   clkId - I - Clock selection. 1=GPIO_11; 2=GPIO_12
 *
 * @param   *pEnabled - O - 0=disable clock functionality on selected pin; 1=enable clock functionality on selected pin
 *
 * @param   *pClkDivider - O - Allowed values in the range of 2 to 127. Output frequency = 96MHz / (Clock Divider)
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmdWithParam(GPCLK_CONFIG, (unsigned char)clkId);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pEnabled = msg.text.data[1];
        *pClkDivider = msg.text.data[2];
        return 0;
    }
    return -1;
}

int DLPC900_SetLEDPWMInvert(bool invert)
/**
 * (I2C: 0x0B)
 * (USB: CMD2: 0x1A, CMD3: 0x05)
 *
 * This API sets the polarity of all LED PWM signals. This API must be called before powering up the LED drivers.
 *
 * @param   invert - I - 0 = Normal polarity, PWM 0 value corresponds to no current while PWM 255 value corresponds to maximum current
 *                       1 = Inverted polarity. PWM 0 value corresponds to maximum current while PWM 255 value corresponds to no current.
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.text.data[2] = invert;
    DLPC900_PrepWriteCmd(&msg, PWM_INVERT);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_GetLEDPWMInvert(bool *inverted)
/**
 * (I2C: 0x0B)
 * (USB: CMD2: 0x1A, CMD3: 0x05)
 *
 * This API reads the polarity of all LED PWM signals.
 *
 * @param   invert - O - 0 = Normal polarity, PWM 0 value corresponds to no current while PWM 255 value corresponds to maximum current
 *                       1 = Inverted polarity. PWM 0 value corresponds to maximum current while PWM 255 value corresponds to no current.
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(PWM_INVERT);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *inverted = (msg.text.data[0] != 0);
        return 0;
    }
    return -1;
}

/* int DLPC900_SetI2CPassThrough(unsigned int port, unsigned int addm, unsigned int clk, unsigned int devadd, unsigned char* wdata, unsigned int nwbytes)
 /**
  * (I2C: 0x7E)
  * (USB: CMD2: 0x1A, CMD3: 0x4F)
  *
  * @param
  *
  * @return  >=0 = PASS    <BR>
  *          <0 = FAIL  <BR>
  *
  *
 {
     int i,x=0;
     hidMessageStruct msg;

     msg.head.flags.rw = 0; //Write
     msg.head.flags.reply = 0; //Host wants a reply from device
     msg.head.flags.dest = 0; //Projector Control Endpoint
     msg.head.flags.reserved = 0;
     msg.head.flags.nack = 0;
     msg.head.seq = seqNum++;
     msg.text.cmd = (CmdList[I2C_PASSTHRU].CMD2 << 8) | CmdList[I2C_PASSTHRU].CMD3;
     msg.head.length = CmdList[I2C_PASSTHRU].len + nwbytes + 2;

     msg.text.data[2] = (((addm & 0x01) << 4) | (port & 0x03));
     msg.text.data[3] = (devadd & 0xff);
     msg.text.data[4] = (unsigned char)(nwbytes);
     msg.text.data[5] = (unsigned char)(nwbytes >> 8);
     msg.text.data[6] = (unsigned char)(clk);
     msg.text.data[7] = (unsigned char)(clk >> 8);
     msg.text.data[8] = (unsigned char)(clk >> 16);
     msg.text.data[9] = (unsigned char)(clk >> 24);

     for ( i = 10; i < (10 + nwbytes); i++)
     {
         msg.text.data[i] = wdata[x++];
     }

     return DLPC900_SendMsg(&msg);
 }

 int DLPC900_GetI2CPassThrough(unsigned int port, unsigned int addm, unsigned int clk, unsigned int devadd, unsigned char* wdata, unsigned int nwbytes, unsigned int nrbytes, unsigned char *rdata)
 /**
  * (I2C: 0x7E)
  * (USB: CMD2: 0x1A, CMD3: 0x4F)
  *
  * @param
  *
  * @return  >=0 = PASS    <BR>
  *          <0 = FAIL  <BR>
  *
  *
 {
     int i,x=0;
     hidMessageStruct msg;

     msg.head.flags.rw = 1; //Read
     msg.head.flags.reply = 1; //Host wants a reply from device
     msg.head.flags.dest = 0; //Projector Control Endpoint
     msg.head.flags.reserved = 0;
     msg.head.flags.nack = 0;
     msg.head.seq = 0;

     msg.text.cmd = (CmdList[I2C_PASSTHRU].CMD2 << 8) | CmdList[I2C_PASSTHRU].CMD3;
     msg.head.length = 10 + nwbytes + 2;

     msg.text.data[2]  = (((addm & 0x01) << 4) | (port & 0x03));
     msg.text.data[3]  = (devadd & 0xff);
     msg.text.data[4]  = (unsigned char)(nwbytes);
     msg.text.data[5]  = (unsigned char)(nwbytes >> 8);
     msg.text.data[6]  = (unsigned char)(nrbytes);
     msg.text.data[7]  = (unsigned char)(nrbytes >> 8);
     msg.text.data[8]  = (unsigned char)(clk);
     msg.text.data[9]  = (unsigned char)(clk >> 8);
     msg.text.data[10] = (unsigned char)(clk >> 16);
     msg.text.data[11] = (unsigned char)(clk >> 24);

     if ( nwbytes > 0 )
     {
         for ( i = 12; i < (12 + nwbytes); i++)
         {
             msg.text.data[i] = wdata[x++];
         }
     }

     OutputBuffer[0]=0; // First byte is the report number
     memcpy(&OutputBuffer[1], &msg, (sizeof(msg.head)+ msg.head.length));

     if(DLPC900_Read() > 0)
     {
         memcpy(&msg, InputBuffer, 65);
         nrbytes &= 0x3F;
         for( i = 0; i < nrbytes; i++ )
            rdata[i] = msg.text.data[i];

         return 0;
     }

     return -1;
 }*/

int DLPC900_I2CConfigure(unsigned int port, unsigned int addm, unsigned int clk)
/**
 * (I2C: 0x4E)
 * (USB: CMD2: 0x1A, CMD3: 0x4E)
 *
 * @param
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    msg.head.flags.rw = 0; //Write
    msg.head.flags.reply = 0; //Host wants a reply from device
    msg.head.flags.dest = 0; //Projector Control Endpoint
    msg.head.flags.reserved = 0;
    msg.head.flags.nack = 0;
    msg.head.seq = 0;
    msg.text.cmd = (CmdList[I2C_CONFIG].CMD2 << 8) | CmdList[I2C_CONFIG].CMD3;
    msg.head.length = CmdList[I2C_CONFIG].len + 2;

    msg.text.data[2] = (((addm & 0x01) << 4) | (port & 0x03));
    msg.text.data[3] = (unsigned char)(clk);
    msg.text.data[4] = (unsigned char)(clk >> 8);
    msg.text.data[5] = (unsigned char)(clk >> 16);
    msg.text.data[6] = (unsigned char)(clk >> 24);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_WriteI2CPassThrough(unsigned int port, unsigned int devadd, unsigned char* wdata, unsigned int nwbytes)
/**
 * (I2C: 0x4F)
 * (USB: CMD2: 0x1A, CMD3: 0x4F)
 *
 * @param
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    int i,x=0;
    hidMessageStruct msg;

    msg.head.flags.rw = 0; //Write
    msg.head.flags.reply = 1; //Host wants a reply from device
    msg.head.flags.dest = 0; //Projector Control Endpoint
    msg.head.flags.reserved = 0;
    msg.head.flags.nack = 0;
    msg.head.seq = seqNum++;
    msg.text.cmd = (CmdList[I2C_PASSTHRU].CMD2 << 8) | CmdList[I2C_PASSTHRU].CMD3;
    msg.head.length = CmdList[I2C_PASSTHRU].len + nwbytes + 2;

    msg.text.data[2] = (unsigned char)(nwbytes);
    msg.text.data[3] = (unsigned char)(nwbytes >> 8);
    msg.text.data[4] = (port & 0x03);
    msg.text.data[5] = (unsigned char)(devadd);
    msg.text.data[6] = (unsigned char)(devadd >> 8);

    for ( i = 7; i < (7 + nwbytes); i++)
    {
        msg.text.data[i] = wdata[x++];
    }

    return DLPC900_SendMsg(&msg);
}

int DLPC900_ReadI2CPassThrough(unsigned int port, unsigned int devadd, unsigned char* wdata, unsigned int nwbytes, unsigned int nrbytes, unsigned char* rdata)
/**
 * (I2C: 0x4F)
 * (USB: CMD2: 0x1A, CMD3: 0x4F)
 *
 * @param
 *
 * @return  >=0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    int i,x=0;
    hidMessageStruct msg;

    msg.head.flags.rw = 1; //Read
    msg.head.flags.reply = 1; //Host wants a reply from device
    msg.head.flags.dest = 0; //Projector Control Endpoint
    msg.head.flags.reserved = 0;
    msg.head.flags.nack = 0;
    msg.head.seq = 0;

    msg.text.cmd = (CmdList[I2C_PASSTHRU].CMD2 << 8) | CmdList[I2C_PASSTHRU].CMD3;
    msg.head.length = 7 + nwbytes + 2;

    msg.text.data[2]  = (unsigned char)(nwbytes);
    msg.text.data[3]  = (unsigned char)(nwbytes >> 8);
    msg.text.data[4]  = (unsigned char)(nrbytes);
    msg.text.data[5]  = (unsigned char)(nrbytes >> 8);
    msg.text.data[6]  = (port & 0x03);
    msg.text.data[7]  = (unsigned char)(devadd);
    msg.text.data[8]  = (unsigned char)(devadd >> 8);


    if ( nwbytes > 0 )
    {
        for ( i = 9; i < (9 + nwbytes); i++)
        {
            msg.text.data[i] = wdata[x++];
        }
    }

    OutputBuffer[0]=0; // First byte is the report number
    memcpy(&OutputBuffer[1], &msg, (sizeof(msg.head)+ msg.head.length));

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        nrbytes &= 0x3F;
        for( i = 0; i < nrbytes; i++ )
            rdata[i] = msg.text.data[i];

        return 0;
    }

    return -1;
}

int DLPC900_InitPatternMemLoad(bool master, unsigned short imageNum, unsigned int size)
{
    hidMessageStruct msg;

    msg.text.data[2] = imageNum & 0xFF;
    msg.text.data[3] = (imageNum >> 8) & 0xFF;
    msg.text.data[4] = size & 0xFF;
    msg.text.data[5] = (size >> 8) & 0xFF;
    msg.text.data[6] = (size >> 16) & 0xFF;
    msg.text.data[7] = (size >> 24) & 0xFF;

    if (master)
        DLPC900_PrepWriteCmd(&msg, PATMEM_LOAD_INIT_MASTER);
    else
        DLPC900_PrepWriteCmd(&msg, PATMEM_LOAD_INIT_SLAVE);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_pattenMemLoad(bool master, unsigned char *pByteArray, int size)
{
    hidMessageStruct msg;
    int retval;
    int dataLen;

    dataLen = 512 - sizeof(msg.head)- sizeof(msg.text.cmd) - 2;//The last -2 is to workaround a bug in bootloader.
    
    //    dataLen = 186;

    /*    if (size < dataLen)
    {
      memcpy(&msg.text.data[3], pByteArray, size);
    }
    else
    {
      memcpy(&msg.text.data[3], pByteArray, dataLen);
    }*/

    if (size < dataLen)
        dataLen = size;


    memcpy(&msg.text.data[4], pByteArray, dataLen);
    msg.text.data[2] = dataLen & 0xFF;
    msg.text.data[3] = (dataLen >> 8) & 0xFF;

    if (master)
    {
        CmdList[PATMEM_LOAD_DATA_MASTER].len = dataLen + 2;
        DLPC900_PrepWriteCmd(&msg, PATMEM_LOAD_DATA_MASTER);
    }
    else
    {
        CmdList[PATMEM_LOAD_DATA_SLAVE].len = dataLen + 2;
        DLPC900_PrepWriteCmd(&msg, PATMEM_LOAD_DATA_SLAVE);
    }

    retval = DLPC900_SendMsg(&msg);
    if(retval > 0)
        return dataLen;

    return -1;
}

int DLPC900_getBatchFileName(unsigned char id, char *batchFileName)
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmdWithParam(BATCHFILE_NAME, id);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        for (int i = 0; i < 16; i++)
        {
            if (msg.text.data[i] == 0)
            {
                batchFileName[i] = '\0';
                break;
            }
            batchFileName[i] = msg.text.data[i];
        }
        return 0;
    }
    return -1;
}

int DLPC900_executeBatchFile(unsigned char id)
{
    hidMessageStruct msg;

    msg.text.data[2] = id;
    DLPC900_PrepWriteCmd(&msg, BATCHFILE_EXECUTE);

    return DLPC900_SendMsg(&msg);
}

int DLPC900_enableDebug()
{
    hidMessageStruct msg;

    msg.text.data[2] = 0x0f;
    msg.text.data[3] = 0x00;
    msg.text.data[4] = 0x00;
    msg.text.data[5] = 0x40;
    msg.text.data[6] = 0x01;
    DLPC900_PrepWriteCmd(&msg, DEBUG);

    return DLPC900_SendMsg(&msg);

}

int DLPC900_executeRawCommand(unsigned char *rawCommand, int count)
{

    hidMessageStruct msg;

    msg.head.flags.rw = 0; //Write
    msg.head.flags.reply = 1; //Host wants a reply from device
    msg.head.flags.dest = 0; //Projector Control Endpoint
    msg.head.flags.reserved = 0;
    msg.head.flags.nack = 0;
    msg.head.seq = seqNum++;
    
    msg.text.cmd = (rawCommand[1] << 8) | rawCommand[0];
    for (int i = 2; i < count; i++)
        msg.text.data[i] = rawCommand[i];

    msg.head.length = count;
    

    int maxDataSize = USB_MAX_PACKET_SIZE-sizeof(msg.head);
    int dataBytesSent = MIN(msg.head.length, maxDataSize);    //Send all data or max possible
    bool ackRequired = true;

    /* Disable Read back for BL_DNLD_DATA, BL_CALC_CHKSUM, PATMEM_LOAD_DATAcommands */
    if ((msg.text.cmd == 0x25) || (msg.text.cmd == 0x26) ||  (msg.text.cmd == 0x1A2B) || (msg.text.cmd == 0x1A2D))
    {
        ackRequired = false;
        msg.head.flags.reply = 0;
    }
    
    OutputBuffer[0]=0; // First byte is the report number
    memcpy(&OutputBuffer[1], &msg, (sizeof(msg.head) + dataBytesSent));


    if(DLPC900_Write(ackRequired) < 0)
        return -1;

    while(dataBytesSent < msg.head.length)
    {
        memcpy(&OutputBuffer[1], &msg.text.data[dataBytesSent], USB_MAX_PACKET_SIZE);
        if(DLPC900_Write(ackRequired) < 0)
            return -1;
        dataBytesSent += USB_MAX_PACKET_SIZE;
    }

    return 0;
}

int API_getUSBCommand(char *command, unsigned char *usbCommand)
{
    int i = 0;
    while(true)
    {
        if(!strcmp(command, CmdList[i].name))
        {
            usbCommand[0] = CmdList[i].CMD3;
            usbCommand[1] = CmdList[i].CMD2;
            return 0;
        }
        if(!strcmp("END", CmdList[i].name))
            return -1;
        i++;
    }
}

int API_getI2CCommand(char *command, unsigned char *i2cCommand)
{
    int i = 0;
    while(true)
    {
        if(!strcmp(command, CmdList[i].name))
        {
            *i2cCommand = CmdList[i].I2CCMD;
            return 0;
        }
        if(!strcmp("END", CmdList[i].name))
            return -1;
        i++;
    }
}

int API_getCommandLength(char *command, int *len)
{
    int i = 0;
    while(true)
    {
        if(!strcmp(command, CmdList[i].name))
        {
            *len = CmdList[i].len;
            return 0;
        }
        if(!strcmp("END", CmdList[i].name))
            return -1;
        i++;
    }
}

int API_getCommandName(unsigned char i2cCommand, char **command)
{
    int i = 0;
    while(true)
    {
        if (i2cCommand == CmdList[i].I2CCMD)
        {
            *command = CmdList[i].name;
            return 0;
        }
        if(!strcmp("END", CmdList[i].name))
            return -1;
        i++;
    }
}

int API_getBatchFilePatternDetails(unsigned char *batchBuffer, int size, unsigned short *patternImageList, int *patternImageCount)
{
    int count = 20;
    int listCount = 0, imageCount = 0, ret;
    unsigned short list[400];

    while (count < size)
    {
        char *commandName;
        int commandLen;
        if (API_getCommandName(batchBuffer[count++], &commandName) < 0)
            return -1;
        if (API_getCommandLength(commandName, &commandLen) < 0)
            return -1;
        if (!(strcmp(commandName, CmdList[MBOX_DATA].name)))
        {
            list[listCount++] = (batchBuffer[count + 10] | batchBuffer[count + 11] << 8) & 0x7FF;
        }
        count += commandLen;
    }

    unsigned short prev = -1;
    for (int i = 0; i < listCount; i++)
    {
        if (list[i] == prev)
            continue;
        patternImageList[imageCount++] = list[i];
        prev = list[i];
    }
    *patternImageCount = imageCount;
    return 0;
}

int API_changeImgNoinBatchFile(unsigned char *batchBuffer, int size, int curId, int changeId)
{
    int count = 20, ret;

    while (count < size)
    {
        char *commandName;
        int commandLen;
        if (API_getCommandName(batchBuffer[count++], &commandName) < 0)
            return -1;
        if (API_getCommandLength(commandName, &commandLen) < 0)
            return -1;
        if (!(strcmp(commandName, CmdList[MBOX_DATA].name)))
        {
            unsigned short param = batchBuffer[count + 10] | (batchBuffer[count + 11] << 8);
            unsigned short imageNo = param & 0x7FF;
            if (imageNo == curId)
            {
                param &= 0xF800;
                param |= (changeId & 0x7FF);
                batchBuffer[count + 10] = param & 0xFF;
                batchBuffer[count + 11] = (param >> 8) & 0xFF;
            }
        }
        count += commandLen;
    }

    return 0;
}

int DLPC900_ReadErrorCode(unsigned int *pCode)
/**
 * (I2C: 0x32)
 * (USB: CMD2: 0x01, CMD3: 0x00)
 * Reads the error code for the last executed command.
 *
 * @return  =0 = PASS    <BR>
 *          <0 = FAIL  <BR>
 *
 */
{
    hidMessageStruct msg;

    DLPC900_PrepReadCmd(READ_ERROR_CODE);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 65);
        *pCode = msg.text.data[0];
        return 0;
    }
    return -1;
}

int DLPC900_ReadErrorString(char *errStr)
{
    hidMessageStruct msg;
    unsigned int i;

    DLPC900_PrepReadCmd(READ_ERROR_MSG);

    if(DLPC900_Read() > 0)
    {
        memcpy(&msg, InputBuffer, 128);

        for(i=0;i<128;i++)
        {
          *errStr = msg.text.data[i];
            errStr++;
        }

        return 0;
    }
    return -1;
}
