/*! 
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Copyright(c) 2012 Apogee Imaging Systems, Inc. 
* \class AscentBasedUsbIo 
* \brief usb io class for ascent, alta f and other ascent based cameras  
* 
*/ 

#include "AscentBasedUsbIo.h" 
#include "apgHelper.h" 
#include "IUsb.h"
#include "PromFx2Io.h" 
#include "CameraInfo.h" 
#include <cstring>  //for memset
namespace
{
    // Number of requested bytes in a single transfer - limit imposed by 
    // WinUSB.
    const uint32_t MAX_USB_BUFFER_SIZE =  0x1FF000;

    const uint32_t ASCENT_EEPROM_MAX_BANKS = 2;
    const uint32_t ASCENT_EEPROM_MAX_BLOCKS = 4;

    const uint8_t FPGA_PROM_ADDR = 0;
    const uint8_t FPGA_PROM_BLOCK  = 1;
    const uint8_t FPGA_PROM_BANK = 0;

    const uint16_t FX2_PROM_ADDR = 0;
    const uint8_t FX2_PROM_BLOCK = 0;
    const uint8_t FX2_PROM_BANK = 0;

    const uint8_t DSCR_PROM_BANK = 0;
    const uint8_t DSCR_PROM_BLOCK = 0;
    const uint16_t DSCR_PROM_ADDR = 0x4200;

    const uint8_t STR_DB_PROM_BANK = 0;
    const uint8_t STR_DB_PROM_BLOCK = 0;
    const uint16_t STR_DB_PROM_ADDR = 0x5000;
    

    const uint16_t HEADER_PROM_ADDR = 0x4000;
    const uint16_t HEADER_PROM_BLOCK = 0;
    const uint16_t HEADER_PROM_BANK = 0;

    UsbFrmwr::INTEL_HEX_RECORD firmware[] =  {
        6,
        0xedb,
        0,
        {0x90,0x13,0xc9,0xef,0xf0,0x22},
        16,
        0xbfb,
        0,
        {0xef,0x14,0x60,0x19,0x14,0x60,0x27,0x14,0x60,0x33,0x24,0x03,0x70,0x3d,0xe4,0x90},
        16,
        0xc0b,
        0,
        {0x13,0xca,0xf0,0x90,0x13,0xc8,0xf0,0x90,0x14,0x0d,0x04,0xf0,0x22,0x90,0x13,0xca},
        16,
        0xc1b,
        0,
        {0x74,0x01,0xf0,0xe4,0x90,0x13,0xc8,0xf0,0x90,0x14,0x0d,0x04,0xf0,0x22,0xe4,0x90},
        16,
        0xc2b,
        0,
        {0x13,0xca,0xf0,0x90,0x13,0xc8,0x04,0xf0,0x90,0x14,0x0d,0xf0,0x22,0x90,0x13,0xca},
        12,
        0xc3b,
        0,
        {0x74,0x01,0xf0,0x90,0x13,0xc8,0xf0,0x90,0x14,0x0d,0xf0,0x22},
        6,
        0xa9a,
        0,
        {0x8e,0x19,0x8f,0x1a,0xa9,0x05},
        16,
        0xaa0,
        0,
        {0x75,0x1b,0x50,0x90,0x13,0xc9,0xe0,0x60,0x03,0x43,0x1b,0x04,0x90,0x13,0xc8,0xe0},
        16,
        0xab0,
        0,
        {0x60,0x03,0x43,0x1b,0x02,0x90,0x14,0x0d,0xe0,0x60,0x03,0x43,0x1b,0x01,0x53,0x19},
        16,
        0xac0,
        0,
        {0x7f,0x90,0x13,0xca,0xe0,0x60,0x03,0x43,0x19,0x80,0xe5,0x19,0x90,0x13,0xcb,0xf0},
        16,
        0xad0,
        0,
        {0xe5,0x1a,0xa3,0xf0,0xe4,0xff,0xfe,0xe9,0xfd,0xc3,0xef,0x9d,0x74,0x80,0xf8,0x6e},
        16,
        0xae0,
        0,
        {0x98,0x50,0x1e,0x8b,0x82,0x8a,0x83,0xe0,0xfd,0x74,0xcd,0x2f,0xf5,0x82,0xe4,0x34},
        16,
        0xaf0,
        0,
        {0x13,0xf5,0x83,0xed,0xf0,0x0f,0xbf,0x00,0x01,0x0e,0x0b,0xbb,0x00,0x01,0x0a,0x80},
        16,
        0xb00,
        0,
        {0xd6,0xe9,0x24,0x02,0xfd,0x7a,0x13,0x7b,0xcb,0xaf,0x1b,0x12,0x0d,0xdf,0xaf,0x1b},
        3,
        0xb10,
        0,
        {0x02,0x0c,0xfb},
        10,
        0x956,
        0,
        {0x8e,0x19,0x8f,0x1a,0x8d,0x1b,0x8a,0x1c,0x8b,0x1d},
        16,
        0x960,
        0,
        {0x75,0x1e,0x50,0x90,0x13,0xc9,0xe0,0x60,0x03,0x43,0x1e,0x04,0x90,0x13,0xc8,0xe0},
        16,
        0x970,
        0,
        {0x60,0x03,0x43,0x1e,0x02,0x90,0x14,0x0d,0xe0,0x60,0x03,0x43,0x1e,0x01,0x53,0x19},
        16,
        0x980,
        0,
        {0x7f,0x90,0x13,0xca,0xe0,0x60,0x03,0x43,0x19,0x80,0xe5,0x19,0x90,0x13,0xcb,0xf0},
        16,
        0x990,
        0,
        {0xe5,0x1a,0xa3,0xf0,0x7a,0x13,0x7b,0xcb,0x7d,0x02,0xaf,0x1e,0x12,0x0d,0xdf,0x7a},
        16,
        0x9a0,
        0,
        {0x13,0x7b,0xcb,0xad,0x1b,0xaf,0x1e,0x12,0x0d,0xc3,0xe4,0xff,0xfe,0xc3,0xef,0x95},
        16,
        0x9b0,
        0,
        {0x1b,0x74,0x80,0xf8,0x6e,0x98,0x50,0x21,0x74,0xcb,0x2f,0xf5,0x82,0xe4,0x34,0x13},
        16,
        0x9c0,
        0,
        {0xf5,0x83,0xe0,0x85,0x1d,0x82,0x85,0x1c,0x83,0xf0,0x0f,0xbf,0x00,0x01,0x0e,0x05},
        9,
        0x9d0,
        0,
        {0x1d,0xe5,0x1d,0x70,0xd8,0x05,0x1c,0x80,0xd4},
        1,
        0x9d9,
        0,
        {0x22},
        16,
        0xe2b,
        0,
        {0x90,0xe6,0x00,0x74,0x10,0xf0,0xc2,0x00,0xe4,0x90,0xe6,0x70,0xf0,0x75,0x80,0xff},
        6,
        0xe3b,
        0,
        {0x75,0xb2,0x7d,0xc2,0x86,0x22},
        10,
        0xeaf,
        0,
        {0x90,0xe6,0x7a,0xe0,0x44,0x01,0xf0,0x02,0x0e,0xd3},
        6,
        0xee1,
        0,
        {0x12,0x0e,0x2b,0x02,0x0e,0xaf},
        3,
        0xeeb,
        0,
        {0x02,0x0e,0xe1},
        3,
        0xeee,
        0,
        {0x02,0x0f,0x1f},
        2,
        0xef1,
        0,
        {0xd3,0x22},
        2,
        0xef3,
        0,
        {0xd3,0x22},
        2,
        0xef5,
        0,
        {0xd3,0x22},
        8,
        0xec3,
        0,
        {0x90,0xe6,0xba,0xe0,0xf5,0x36,0xd3,0x22},
        16,
        0xe6d,
        0,
        {0x90,0xe7,0x40,0xe5,0x36,0xf0,0xe4,0x90,0xe6,0x8a,0xf0,0x90,0xe6,0x8b,0x04,0xf0},
        2,
        0xe7d,
        0,
        {0xd3,0x22},
        8,
        0xecb,
        0,
        {0x90,0xe6,0xba,0xe0,0xf5,0x35,0xd3,0x22},
        16,
        0xe7f,
        0,
        {0x90,0xe7,0x40,0xe5,0x35,0xf0,0xe4,0x90,0xe6,0x8a,0xf0,0x90,0xe6,0x8b,0x04,0xf0},
        2,
        0xe8f,
        0,
        {0xd3,0x22},
        2,
        0xef7,
        0,
        {0xd3,0x22},
        2,
        0xef9,
        0,
        {0xd3,0x22},
        2,
        0xefb,
        0,
        {0xd3,0x22},
        16,
        0xe91,
        0,
        {0xc2,0x04,0x90,0xe6,0xb8,0xe0,0x54,0x60,0xf5,0x09,0x12,0x0b,0x13,0x92,0x04,0xa2},
        1,
        0xea1,
        0,
        {0x04},
        1,
        0xea2,
        0,
        {0x22},
        16,
        0xdfb,
        0,
        {0xc0,0xe0,0xc0,0x83,0xc0,0x82,0xd2,0x01,0x53,0x91,0xef,0x90,0xe6,0x5d,0x74,0x01},
        8,
        0xe0b,
        0,
        {0xf0,0xd0,0x82,0xd0,0x83,0xd0,0xe0,0x32},
        16,
        0xe41,
        0,
        {0xc0,0xe0,0xc0,0x83,0xc0,0x82,0x53,0x91,0xef,0x90,0xe6,0x5d,0x74,0x04,0xf0,0xd0},
        6,
        0xe51,
        0,
        {0x82,0xd0,0x83,0xd0,0xe0,0x32},
        16,
        0xe57,
        0,
        {0xc0,0xe0,0xc0,0x83,0xc0,0x82,0x53,0x91,0xef,0x90,0xe6,0x5d,0x74,0x02,0xf0,0xd0},
        6,
        0xe67,
        0,
        {0x82,0xd0,0x83,0xd0,0xe0,0x32},
        16,
        0xcc5,
        0,
        {0xc0,0xe0,0xc0,0x83,0xc0,0x82,0x85,0x29,0x25,0x85,0x2a,0x26,0x85,0x26,0x82,0x85},
        16,
        0xcd5,
        0,
        {0x25,0x83,0xa3,0x74,0x02,0xf0,0x85,0x21,0x27,0x85,0x22,0x28,0x85,0x28,0x82,0x85},
        16,
        0xce5,
        0,
        {0x27,0x83,0xa3,0x74,0x07,0xf0,0x53,0x91,0xef,0x90,0xe6,0x5d,0x74,0x10,0xf0,0xd0},
        6,
        0xcf5,
        0,
        {0x82,0xd0,0x83,0xd0,0xe0,0x32},
        16,
        0xe13,
        0,
        {0xc0,0xe0,0xc0,0x83,0xc0,0x82,0xd2,0x03,0x53,0x91,0xef,0x90,0xe6,0x5d,0x74,0x08},
        8,
        0xe23,
        0,
        {0xf0,0xd0,0x82,0xd0,0x83,0xd0,0xe0,0x32},
        16,
        0xc88,
        0,
        {0xc0,0xe0,0xc0,0x83,0xc0,0x82,0x90,0xe6,0x80,0xe0,0x30,0xe7,0x20,0x85,0x21,0x25},
        16,
        0xc98,
        0,
        {0x85,0x22,0x26,0x85,0x29,0x27,0x85,0x2a,0x28,0x85,0x26,0x82,0x85,0x25,0x83,0xa3},
        16,
        0xca8,
        0,
        {0x74,0x07,0xf0,0x85,0x28,0x82,0x85,0x27,0x83,0xa3,0x74,0x02,0xf0,0x53,0x91,0xef},
        13,
        0xcb8,
        0,
        {0x90,0xe6,0x5d,0x74,0x20,0xf0,0xd0,0x82,0xd0,0x83,0xd0,0xe0,0x32},
        1,
        0x7ff,
        0,
        {0x32},
        1,
        0x9ff,
        0,
        {0x32},
        1,
        0xefd,
        0,
        {0x32},
        1,
        0xefe,
        0,
        {0x32},
        1,
        0xeff,
        0,
        {0x32},
        1,
        0xf00,
        0,
        {0x32},
        1,
        0xf01,
        0,
        {0x32},
        1,
        0xf02,
        0,
        {0x32},
        1,
        0xf03,
        0,
        {0x32},
        1,
        0xf04,
        0,
        {0x32},
        1,
        0xf05,
        0,
        {0x32},
        1,
        0xf06,
        0,
        {0x32},
        1,
        0xf07,
        0,
        {0x32},
        1,
        0xf08,
        0,
        {0x32},
        1,
        0xf09,
        0,
        {0x32},
        1,
        0xf0a,
        0,
        {0x32},
        1,
        0xf0b,
        0,
        {0x32},
        1,
        0xf0c,
        0,
        {0x32},
        1,
        0xf0d,
        0,
        {0x32},
        1,
        0xf0e,
        0,
        {0x32},
        1,
        0xf0f,
        0,
        {0x32},
        1,
        0xf10,
        0,
        {0x32},
        1,
        0xf11,
        0,
        {0x32},
        1,
        0xf12,
        0,
        {0x32},
        1,
        0xf13,
        0,
        {0x32},
        1,
        0xf14,
        0,
        {0x32},
        1,
        0xf15,
        0,
        {0x32},
        1,
        0xf16,
        0,
        {0x32},
        1,
        0xf17,
        0,
        {0x32},
        1,
        0xf18,
        0,
        {0x32},
        1,
        0xf19,
        0,
        {0x32},
        1,
        0xf1a,
        0,
        {0x32},
        1,
        0xf1b,
        0,
        {0x32},
        1,
        0xf1c,
        0,
        {0x32},
        1,
        0xf1d,
        0,
        {0x32},
        1,
        0xf1e,
        0,
        {0x32},
        1,
        0xf1f,
        0,
        {0x22},
        16,
        0x38c,
        0,
        {0xe4,0xf5,0x18,0x90,0x13,0x6a,0xe0,0xf5,0x0c,0xa3,0xe0,0xf5,0x0d,0xaf,0x0d,0xe5},
        16,
        0x39c,
        0,
        {0x0d,0x12,0x0b,0xfb,0xe5,0x0c,0xff,0x12,0x0e,0xdb,0x90,0x13,0x0a,0xe0,0xfe,0xa3},
        16,
        0x3ac,
        0,
        {0xe0,0xff,0xe4,0x8f,0x11,0x8e,0x10,0xf5,0x0f,0xf5,0x0e,0x90,0x13,0x68,0xe0,0xf5},
        16,
        0x3bc,
        0,
        {0x12,0xa3,0xe0,0xf5,0x13,0xaf,0x11,0xae,0x10,0xad,0x0f,0xac,0x0e,0xec,0x4d,0x4e},
        16,
        0x3cc,
        0,
        {0x4f,0x70,0x03,0x02,0x04,0xbf,0xe4,0x90,0xe6,0x8a,0xf0,0x90,0xe6,0x8b,0xf0,0x00},
        16,
        0x3dc,
        0,
        {0x00,0x00,0x90,0xe6,0xa0,0xe0,0x20,0xe1,0xf9,0x90,0xe6,0x8b,0xe0,0x75,0x14,0x00},
        16,
        0x3ec,
        0,
        {0xf5,0x15,0xe4,0xf5,0x16,0xf5,0x17,0xc3,0xe5,0x17,0x95,0x15,0xe5,0x16,0x95,0x14},
        16,
        0x3fc,
        0,
        {0x50,0x24,0xaf,0x17,0x74,0x40,0x2f,0xf5,0x82,0xe4,0x34,0xe7,0xf5,0x83,0xe0,0xfe},
        16,
        0x40c,
        0,
        {0x74,0x0c,0x2f,0xf5,0x82,0xe4,0x34,0x13,0xf5,0x83,0xee,0xf0,0x05,0x17,0xe5,0x17},
        16,
        0x41c,
        0,
        {0x70,0xd5,0x05,0x16,0x80,0xd1,0xe5,0x18,0xc3,0x94,0x03,0x50,0x5b,0xad,0x15,0x7a},
        16,
        0x42c,
        0,
        {0x13,0x7b,0x0c,0xaf,0x13,0xae,0x12,0x12,0x0a,0x9a,0xad,0x15,0x7a,0x13,0x7b,0x6c},
        16,
        0x43c,
        0,
        {0xaf,0x13,0xae,0x12,0x12,0x09,0x56,0xc2,0x06,0xe4,0xf5,0x16,0xf5,0x17,0xc3,0xe5},
        16,
        0x44c,
        0,
        {0x17,0x95,0x15,0xe5,0x16,0x95,0x14,0x50,0x28,0xaf,0x17,0x74,0x6c,0x2f,0xf5,0x82},
        16,
        0x45c,
        0,
        {0xe4,0x34,0x13,0xf5,0x83,0xe0,0xfe,0x74,0x0c,0x2f,0xf5,0x82,0xe4,0x34,0x13,0xf5},
        16,
        0x46c,
        0,
        {0x83,0xe0,0x6e,0x60,0x02,0xd2,0x06,0x05,0x17,0xe5,0x17,0x70,0xd1,0x05,0x16,0x80},
        16,
        0x47c,
        0,
        {0xcd,0x30,0x06,0x04,0x05,0x18,0x80,0x9e,0xe5,0x15,0x25,0x13,0xf5,0x13,0xe5,0x14},
        16,
        0x48c,
        0,
        {0x35,0x12,0xf5,0x12,0xae,0x14,0xaf,0x15,0xe4,0xfc,0xfd,0xaa,0x06,0xab,0x07,0xc3},
        16,
        0x49c,
        0,
        {0xe5,0x11,0x9b,0xf5,0x11,0xe5,0x10,0x9a,0xf5,0x10,0xe5,0x0f,0x94,0x00,0xf5,0x0f},
        16,
        0x4ac,
        0,
        {0xe5,0x0e,0x94,0x00,0xf5,0x0e,0x20,0x06,0x03,0x02,0x03,0xc1,0x90,0xe6,0xa0,0xe0},
        3,
        0x4bc,
        0,
        {0x44,0x01,0xf0},
        1,
        0x4bf,
        0,
        {0x22},
        16,
        0x6ef,
        0,
        {0x90,0x13,0x0a,0xe0,0xfe,0xa3,0xe0,0xff,0xe4,0x8f,0x15,0x8e,0x14,0xf5,0x13,0xf5},
        16,
        0x6ff,
        0,
        {0x12,0x90,0x13,0x68,0xe0,0xf5,0x10,0xa3,0xe0,0xf5,0x11,0xa3,0xe0,0xf5,0x0c,0xa3},
        16,
        0x70f,
        0,
        {0xe0,0xf5,0x0d,0xaf,0x0d,0xe5,0x0d,0x12,0x0b,0xfb,0xe5,0x0c,0xff,0x12,0x0e,0xdb},
        16,
        0x71f,
        0,
        {0xaf,0x15,0xae,0x14,0xad,0x13,0xac,0x12,0xec,0x4d,0x4e,0x4f,0x70,0x03,0x02,0x07},
        16,
        0x72f,
        0,
        {0xd2,0xe4,0x7f,0x40,0xfe,0xfd,0xfc,0xab,0x15,0xaa,0x14,0xa9,0x13,0xa8,0x12,0xc3},
        16,
        0x73f,
        0,
        {0x12,0x09,0x07,0x50,0x06,0xae,0x14,0xaf,0x15,0x80,0x04,0x7e,0x00,0x7f,0x40,0x8e},
        16,
        0x74f,
        0,
        {0x0e,0x8f,0x0f,0xad,0x0f,0x7a,0x13,0x7b,0x0c,0xaf,0x11,0xae,0x10,0x12,0x09,0x56},
        16,
        0x75f,
        0,
        {0xe4,0xff,0xfe,0xc3,0xef,0x95,0x0f,0xee,0x95,0x0e,0x50,0x1f,0x74,0x0c,0x2f,0xf5},
        16,
        0x76f,
        0,
        {0x82,0xe4,0x34,0x13,0xf5,0x83,0xe0,0xfd,0x74,0x40,0x2f,0xf5,0x82,0xe4,0x34,0xe7},
        16,
        0x77f,
        0,
        {0xf5,0x83,0xed,0xf0,0x0f,0xbf,0x00,0x01,0x0e,0x80,0xd8,0xe5,0x0f,0x25,0x11,0xf5},
        16,
        0x78f,
        0,
        {0x11,0xe5,0x0e,0x35,0x10,0xf5,0x10,0xae,0x0e,0xaf,0x0f,0xe4,0xfc,0xfd,0xaa,0x06},
        16,
        0x79f,
        0,
        {0xab,0x07,0xc3,0xe5,0x15,0x9b,0xf5,0x15,0xe5,0x14,0x9a,0xf5,0x14,0xe5,0x13,0x94},
        16,
        0x7af,
        0,
        {0x00,0xf5,0x13,0xe5,0x12,0x94,0x00,0xf5,0x12,0xe5,0x0e,0x90,0xe6,0x8a,0xf0,0xef},
        16,
        0x7bf,
        0,
        {0x90,0xe6,0x8b,0xf0,0x00,0x00,0x00,0x90,0xe6,0xa0,0xe0,0x20,0xe1,0x03,0x02,0x07},
        3,
        0x7cf,
        0,
        {0x1f,0x80,0xf4},
        1,
        0x7d2,
        0,
        {0x22},
        16,
        0xb13,
        0,
        {0xe4,0xf5,0x0a,0xf5,0x0b,0x90,0xe6,0xbf,0xe0,0xfe,0x90,0xe6,0xbe,0xe0,0x7c,0x00},
        16,
        0xb23,
        0,
        {0x24,0x00,0x90,0x13,0x0b,0xf0,0xec,0x3e,0x90,0x13,0x0a,0xf0,0x90,0xe6,0xbd,0xe0},
        16,
        0xb33,
        0,
        {0xfe,0x90,0xe6,0xbc,0xe0,0x24,0x00,0x90,0x13,0x69,0xf0,0xec,0x3e,0x90,0x13,0x68},
        16,
        0xb43,
        0,
        {0xf0,0x90,0xe6,0xbb,0xe0,0xfe,0x90,0xe6,0xba,0xe0,0x24,0x00,0x90,0x13,0x6b,0xf0},
        16,
        0xb53,
        0,
        {0xec,0x3e,0x90,0x13,0x6a,0xf0,0x90,0xe6,0xb8,0xe0,0x33,0x92,0x05,0xa3,0xe0,0xff},
        16,
        0xb63,
        0,
        {0x24,0x3a,0x70,0x0d,0x30,0x05,0x05,0x12,0x06,0xef,0x80,0x0b,0x12,0x03,0x8c,0x80},
        16,
        0xb73,
        0,
        {0x06,0x74,0xff,0xf5,0x0a,0xf5,0x0b,0xae,0x0a,0xaf,0x0b,0xbe,0xff,0x06,0xbf,0xff},
        5,
        0xb83,
        0,
        {0x03,0xd3,0x80,0x01,0xc3},
        1,
        0xb88,
        0,
        {0x22},
        10,
        0xeb9,
        0,
        {0x00,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05},
        16,
        0x4c0,
        0,
        {0x7e,0x0a,0x7f,0x00,0x8e,0x23,0x8f,0x24,0x75,0x2b,0x0a,0x75,0x2c,0x12,0x75,0x21},
        16,
        0x4d0,
        0,
        {0x0a,0x75,0x22,0x1c,0x75,0x29,0x0a,0x75,0x2a,0x2e,0x75,0x2d,0x0a,0x75,0x2e,0x40},
        16,
        0x4e0,
        0,
        {0xee,0x54,0xe0,0x70,0x03,0x02,0x05,0xb3,0x90,0x13,0x02,0x74,0x12,0xf0,0xa3,0x74},
        16,
        0x4f0,
        0,
        {0x02,0xf0,0xa3,0xee,0xf0,0xa3,0xef,0xf0,0xc3,0x74,0x98,0x9f,0xff,0x74,0x0a,0x9e},
        16,
        0x500,
        0,
        {0xcf,0x24,0x02,0xcf,0x34,0x00,0xfe,0xe4,0xfc,0xfd,0xa3,0x12,0x09,0x24,0xe4,0xf5},
        16,
        0x510,
        0,
        {0x0b,0xf5,0x0a,0xf5,0x09,0xf5,0x08,0x90,0x13,0x06,0x12,0x09,0x18,0xab,0x0b,0xaa},
        16,
        0x520,
        0,
        {0x0a,0xa9,0x09,0xa8,0x08,0xc3,0x12,0x09,0x07,0x50,0x3a,0xae,0x0a,0xaf,0x0b,0x90},
        16,
        0x530,
        0,
        {0x13,0x04,0xe0,0xfc,0xa3,0xe0,0x2f,0xf5,0x82,0xec,0x3e,0xf5,0x83,0xe4,0x93,0xfd},
        16,
        0x540,
        0,
        {0x90,0x13,0x02,0xe0,0xfa,0xa3,0xe0,0x2f,0xf5,0x82,0xea,0x3e,0xf5,0x83,0xed,0xf0},
        16,
        0x550,
        0,
        {0xef,0x24,0x01,0xf5,0x0b,0xe4,0x3e,0xf5,0x0a,0xe4,0x35,0x09,0xf5,0x09,0xe4,0x35},
        16,
        0x560,
        0,
        {0x08,0xf5,0x08,0x80,0xb2,0x90,0x13,0x02,0xe0,0xfe,0xa3,0xe0,0xff,0xc3,0x74,0x00},
        16,
        0x570,
        0,
        {0x9f,0xff,0x74,0x0a,0x9e,0xfe,0x90,0x12,0x00,0xf0,0xa3,0xef,0xf0,0x90,0x13,0x02},
        16,
        0x580,
        0,
        {0xe0,0xf5,0x23,0xa3,0xe0,0xf5,0x24,0xc3,0xe5,0x2c,0x9f,0xf5,0x2c,0xe5,0x2b,0x9e},
        16,
        0x590,
        0,
        {0xf5,0x2b,0xc3,0xe5,0x22,0x9f,0xf5,0x22,0xe5,0x21,0x9e,0xf5,0x21,0xc3,0xe5,0x2a},
        16,
        0x5a0,
        0,
        {0x9f,0xf5,0x2a,0xe5,0x29,0x9e,0xf5,0x29,0xc3,0xe5,0x2e,0x9f,0xf5,0x2e,0xe5,0x2d},
        16,
        0x5b0,
        0,
        {0x9e,0xf5,0x2d,0x85,0x29,0x25,0x85,0x2a,0x26,0x85,0x21,0x27,0x85,0x22,0x28,0x90},
        16,
        0x5c0,
        0,
        {0xe6,0x80,0xe0,0x30,0xe7,0x0c,0x85,0x21,0x25,0x85,0x22,0x26,0x85,0x29,0x27,0x85},
        16,
        0x5d0,
        0,
        {0x2a,0x28,0x85,0x26,0x82,0x85,0x25,0x83,0xa3,0x74,0x02,0xf0,0x85,0x28,0x82,0x85},
        6,
        0x5e0,
        0,
        {0x27,0x83,0xa3,0x74,0x07,0xf0},
        1,
        0x5e6,
        0,
        {0x22},
        16,
        0x100,
        0,
        {0x90,0xe6,0xb8,0xe0,0x54,0x60,0xf5,0x08,0xa3,0xe0,0x12,0x09,0x30,0x01,0xce,0x00},
        16,
        0x110,
        0,
        {0x02,0x77,0x01,0x03,0x0c,0x03,0x01,0x29,0x06,0x01,0xc8,0x08,0x01,0xc2,0x09,0x01},
        16,
        0x120,
        0,
        {0xb6,0x0a,0x01,0xbc,0x0b,0x00,0x00,0x03,0x78,0x12,0x0e,0xf5,0x40,0x03,0x02,0x03},
        16,
        0x130,
        0,
        {0x84,0x90,0xe6,0xbb,0xe0,0x24,0xfe,0x60,0x2c,0x14,0x60,0x47,0x24,0xfd,0x60,0x16},
        16,
        0x140,
        0,
        {0x14,0x60,0x31,0x24,0x06,0x70,0x65,0xe5,0x23,0x90,0xe6,0xb3,0xf0,0xe5,0x24,0x90},
        16,
        0x150,
        0,
        {0xe6,0xb4,0xf0,0x02,0x03,0x84,0xe5,0x2b,0x90,0xe6,0xb3,0xf0,0xe5,0x2c,0x90,0xe6},
        16,
        0x160,
        0,
        {0xb4,0xf0,0x02,0x03,0x84,0xe5,0x25,0x90,0xe6,0xb3,0xf0,0xe5,0x26,0x90,0xe6,0xb4},
        16,
        0x170,
        0,
        {0xf0,0x02,0x03,0x84,0xe5,0x27,0x90,0xe6,0xb3,0xf0,0xe5,0x28,0x90,0xe6,0xb4,0xf0},
        16,
        0x180,
        0,
        {0x02,0x03,0x84,0x90,0xe6,0xba,0xe0,0xff,0x12,0x0d,0x97,0xaa,0x06,0xa9,0x07,0x7b},
        16,
        0x190,
        0,
        {0x01,0xea,0x49,0x60,0x0d,0xee,0x90,0xe6,0xb3,0xf0,0xef,0x90,0xe6,0xb4,0xf0,0x02},
        16,
        0x1a0,
        0,
        {0x03,0x84,0x90,0xe6,0xa0,0xe0,0x44,0x01,0xf0,0x02,0x03,0x84,0x90,0xe6,0xa0,0xe0},
        16,
        0x1b0,
        0,
        {0x44,0x01,0xf0,0x02,0x03,0x84,0x12,0x0e,0x7f,0x02,0x03,0x84,0x12,0x0e,0xcb,0x02},
        16,
        0x1c0,
        0,
        {0x03,0x84,0x12,0x0e,0xc3,0x02,0x03,0x84,0x12,0x0e,0x6d,0x02,0x03,0x84,0x12,0x0e},
        16,
        0x1d0,
        0,
        {0xf7,0x40,0x03,0x02,0x03,0x84,0x90,0xe6,0xb8,0xe0,0x24,0x7f,0x60,0x2b,0x14,0x60},
        16,
        0x1e0,
        0,
        {0x3c,0x24,0x02,0x60,0x03,0x02,0x02,0x6d,0xa2,0x00,0xe4,0x33,0xff,0x25,0xe0,0xff},
        16,
        0x1f0,
        0,
        {0xa2,0x02,0xe4,0x33,0x4f,0x90,0xe7,0x40,0xf0,0xe4,0xa3,0xf0,0x90,0xe6,0x8a,0xf0},
        16,
        0x200,
        0,
        {0x90,0xe6,0x8b,0x74,0x02,0xf0,0x02,0x03,0x84,0xe4,0x90,0xe7,0x40,0xf0,0xa3,0xf0},
        16,
        0x210,
        0,
        {0x90,0xe6,0x8a,0xf0,0x90,0xe6,0x8b,0x74,0x02,0xf0,0x02,0x03,0x84,0x90,0xe6,0xbc},
        16,
        0x220,
        0,
        {0xe0,0x54,0x7e,0xff,0x7e,0x00,0xe0,0xd3,0x94,0x80,0x40,0x06,0x7c,0x00,0x7d,0x01},
        16,
        0x230,
        0,
        {0x80,0x04,0x7c,0x00,0x7d,0x00,0xec,0x4e,0xfe,0xed,0x4f,0x24,0xb9,0xf5,0x82,0x74},
        16,
        0x240,
        0,
        {0x0e,0x3e,0xf5,0x83,0xe4,0x93,0xff,0x33,0x95,0xe0,0xfe,0xef,0x24,0xa1,0xff,0xee},
        16,
        0x250,
        0,
        {0x34,0xe6,0x8f,0x82,0xf5,0x83,0xe0,0x54,0x01,0x90,0xe7,0x40,0xf0,0xe4,0xa3,0xf0},
        16,
        0x260,
        0,
        {0x90,0xe6,0x8a,0xf0,0x90,0xe6,0x8b,0x74,0x02,0xf0,0x02,0x03,0x84,0x90,0xe6,0xa0},
        16,
        0x270,
        0,
        {0xe0,0x44,0x01,0xf0,0x02,0x03,0x84,0x12,0x0e,0xf9,0x40,0x03,0x02,0x03,0x84,0x90},
        16,
        0x280,
        0,
        {0xe6,0xb8,0xe0,0x24,0xfe,0x60,0x1d,0x24,0x02,0x60,0x03,0x02,0x03,0x84,0x90,0xe6},
        16,
        0x290,
        0,
        {0xba,0xe0,0xb4,0x01,0x05,0xc2,0x00,0x02,0x03,0x84,0x90,0xe6,0xa0,0xe0,0x44,0x01},
        16,
        0x2a0,
        0,
        {0xf0,0x02,0x03,0x84,0x90,0xe6,0xba,0xe0,0x70,0x59,0x90,0xe6,0xbc,0xe0,0x54,0x7e},
        16,
        0x2b0,
        0,
        {0xff,0x7e,0x00,0xe0,0xd3,0x94,0x80,0x40,0x06,0x7c,0x00,0x7d,0x01,0x80,0x04,0x7c},
        16,
        0x2c0,
        0,
        {0x00,0x7d,0x00,0xec,0x4e,0xfe,0xed,0x4f,0x24,0xb9,0xf5,0x82,0x74,0x0e,0x3e,0xf5},
        16,
        0x2d0,
        0,
        {0x83,0xe4,0x93,0xff,0x33,0x95,0xe0,0xfe,0xef,0x24,0xa1,0xff,0xee,0x34,0xe6,0x8f},
        16,
        0x2e0,
        0,
        {0x82,0xf5,0x83,0xe0,0x54,0xfe,0xf0,0x90,0xe6,0xbc,0xe0,0x54,0x80,0xff,0x13,0x13},
        16,
        0x2f0,
        0,
        {0x13,0x54,0x1f,0xff,0xe0,0x54,0x0f,0x2f,0x90,0xe6,0x83,0xf0,0xe0,0x44,0x20,0xf0},
        16,
        0x300,
        0,
        {0x02,0x03,0x84,0x90,0xe6,0xa0,0xe0,0x44,0x01,0xf0,0x80,0x78,0x12,0x0e,0xfb,0x50},
        16,
        0x310,
        0,
        {0x73,0x90,0xe6,0xb8,0xe0,0x24,0xfe,0x60,0x20,0x24,0x02,0x70,0x67,0x90,0xe6,0xba},
        16,
        0x320,
        0,
        {0xe0,0xb4,0x01,0x04,0xd2,0x00,0x80,0x5c,0x90,0xe6,0xba,0xe0,0x64,0x02,0x60,0x54},
        16,
        0x330,
        0,
        {0x90,0xe6,0xa0,0xe0,0x44,0x01,0xf0,0x80,0x4b,0x90,0xe6,0xbc,0xe0,0x54,0x7e,0xff},
        16,
        0x340,
        0,
        {0x7e,0x00,0xe0,0xd3,0x94,0x80,0x40,0x06,0x7c,0x00,0x7d,0x01,0x80,0x04,0x7c,0x00},
        16,
        0x350,
        0,
        {0x7d,0x00,0xec,0x4e,0xfe,0xed,0x4f,0x24,0xb9,0xf5,0x82,0x74,0x0e,0x3e,0xf5,0x83},
        16,
        0x360,
        0,
        {0xe4,0x93,0xff,0x33,0x95,0xe0,0xfe,0xef,0x24,0xa1,0xff,0xee,0x34,0xe6,0x8f,0x82},
        16,
        0x370,
        0,
        {0xf5,0x83,0xe0,0x44,0x01,0xf0,0x80,0x0c,0x12,0x0e,0x91,0x50,0x07,0x90,0xe6,0xa0},
        11,
        0x380,
        0,
        {0xe0,0x44,0x01,0xf0,0x90,0xe6,0xa0,0xe0,0x44,0x80,0xf0},
        1,
        0x38b,
        0,
        {0x22},
        16,
        0xb89,
        0,
        {0xc2,0x03,0xc2,0x00,0xc2,0x02,0xc2,0x01,0x12,0x0e,0xeb,0x12,0x04,0xc0,0xd2,0xe8},
        16,
        0xb99,
        0,
        {0x43,0xd8,0x20,0x90,0xe6,0x68,0xe0,0x44,0x0b,0xf0,0x75,0xa2,0xff,0x90,0xe6,0x5c},
        16,
        0xba9,
        0,
        {0xe0,0x44,0x3d,0xf0,0xd2,0xaf,0x90,0xe6,0x80,0xe0,0x44,0x02,0xf0,0x00,0x00,0x00},
        16,
        0xbb9,
        0,
        {0xe0,0x54,0xf7,0xf0,0x53,0x8e,0xf8,0xc2,0x03,0x30,0x01,0x05,0x12,0x01,0x00,0xc2},
        16,
        0xbc9,
        0,
        {0x01,0x30,0x03,0x29,0x12,0x0e,0xf1,0x50,0x24,0xc2,0x03,0x12,0x09,0xda,0x20,0x00},
        16,
        0xbd9,
        0,
        {0x16,0x90,0xe6,0x82,0xe0,0x30,0xe7,0x04,0xe0,0x20,0xe1,0xef,0x90,0xe6,0x82,0xe0},
        16,
        0xbe9,
        0,
        {0x30,0xe6,0x04,0xe0,0x20,0xe0,0xe4,0x12,0x07,0xd3,0x12,0x0e,0xf3,0x12,0x0e,0xee},
        2,
        0xbf9,
        0,
        {0x80,0xc7},
        3,
        0x33,
        0,
        {0x02,0x0e,0xe7},
        4,
        0xee7,
        0,
        {0x53,0xd8,0xef,0x32},
        3,
        0x43,
        0,
        {0x02,0x08,0x00},
        3,
        0x53,
        0,
        {0x02,0x08,0x00},
        16,
        0x800,
        0,
        {0x02,0x0d,0xfb,0x00,0x02,0x0e,0x57,0x00,0x02,0x0e,0x41,0x00,0x02,0x0e,0x13,0x00},
        16,
        0x810,
        0,
        {0x02,0x0c,0xc5,0x00,0x02,0x0c,0x88,0x00,0x02,0x07,0xff,0x00,0x02,0x09,0xff,0x00},
        16,
        0x820,
        0,
        {0x02,0x0e,0xfd,0x00,0x02,0x0e,0xfe,0x00,0x02,0x0e,0xff,0x00,0x02,0x0f,0x00,0x00},
        16,
        0x830,
        0,
        {0x02,0x0f,0x01,0x00,0x02,0x0f,0x02,0x00,0x02,0x0f,0x03,0x00,0x02,0x0f,0x04,0x00},
        16,
        0x840,
        0,
        {0x02,0x0f,0x05,0x00,0x02,0x09,0xff,0x00,0x02,0x0f,0x06,0x00,0x02,0x0f,0x07,0x00},
        16,
        0x850,
        0,
        {0x02,0x0f,0x08,0x00,0x02,0x0f,0x09,0x00,0x02,0x0f,0x0a,0x00,0x02,0x0f,0x0b,0x00},
        16,
        0x860,
        0,
        {0x02,0x0f,0x0c,0x00,0x02,0x09,0xff,0x00,0x02,0x09,0xff,0x00,0x02,0x09,0xff,0x00},
        16,
        0x870,
        0,
        {0x02,0x0f,0x0d,0x00,0x02,0x0f,0x0e,0x00,0x02,0x0f,0x0f,0x00,0x02,0x0f,0x10,0x00},
        16,
        0x880,
        0,
        {0x02,0x0f,0x11,0x00,0x02,0x0f,0x12,0x00,0x02,0x0f,0x13,0x00,0x02,0x0f,0x14,0x00},
        16,
        0x890,
        0,
        {0x02,0x0f,0x15,0x00,0x02,0x0f,0x16,0x00,0x02,0x0f,0x17,0x00,0x02,0x0f,0x18,0x00},
        16,
        0x8a0,
        0,
        {0x02,0x0f,0x19,0x00,0x02,0x0f,0x1a,0x00,0x02,0x0f,0x1b,0x00,0x02,0x0f,0x1c,0x00},
        8,
        0x8b0,
        0,
        {0x02,0x0f,0x1d,0x00,0x02,0x0f,0x1e,0x00},
        16,
        0xa00,
        0,
        {0x12,0x01,0x00,0x02,0x00,0x00,0x00,0x40,0xb4,0x04,0x13,0x86,0x00,0x00,0x01,0x02},
        16,
        0xa10,
        0,
        {0x00,0x01,0x0a,0x06,0x00,0x02,0x00,0x00,0x00,0x40,0x01,0x00,0x09,0x02,0x12,0x00},
        16,
        0xa20,
        0,
        {0x01,0x01,0x00,0x60,0x32,0x09,0x04,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x09,0x02},
        16,
        0xa30,
        0,
        {0x12,0x00,0x01,0x01,0x00,0x60,0x32,0x09,0x04,0x00,0x00,0x00,0xff,0x00,0x00,0x00},
        16,
        0xa40,
        0,
        {0x04,0x03,0x09,0x04,0x30,0x03,0x41,0x00,0x70,0x00,0x6f,0x00,0x67,0x00,0x65,0x00},
        16,
        0xa50,
        0,
        {0x65,0x00,0x2d,0x00,0x49,0x00,0x6e,0x00,0x73,0x00,0x74,0x00,0x72,0x00,0x75,0x00},
        16,
        0xa60,
        0,
        {0x6d,0x00,0x65,0x00,0x6e,0x00,0x74,0x00,0x73,0x00,0x20,0x00,0x49,0x00,0x6e,0x00},
        16,
        0xa70,
        0,
        {0x63,0x00,0x2e,0x00,0x24,0x03,0x41,0x00,0x73,0x00,0x63,0x00,0x65,0x00,0x6e,0x00},
        16,
        0xa80,
        0,
        {0x74,0x00,0x20,0x00,0x52,0x00,0x4f,0x00,0x4d,0x00,0x20,0x00,0x4c,0x00,0x4f,0x00},
        10,
        0xa90,
        0,
        {0x41,0x00,0x44,0x00,0x45,0x00,0x52,0x00,0x00,0x00},
        16,
        0x7d3,
        0,
        {0x90,0xe6,0x82,0xe0,0x30,0xe0,0x04,0xe0,0x20,0xe6,0x0b,0x90,0xe6,0x82,0xe0,0x30},
        16,
        0x7e3,
        0,
        {0xe1,0x19,0xe0,0x30,0xe7,0x15,0x90,0xe6,0x80,0xe0,0x44,0x01,0xf0,0x7f,0x14,0x7e},
        12,
        0x7f3,
        0,
        {0x00,0x12,0x0c,0x47,0x90,0xe6,0x80,0xe0,0x54,0xfe,0xf0,0x22},
        16,
        0x9da,
        0,
        {0x90,0xe6,0x82,0xe0,0x44,0xc0,0xf0,0x90,0xe6,0x81,0xf0,0x43,0x87,0x01,0x00,0x00},
        4,
        0x9ea,
        0,
        {0x00,0x00,0x00,0x22},
        8,
        0xed3,
        0,
        {0xe4,0xf5,0x34,0xd2,0xe9,0xd2,0xaf,0x22},
        16,
        0xcfb,
        0,
        {0x90,0xe6,0x78,0xe0,0x20,0xe6,0xf9,0xc2,0xe9,0x90,0xe6,0x78,0xe0,0x44,0x80,0xf0},
        16,
        0xd0b,
        0,
        {0xef,0x25,0xe0,0x90,0xe6,0x79,0xf0,0x90,0xe6,0x78,0xe0,0x30,0xe0,0xf9,0x90,0xe6},
        16,
        0xd1b,
        0,
        {0x78,0xe0,0x44,0x40,0xf0,0x90,0xe6,0x78,0xe0,0x20,0xe6,0xf9,0x90,0xe6,0x78,0xe0},
        6,
        0xd2b,
        0,
        {0x30,0xe1,0xd6,0xd2,0xe9,0x22},
        16,
        0xd65,
        0,
        {0xa9,0x07,0x90,0xe6,0x78,0xe0,0x20,0xe6,0xf9,0xe5,0x34,0x70,0x23,0x90,0xe6,0x78},
        16,
        0xd75,
        0,
        {0xe0,0x44,0x80,0xf0,0xe9,0x25,0xe0,0x90,0xe6,0x79,0xf0,0x8d,0x2f,0xaf,0x03,0xa9},
        16,
        0xd85,
        0,
        {0x07,0x75,0x30,0x01,0x8a,0x31,0x89,0x32,0xe4,0xf5,0x33,0x75,0x34,0x01,0xd3,0x22},
        2,
        0xd95,
        0,
        {0xc3,0x22},
        16,
        0xd31,
        0,
        {0xa9,0x07,0x90,0xe6,0x78,0xe0,0x20,0xe6,0xf9,0xe5,0x34,0x70,0x25,0x90,0xe6,0x78},
        16,
        0xd41,
        0,
        {0xe0,0x44,0x80,0xf0,0xe9,0x25,0xe0,0x44,0x01,0x90,0xe6,0x79,0xf0,0x8d,0x2f,0xaf},
        16,
        0xd51,
        0,
        {0x03,0xa9,0x07,0x75,0x30,0x01,0x8a,0x31,0x89,0x32,0xe4,0xf5,0x33,0x75,0x34,0x03},
        4,
        0xd61,
        0,
        {0xd3,0x22,0xc3,0x22},
        3,
        0x4b,
        0,
        {0x02,0x05,0xe7},
        16,
        0x5e7,
        0,
        {0xc0,0xe0,0xc0,0x83,0xc0,0x82,0xc0,0x85,0xc0,0x84,0xc0,0x86,0x75,0x86,0x00,0xc0},
        16,
        0x5f7,
        0,
        {0xd0,0x75,0xd0,0x00,0xc0,0x00,0xc0,0x01,0xc0,0x02,0xc0,0x03,0xc0,0x06,0xc0,0x07},
        16,
        0x607,
        0,
        {0x90,0xe6,0x78,0xe0,0x30,0xe2,0x06,0x75,0x34,0x06,0x02,0x06,0xd1,0x90,0xe6,0x78},
        16,
        0x617,
        0,
        {0xe0,0x20,0xe1,0x0c,0xe5,0x34,0x64,0x02,0x60,0x06,0x75,0x34,0x07,0x02,0x06,0xd1},
        16,
        0x627,
        0,
        {0xe5,0x34,0x24,0xfe,0x60,0x5f,0x14,0x60,0x36,0x24,0xfe,0x70,0x03,0x02,0x06,0xc2},
        16,
        0x637,
        0,
        {0x24,0xfc,0x70,0x03,0x02,0x06,0xce,0x24,0x08,0x60,0x03,0x02,0x06,0xd1,0xab,0x30},
        16,
        0x647,
        0,
        {0xaa,0x31,0xa9,0x32,0xaf,0x33,0x05,0x33,0x8f,0x82,0x75,0x83,0x00,0x12,0x08,0xb8},
        16,
        0x657,
        0,
        {0x90,0xe6,0x79,0xf0,0xe5,0x33,0x65,0x2f,0x70,0x70,0x75,0x34,0x05,0x80,0x6b,0x90},
        16,
        0x667,
        0,
        {0xe6,0x79,0xe0,0xab,0x30,0xaa,0x31,0xa9,0x32,0xae,0x33,0x8e,0x82,0x75,0x83,0x00},
        16,
        0x677,
        0,
        {0x12,0x08,0xe5,0x75,0x34,0x02,0xe5,0x2f,0x64,0x01,0x70,0x4e,0x90,0xe6,0x78,0xe0},
        16,
        0x687,
        0,
        {0x44,0x20,0xf0,0x80,0x45,0xe5,0x2f,0x24,0xfe,0xb5,0x33,0x07,0x90,0xe6,0x78,0xe0},
        16,
        0x697,
        0,
        {0x44,0x20,0xf0,0xe5,0x2f,0x14,0xb5,0x33,0x0a,0x90,0xe6,0x78,0xe0,0x44,0x40,0xf0},
        16,
        0x6a7,
        0,
        {0x75,0x34,0x00,0x90,0xe6,0x79,0xe0,0xab,0x30,0xaa,0x31,0xa9,0x32,0xae,0x33,0x8e},
        16,
        0x6b7,
        0,
        {0x82,0x75,0x83,0x00,0x12,0x08,0xe5,0x05,0x33,0x80,0x0f,0x90,0xe6,0x78,0xe0,0x44},
        16,
        0x6c7,
        0,
        {0x40,0xf0,0x75,0x34,0x00,0x80,0x03,0x75,0x34,0x00,0x53,0x91,0xdf,0xd0,0x07,0xd0},
        16,
        0x6d7,
        0,
        {0x06,0xd0,0x03,0xd0,0x02,0xd0,0x01,0xd0,0x00,0xd0,0xd0,0xd0,0x86,0xd0,0x84,0xd0},
        8,
        0x6e7,
        0,
        {0x85,0xd0,0x82,0xd0,0x83,0xd0,0xe0,0x32},
        2,
        0xd97,
        0,
        {0xa9,0x07},
        16,
        0xd99,
        0,
        {0xae,0x2d,0xaf,0x2e,0x8f,0x82,0x8e,0x83,0xa3,0xe0,0x64,0x03,0x70,0x17,0xad,0x01},
        16,
        0xda9,
        0,
        {0x19,0xed,0x70,0x01,0x22,0x8f,0x82,0x8e,0x83,0xe0,0x7c,0x00,0x2f,0xfd,0xec,0x3e},
        9,
        0xdb9,
        0,
        {0xfe,0xaf,0x05,0x80,0xdf,0x7e,0x00,0x7f,0x00},
        1,
        0xdc2,
        0,
        {0x22},
        16,
        0xdc3,
        0,
        {0x12,0x0d,0x31,0xe5,0x34,0x24,0xfa,0x60,0x0e,0x14,0x60,0x06,0x24,0x07,0x70,0xf3},
        12,
        0xdd3,
        0,
        {0xd3,0x22,0xe4,0xf5,0x34,0xd3,0x22,0xe4,0xf5,0x34,0xd3,0x22},
        16,
        0xddf,
        0,
        {0x12,0x0d,0x65,0xe5,0x34,0x24,0xfa,0x60,0x0e,0x14,0x60,0x06,0x24,0x07,0x70,0xf3},
        12,
        0xdef,
        0,
        {0xd3,0x22,0xe4,0xf5,0x34,0xd3,0x22,0xe4,0xf5,0x34,0xd3,0x22},
        16,
        0xc47,
        0,
        {0x8e,0x08,0x8f,0x09,0x90,0xe6,0x00,0xe0,0x54,0x18,0x70,0x0d,0xe5,0x08,0xc3,0x13},
        16,
        0xc57,
        0,
        {0xf5,0x08,0xe5,0x09,0x13,0xf5,0x09,0x80,0x15,0x90,0xe6,0x00,0xe0,0x54,0x18,0xff},
        16,
        0xc67,
        0,
        {0xbf,0x10,0x0b,0xe5,0x09,0x25,0xe0,0xf5,0x09,0xe5,0x08,0x33,0xf5,0x08,0xe5,0x09},
        16,
        0xc77,
        0,
        {0x15,0x09,0xae,0x08,0x70,0x02,0x15,0x08,0x4e,0x60,0x05,0x12,0x09,0xee,0x80,0xee},
        1,
        0xc87,
        0,
        {0x22},
        16,
        0x9ee,
        0,
        {0x74,0x00,0xf5,0x86,0x90,0xfd,0xa5,0x7c,0x05,0xa3,0xe5,0x82,0x45,0x83,0x70,0xf9},
        1,
        0x9fe,
        0,
        {0x22},
        3,
        0x0,
        0,
        {0x02,0x0e,0xa3},
        12,
        0xea3,
        0,
        {0x78,0x7f,0xe4,0xf6,0xd8,0xfd,0x75,0x81,0x36,0x02,0x0b,0x89},
        16,
        0x8b8,
        0,
        {0xbb,0x01,0x0c,0xe5,0x82,0x29,0xf5,0x82,0xe5,0x83,0x3a,0xf5,0x83,0xe0,0x22,0x50},
        16,
        0x8c8,
        0,
        {0x06,0xe9,0x25,0x82,0xf8,0xe6,0x22,0xbb,0xfe,0x06,0xe9,0x25,0x82,0xf8,0xe2,0x22},
        13,
        0x8d8,
        0,
        {0xe5,0x82,0x29,0xf5,0x82,0xe5,0x83,0x3a,0xf5,0x83,0xe4,0x93,0x22},
        16,
        0x8e5,
        0,
        {0xf8,0xbb,0x01,0x0d,0xe5,0x82,0x29,0xf5,0x82,0xe5,0x83,0x3a,0xf5,0x83,0xe8,0xf0},
        16,
        0x8f5,
        0,
        {0x22,0x50,0x06,0xe9,0x25,0x82,0xc8,0xf6,0x22,0xbb,0xfe,0x05,0xe9,0x25,0x82,0xc8},
        2,
        0x905,
        0,
        {0xf2,0x22},
        16,
        0x907,
        0,
        {0xeb,0x9f,0xf5,0xf0,0xea,0x9e,0x42,0xf0,0xe9,0x9d,0x42,0xf0,0xe8,0x9c,0x45,0xf0},
        1,
        0x917,
        0,
        {0x22},
        12,
        0x918,
        0,
        {0xe0,0xfc,0xa3,0xe0,0xfd,0xa3,0xe0,0xfe,0xa3,0xe0,0xff,0x22},
        12,
        0x924,
        0,
        {0xec,0xf0,0xa3,0xed,0xf0,0xa3,0xee,0xf0,0xa3,0xef,0xf0,0x22},
        16,
        0x930,
        0,
        {0xd0,0x83,0xd0,0x82,0xf8,0xe4,0x93,0x70,0x12,0x74,0x01,0x93,0x70,0x0d,0xa3,0xa3},
        16,
        0x940,
        0,
        {0x93,0xf8,0x74,0x01,0x93,0xf5,0x82,0x88,0x83,0xe4,0x73,0x74,0x02,0x93,0x68,0x60},
        6,
        0x950,
        0,
        {0xef,0xa3,0xa3,0xa3,0x80,0xdf},
        0,
        0x0,
        1,
        {0}
    };
}

//////////////////////////// 
// CTOR 
AscentBasedUsbIo::AscentBasedUsbIo( const std::string & DeviceEnum ) :
                                        CamUsbIo( DeviceEnum, MAX_USB_BUFFER_SIZE, false ),
                                        m_fileName( __FILE__)
{ 

} 

//////////////////////////// 
// DTOR 
AscentBasedUsbIo::~AscentBasedUsbIo() 
{ 

} 

//////////////////////////// 
//      SET     SERIAL       NUMBER
void AscentBasedUsbIo::SetSerialNumber(const std::string & num)
{
    std::vector< std::string > result = ReadStrDatabase();
    CamInfo::StrDb db =  CamInfo::MkStrDbFromStrVect( result );
    db.CustomerSn = num;
    WriteStrDatabase(  CamInfo::MkStrVectFromStrDb( db ) );
}

//////////////////////////// 
//  GET     SERIAL       NUMBER
std::string AscentBasedUsbIo::GetSerialNumber()
{
    std::vector< std::string > result = ReadStrDatabase();

    CamInfo::StrDb infoStruct =  CamInfo::MkStrDbFromStrVect( result );

    return infoStruct.CustomerSn;
}


//////////////////////////// 
//      WRITE        STR     DATABASE
void AscentBasedUsbIo::WriteStrDatabase( const std::vector<std::string> & info )
{
 
  std::vector<uint8_t> buffer =  CamStrDb::PackStrings( info );
  
  PromFx2Io pf( m_Usb,
        ASCENT_EEPROM_MAX_BLOCKS,
        ASCENT_EEPROM_MAX_BANKS );

  pf.BufferWriteEeprom( STR_DB_PROM_BANK, STR_DB_PROM_BLOCK,
    STR_DB_PROM_ADDR, buffer );
 
}

//////////////////////////// 
//      READ       STR     DATABASE
std::vector<std::string> AscentBasedUsbIo::ReadStrDatabase()
{
   std::vector<uint8_t> buffer( CamStrDb::MAX_STR_DB_BYTES );

   PromFx2Io pf( m_Usb,
        ASCENT_EEPROM_MAX_BLOCKS,
        ASCENT_EEPROM_MAX_BANKS );

   pf.BufferReadEeprom( STR_DB_PROM_BANK, STR_DB_PROM_BLOCK,
    STR_DB_PROM_ADDR, buffer );

   std::vector<std::string> out = CamStrDb::UnpackStrings( buffer );

   return out;
}

//////////////////////////// 
// DOWNLOAD    FIRMWARE
void AscentBasedUsbIo::DownloadFirmware()
{
    std::vector<UsbFrmwr::IntelHexRec> frmwr = 
        UsbFrmwr::MakeRecVect( firmware );

    PromFx2Io pf( m_Usb,
        ASCENT_EEPROM_MAX_BLOCKS,
        ASCENT_EEPROM_MAX_BANKS );

    pf.FirmwareDownload( frmwr );
}

//////////////////////////// 
//      READ      HEADER
void AscentBasedUsbIo::ReadHeader( Eeprom::Header & hdr )
{
    PromFx2Io pf( m_Usb,
        ASCENT_EEPROM_MAX_BLOCKS,
        ASCENT_EEPROM_MAX_BANKS );

    pf.ReadEepromHdr( hdr, HEADER_PROM_BANK,
        HEADER_PROM_BLOCK, HEADER_PROM_ADDR );
}

 //////////////////////////// 
//      PROGRAM  
void AscentBasedUsbIo::Program(const std::string & FilenameFpga,
    const std::string & FilenameFx2, const std::string & FilenameDescriptor,
    bool Print2StdOut)
{

    m_Print2StdOut = Print2StdOut;
    //STEP 1
    //download usb firmware if we have to
    uint16_t Vid  = 0; 
    uint16_t Pid = 0;
    uint16_t Did = 0;
    GetUsbVendorInfo( Vid, Pid, Did );
    if ( UsbFrmwr::CYPRESS_VID == Vid )
	{
        DownloadFirmware();
    }

    Progress2StdOut( 16 );

    //STEP 2
    // initialize prom header information
	Eeprom::Header hdr;
    memset(&hdr, 0, sizeof( hdr ) );
    hdr.Size = sizeof( hdr );
    hdr.Version = Eeprom::HEADER_VERSION;

    Progress2StdOut( 32 );

    //STEP 3
    //program fpga bits
    PromFx2Io pf( m_Usb,
        ASCENT_EEPROM_MAX_BLOCKS,
        ASCENT_EEPROM_MAX_BANKS );

    uint32_t fgpaDownloadSize = 0;
    pf.WriteFile2Eeprom( FilenameFpga, FPGA_PROM_BANK, 
        FPGA_PROM_BLOCK, FPGA_PROM_ADDR, fgpaDownloadSize );

    //xlinix firmware bits programmed set the "buf con" size and
    //valid bits.  for the ascent and the gee the buf con prom header is overloaded
    //to mean all xlinix firmware, since buf and cam con are in one package
    //now
    hdr.BufConSize = fgpaDownloadSize;
    hdr.Fields |= Eeprom::HEADER_BUFCON_VALID_BIT;

    Progress2StdOut( 48 );

    //STEP 4
    //download the fx2
    uint32_t DownloadSize = 0;
    pf.WriteFile2Eeprom( FilenameFx2, FX2_PROM_BANK,
        FX2_PROM_BLOCK, FX2_PROM_ADDR, DownloadSize );

    hdr.Fields |= Eeprom::HEADER_BOOTROM_VALID_BIT;

    Progress2StdOut( 64 );

    //STEP 5
    //download usb descriptors
    pf.WriteFile2Eeprom( FilenameDescriptor, DSCR_PROM_BANK,
        DSCR_PROM_BLOCK, DSCR_PROM_ADDR, DownloadSize );

     hdr.Fields |= Eeprom::HEADER_DESCRIPTOR_VALID_BIT;

     Progress2StdOut( 80 );

    //STEP 6
    //write the header
    hdr.CheckSum = Eeprom::CalcHdrCheckSum( hdr );

    pf.WriteEepromHdr( hdr, HEADER_PROM_BANK,
        HEADER_PROM_BLOCK, HEADER_PROM_ADDR);

    Progress2StdOut( 100 );

    //turn this off on exit
    m_Print2StdOut = false;
}

