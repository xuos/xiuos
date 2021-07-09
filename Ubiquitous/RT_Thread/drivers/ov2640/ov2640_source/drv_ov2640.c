/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2020-08-03     thread-liu        the first version
 * 2021-1-15      xiaoyu            add set resolution function and the way of reset different manufactures's ov2640 egg:zhendianyuanzi and weixue
 */

#include "board.h"
#include <rtthread.h>
#include <stdlib.h> 


#ifdef DRV_USING_OV2640
#include <drv_ov2640.h>
#ifdef RT_USING_POSIX
#include <dfs_posix.h>
#include <dfs_poll.h>
#ifdef RT_USING_POSIX_TERMIOS
#include <posix_termios.h>
#endif
#endif
#define DRV_DEBUG
#define LOG_TAG     "drv.ov2640"
#define DBG_LVL               DBG_LOG
#include <rtdbg.h>

/*  
resolution grade:
        	160,120,	//QQVGA 0
        	176,144,	//QCIF  1
        	320,240,	//QVGA  2
        	400,240,	//WQVGA 3
        	352,288,	//CIF   4
        	640,480,	//VGA   5
        	800,600,	//SVGA  6
        	1024,768,	//XGA   7
        	1280,800,	//WXGA  8
        	1280,960,	//XVGA  9
        	1440,900,	//WXGA+ 10
        	1280,1024,	//SXGA  11
        	1600,1200,	//UXGA	12     	
*/
#ifdef SOC_FAMILY_STM32
static rt_uint8_t g_ov2640_reso_level = 6;      //resolution level variable  0 ~ 12
#elif defined BOARD_K210_EVB
static rt_uint8_t g_ov2640_reso_level = 2;
#endif

static struct camera_device _ov2640_device;
const rt_uint8_t OV2640_AUTOEXPOSURE_LEVEL[5][8]=
{
    {
        0xFF,0x01,
        0x24,0x20,
        0x25,0x18,
        0x26,0x60,
    },
    {
        0xFF,0x01,
        0x24,0x34,
        0x25,0x1c,
        0x26,0x00,
    },
    {
        0xFF,0x01,
        0x24,0x3e,
        0x25,0x38,
        0x26,0x81,
    },
    {
        0xFF,0x01,
        0x24,0x48,
        0x25,0x40,
        0x26,0x81,
    },
    {
        0xFF,0x01,
        0x24,0x58,
        0x25,0x50,
        0x26,0x92,
    },
};
//SXGA(1600*1200) 
#ifdef  OV2640_ZDYZ
const rt_uint8_t ov2640_sxga_init_reg_tbl[][2]= 
{   
	0xff, 0x00,
	0x2c, 0xff,
	0x2e, 0xdf,
	0xff, 0x01,
	0x3c, 0x32,
	//
	0x11, 0x00,
	0x09, 0x02,
	0x04, 0xD8,
	0x13, 0xe5,
	0x14, 0x48,
	0x2c, 0x0c,
	0x33, 0x78,
	0x3a, 0x33,
	0x3b, 0xfB,
	//
	0x3e, 0x00,
	0x43, 0x11,
	0x16, 0x10,
	//
	0x39, 0x92,
	//
	0x35, 0xda,
	0x22, 0x1a,
	0x37, 0xc3,
	0x23, 0x00,
	0x34, 0xc0,
	0x36, 0x1a,
	0x06, 0x88,
	0x07, 0xc0,
	0x0d, 0x87,
	0x0e, 0x41,
	0x4c, 0x00,
	
	0x48, 0x00,
	0x5B, 0x00,
	0x42, 0x03,
	//
	0x4a, 0x81,
	0x21, 0x99,
	//
	0x24, 0x40,
	0x25, 0x38,
	0x26, 0x82,
	0x5c, 0x00,
	0x63, 0x00,
	0x46, 0x00,
	0x0c, 0x3c,
	//
	0x61, 0x70,
	0x62, 0x80,
	0x7c, 0x05,
	//
	0x20, 0x80,
	0x28, 0x30,
	0x6c, 0x00,
	0x6d, 0x80,
	0x6e, 0x00,
	0x70, 0x02,
	0x71, 0x94,
	0x73, 0xc1, 
	0x3d, 0x34, 
	0x5a, 0x57,
	//
	0x12, 0x00,//UXGA 1600*1200
	
	0x17, 0x11,
	0x18, 0x75,
	0x19, 0x01,
	0x1a, 0x97,
	0x32, 0x36,
	0x03, 0x0f, 
	0x37, 0x40,
	// 
	0x4f, 0xca,
	0x50, 0xa8,
	0x5a, 0x23,
	0x6d, 0x00,
	0x6d, 0x38,
	//
	0xff, 0x00,
	0xe5, 0x7f,
	0xf9, 0xc0,
	0x41, 0x24,
	0xe0, 0x14,
	0x76, 0xff,
	0x33, 0xa0,
	0x42, 0x20,
	0x43, 0x18,
	0x4c, 0x00,
	0x87, 0xd5,
	0x88, 0x3f,
	0xd7, 0x03,
	0xd9, 0x10,
	0xd3, 0x82,
	//
	0xc8, 0x08,
	0xc9, 0x80,
	//
	0x7c, 0x00,
	0x7d, 0x00,
	0x7c, 0x03,
	0x7d, 0x48,
	0x7d, 0x48,
	0x7c, 0x08,
	0x7d, 0x20,
	0x7d, 0x10,
	0x7d, 0x0e,
	//
	0x90, 0x00,
	0x91, 0x0e,
	0x91, 0x1a,
	0x91, 0x31,
	0x91, 0x5a,
	0x91, 0x69,
	0x91, 0x75,
	0x91, 0x7e,
	0x91, 0x88,
	0x91, 0x8f,
	0x91, 0x96,
	0x91, 0xa3,
	0x91, 0xaf,
	0x91, 0xc4,
	0x91, 0xd7,
	0x91, 0xe8,
	0x91, 0x20,
	//
	0x92, 0x00,
	0x93, 0x06,
	0x93, 0xe3,
	0x93, 0x05,
	0x93, 0x05,
	0x93, 0x00,
	0x93, 0x04,
	0x93, 0x00,
	0x93, 0x00,
	0x93, 0x00,
	0x93, 0x00,
	0x93, 0x00,
	0x93, 0x00,
	0x93, 0x00,
	//
	0x96, 0x00,
	0x97, 0x08,
	0x97, 0x19,
	0x97, 0x02,
	0x97, 0x0c,
	0x97, 0x24,
	0x97, 0x30,
	0x97, 0x28,
	0x97, 0x26,
	0x97, 0x02,
	0x97, 0x98,
	0x97, 0x80,
	0x97, 0x00,
	0x97, 0x00,
	//
	0xc3, 0xef,
	
	0xa4, 0x00,
	0xa8, 0x00,
	0xc5, 0x11,
	0xc6, 0x51,
	0xbf, 0x80,
	0xc7, 0x10,
	0xb6, 0x66,
	0xb8, 0xA5,
	0xb7, 0x64,
	0xb9, 0x7C,
	0xb3, 0xaf,
	0xb4, 0x97,
	0xb5, 0xFF,
	0xb0, 0xC5,
	0xb1, 0x94,
	0xb2, 0x0f,
	0xc4, 0x5c,
	//
	0xc0, 0xc8,
	0xc1, 0x96,
	0x8c, 0x00,
	0x86, 0x3d,
	0x50, 0x00,
	0x51, 0x90,
	0x52, 0x2c,
	0x53, 0x00,
	0x54, 0x00,
	0x55, 0x88,
	
	0x5a, 0x90,
	0x5b, 0x2C,
	0x5c, 0x05,
	
	0xd3, 0x04,//auto xiaoxin
	//
	0xc3, 0xed,
	0x7f, 0x00,
	
	0xda, 0x09,
	
	0xe5, 0x1f,
	0xe1, 0x67,
	0xe0, 0x00,
	0xdd, 0x7f,
	0x05, 0x00,
};
#else
const rt_uint8_t ov2640_sxga_init_reg_tbl[][2]=
{
    {0xff, 0x00},
    {0x2c, 0xff},
    {0x2e, 0xdf},
    {0xff, 0x01},
    {0x3c, 0x32},
    {0x11, 0x00},
    {0x09, 0x02},
    {0x04, 0xD8},
    {0x13, 0xe5},
    {0x14, 0x48},
    {0x2c, 0x0c},
    {0x33, 0x78},
    {0x3a, 0x33},
    {0x3b, 0xfB},
    {0x3e, 0x00},
    {0x43, 0x11},
    {0x16, 0x10},
    {0x39, 0x92},
    {0x35, 0xda},
    {0x22, 0x1a},
    {0x37, 0xc3},
    {0x23, 0x00},
    {0x34, 0xc0},
    {0x36, 0x1a},
    {0x06, 0x88},
    {0x07, 0xc0},
    {0x0d, 0x87},
    {0x0e, 0x41},
    {0x4c, 0x00},
    {0x48, 0x00},
    {0x5B, 0x00},
    {0x42, 0x03},
    {0x4a, 0x81},
    {0x21, 0x99},
    {0x24, 0x40},
    {0x25, 0x38},
    {0x26, 0x82},
    {0x5c, 0x00},
    {0x63, 0x00},
    {0x46, 0x00},
    {0x0c, 0x3c},
    {0x61, 0x70},
    {0x62, 0x80},
    {0x7c, 0x05},
    {0x20, 0x80},
    {0x28, 0x30},
    {0x6c, 0x00},
    {0x6d, 0x80},
    {0x6e, 0x00},
    {0x70, 0x02},
    {0x71, 0x94},
    {0x73, 0xc1},
    {0x3d, 0x34},
    {0x5a, 0x57},
    {0x12, 0x00},
    {0x17, 0x11},
    {0x18, 0x75},
    {0x19, 0x01},
    {0x1a, 0x97},
    {0x32, 0x36},
    {0x03, 0x0f},
    {0x37, 0x40},
    {0x4f, 0xca},
    {0x50, 0xa8},
    {0x5a, 0x23},
    {0x6d, 0x00},
    {0x6d, 0x38},
    {0xff, 0x00},
    {0xe5, 0x7f},
    {0xf9, 0xc0},
    {0x41, 0x24},
    {0xe0, 0x14},
    {0x76, 0xff},
    {0x33, 0xa0},
    {0x42, 0x20},
    {0x43, 0x18},
    {0x4c, 0x00},
    {0x87, 0xd5},
    {0x88, 0x3f},
    {0xd7, 0x03},
    {0xd9, 0x10},
    {0xd3, 0x82},
    {0xc8, 0x08},
    {0xc9, 0x80},
    {0x7c, 0x00},
    {0x7d, 0x00},
    {0x7c, 0x03},
    {0x7d, 0x48},
    {0x7d, 0x48},
    {0x7c, 0x08},
    {0x7d, 0x20},
    {0x7d, 0x10},
    {0x7d, 0x0e},
    {0x90, 0x00},
    {0x91, 0x0e},
    {0x91, 0x1a},
    {0x91, 0x31},
    {0x91, 0x5a},
    {0x91, 0x69},
    {0x91, 0x75},
    {0x91, 0x7e},
    {0x91, 0x88},
    {0x91, 0x8f},
    {0x91, 0x96},
    {0x91, 0xa3},
    {0x91, 0xaf},
    {0x91, 0xc4},
    {0x91, 0xd7},
    {0x91, 0xe8},
    {0x91, 0x20},
    {0x92, 0x00},
    {0x93, 0x06},
    {0x93, 0xe3},
    {0x93, 0x05},
    {0x93, 0x05},
    {0x93, 0x00},
    {0x93, 0x04},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x96, 0x00},
    {0x97, 0x08},
    {0x97, 0x19},
    {0x97, 0x02},
    {0x97, 0x0c},
    {0x97, 0x24},
    {0x97, 0x30},
    {0x97, 0x28},
    {0x97, 0x26},
    {0x97, 0x02},
    {0x97, 0x98},
    {0x97, 0x80},
    {0x97, 0x00},
    {0x97, 0x00},
    {0xc3, 0xef},
    {0xa4, 0x00},
    {0xa8, 0x00},
    {0xc5, 0x11},
    {0xc6, 0x51},
    {0xbf, 0x80},
    {0xc7, 0x10},
    {0xb6, 0x66},
    {0xb8, 0xA5},
    {0xb7, 0x64},
    {0xb9, 0x7C},
    {0xb3, 0xaf},
    {0xb4, 0x97},
    {0xb5, 0xFF},
    {0xb0, 0xC5},
    {0xb1, 0x94},
    {0xb2, 0x0f},
    {0xc4, 0x5c},
    {0xc0, 0xc8},
    {0xc1, 0x96},
    {0x8c, 0x00},
    {0x86, 0x3d},
    {0x50, 0x00},
    {0x51, 0x90},
    {0x52, 0x2c},
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x88},
    {0x5a, 0x90},
    {0x5b, 0x2C},
    {0x5c, 0x05},
    {0xd3, 0x02},
    {0xc3, 0xed},
    {0x7f, 0x00},
    {0xda, 0x09},
    {0xe5, 0x1f},
    {0xe1, 0x67},
    {0xe0, 0x00},
    {0xdd, 0x7f},
    {0x05, 0x00},
};
#endif
/* SVGA 800*600 */
const rt_uint8_t ov2640_svga_init_reg_tbl[][2]=
{
    {0xff, 0x00},
    {0x2c, 0xff},
    {0x2e, 0xdf},
    {0xff, 0x01},
    {0x3c, 0x32},
    {0x11, 0x00},
    {0x09, 0x02},
    {0x04, 0xD8},  
    {0x13, 0xe5},
    {0x14, 0x48},
    {0x2c, 0x0c},
    {0x33, 0x78},
    {0x3a, 0x33},
    {0x3b, 0xfB},
    {0x3e, 0x00},
    {0x43, 0x11},
    {0x16, 0x10},
    {0x39, 0x92},
    {0x35, 0xda},
    {0x22, 0x1a},
    {0x37, 0xc3},
    {0x23, 0x00},
    {0x34, 0xc0},
    {0x36, 0x1a},
    {0x06, 0x88},
    {0x07, 0xc0},
    {0x0d, 0x87},
    {0x0e, 0x41},
    {0x4c, 0x00},
    {0x48, 0x00},
    {0x5B, 0x00},
    {0x42, 0x03},
    {0x4a, 0x81},
    {0x21, 0x99},
    {0x24, 0x40},
    {0x25, 0x38},
    {0x26, 0x82},
    {0x5c, 0x00},
    {0x63, 0x00},
    {0x46, 0x22},
    {0x0c, 0x3c},
    {0x61, 0x70},
    {0x62, 0x80},
    {0x7c, 0x05},
    {0x20, 0x80},
    {0x28, 0x30},
    {0x6c, 0x00},
    {0x6d, 0x80},
    {0x6e, 0x00},
    {0x70, 0x02},
    {0x71, 0x94},
    {0x73, 0xc1},
    {0x3d, 0x34},
    {0x5a, 0x57},
    {0x12, 0x40},
    {0x17, 0x11},
    {0x18, 0x43},
    {0x19, 0x00},
    {0x1a, 0x4b},
    {0x32, 0x09},
    {0x37, 0xc0},
    {0x4f, 0xca},
    {0x50, 0xa8},
    {0x5a, 0x23},
    {0x6d, 0x00},
    {0x3d, 0x38},
    {0xff, 0x00},
    {0xe5, 0x7f},
    {0xf9, 0xc0},
    {0x41, 0x24},
    {0xe0, 0x14},
    {0x76, 0xff},
    {0x33, 0xa0},
    {0x42, 0x20},
    {0x43, 0x18},
    {0x4c, 0x00},
    {0x87, 0xd5},
    {0x88, 0x3f},
    {0xd7, 0x03},
    {0xd9, 0x10},
    {0xd3, 0x82},
    {0xc8, 0x08},
    {0xc9, 0x80},
    {0x7c, 0x00},
    {0x7d, 0x00},
    {0x7c, 0x03},
    {0x7d, 0x48},
    {0x7d, 0x48},
    {0x7c, 0x08},
    {0x7d, 0x20},
    {0x7d, 0x10},
    {0x7d, 0x0e},
    {0x90, 0x00},
    {0x91, 0x0e},
    {0x91, 0x1a},
    {0x91, 0x31},
    {0x91, 0x5a},
    {0x91, 0x69},
    {0x91, 0x75},
    {0x91, 0x7e},
    {0x91, 0x88},
    {0x91, 0x8f},
    {0x91, 0x96},
    {0x91, 0xa3},
    {0x91, 0xaf},
    {0x91, 0xc4},
    {0x91, 0xd7},
    {0x91, 0xe8},
    {0x91, 0x20},
    {0x92, 0x00},
    {0x93, 0x06},
    {0x93, 0xe3},
    {0x93, 0x05},
    {0x93, 0x05},
    {0x93, 0x00},
    {0x93, 0x04},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x96, 0x00},
    {0x97, 0x08},
    {0x97, 0x19},
    {0x97, 0x02},
    {0x97, 0x0c},
    {0x97, 0x24},
    {0x97, 0x30},
    {0x97, 0x28},
    {0x97, 0x26},
    {0x97, 0x02},
    {0x97, 0x98},
    {0x97, 0x80},
    {0x97, 0x00},
    {0x97, 0x00},
    {0xc3, 0xed},
    {0xa4, 0x00},
    {0xa8, 0x00},
    {0xc5, 0x11},
    {0xc6, 0x51},
    {0xbf, 0x80},
    {0xc7, 0x10},
    {0xb6, 0x66},
    {0xb8, 0xA5},
    {0xb7, 0x64},
    {0xb9, 0x7C},
    {0xb3, 0xaf},
    {0xb4, 0x97},
    {0xb5, 0xFF},
    {0xb0, 0xC5},
    {0xb1, 0x94},
    {0xb2, 0x0f},
    {0xc4, 0x5c},
    {0xc0, 0x64},
    {0xc1, 0x4B},
    {0x8c, 0x00},
    {0x86, 0x3D},
    {0x50, 0x00},
    {0x51, 0xC8},
    {0x52, 0x96},
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x00},
    {0x5a, 0xC8},
    {0x5b, 0x96},
    {0x5c, 0x00},
    {0xd3, 0x02},
    {0xc3, 0xed},
    {0x7f, 0x00},
    {0xda, 0x09},
    {0xe5, 0x1f},
    {0xe1, 0x67},
    {0xe0, 0x00},
    {0xdd, 0x7f},
    {0x05, 0x00},
};

const rt_uint8_t ov2640_jpeg_reg_tbl[][2]=
{
    {0xff, 0x01},
    {0xe0, 0x14},
    {0xe1, 0x77},
    {0xe5, 0x1f},
    {0xd7, 0x03},
    {0xda, 0x10},
    {0xe0, 0x00},
};

const rt_uint8_t ov2640_rgb565_reg_tbl[][2]=
{
    {0xFF, 0x00},
    {0xDA, 0x08},
    {0xD7, 0x03},
    {0xDF, 0x02},
    {0x33, 0xa0},
    {0x3C, 0x00},
    {0xe1, 0x67},

    {0xff, 0x01},
    {0xe0, 0x00},
    {0xe1, 0x00},
    {0xe5, 0x00},
    {0xd7, 0x00},
    {0xda, 0x00},
    {0xe0, 0x00},
};
const rt_uint8_t ov2640_yuv422_reg_tbl[][2] =
{
    {0xFF, 0x00},
    {0xDA, 0x10},
    {0xD7, 0x03},
    {0xDF, 0x00},
    {0x33, 0x80},
    {0x3C, 0x40},
    {0xe1, 0x77},
    {0x00, 0x00},
};


const rt_uint16_t jpeg_img_size_tbl[][2]=
{
	160,120,	//QQVGA 0
	176,144,	//QCIF  1
	320,240,	//QVGA  2
	400,240,	//WQVGA 3
	352,288,	//CIF   4
	640,480,	//VGA   5
	800,600,	//SVGA  6
	1024,768,	//XGA   7
	1280,800,	//WXGA  8
	1280,960,	//XVGA  9
	1440,900,	//WXGA+ 10
	1280,1024,	//SXGA  11
	1600,1200,	//UXGA	12
};

struct rt_i2c_bus_device *i2c_bus  = RT_NULL;
/* event control block*/
static struct rt_event g_rec_photo_event;
/*ov2640 mutex */
static rt_mutex_t g_ov2640_mutex = RT_NULL;

static rt_err_t sccb_read_reg(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t *buf)
{
    if(NULL == buf)
    {
        LOG_E("buf == NULL");
        return RT_ERROR;
    }
    #ifdef SOC_FAMILY_STM32
    //stm32 bsp
    struct rt_i2c_msg msg[2];

    RT_ASSERT(bus != RT_NULL);
    
    msg[0].addr  = DEV_ADDRESS;
    msg[0].flags = RT_I2C_WR;
    msg[0].buf   = &reg;
    msg[0].len   = 2;
    msg[1].addr  = DEV_ADDRESS;
    msg[1].flags = RT_I2C_RD;
    msg[1].len   = 1;
    msg[1].buf   = buf;
    if (rt_i2c_transfer(bus, msg, 2) == 2)
    {
        return RT_EOK;
    }
    return RT_ERROR;
    #elif defined BOARD_K210_EVB
    //k210 bsp
    *buf =dvp_sccb_receive_data(DEV_ADDRESS,reg);
    return RT_EOK;
    #endif
}

/* i2c write reg */
static rt_err_t sccb_write_reg(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t data)
{
    #ifdef SOC_FAMILY_STM32
    rt_uint8_t buf[2];
    struct rt_i2c_msg msgs;

    RT_ASSERT(bus != RT_NULL);

    buf[0] = reg ;
    buf[1] = data;

    msgs.addr = DEV_ADDRESS;
    msgs.flags = RT_I2C_WR;
    msgs.buf = buf;
    msgs.len = 2;
    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }	
    return RT_ERROR;
    #elif defined BOARD_K210_EVB
    dvp_sccb_send_data(DEV_ADDRESS,reg,data);
    return RT_EOK;
    #endif
}

static rt_err_t ov2640_read_id(struct rt_i2c_bus_device *bus)
{
    rt_uint8_t read_value[2];
    rt_uint16_t id = 0;
    sccb_read_reg(bus, OV2640_SENSOR_MIDH,&read_value[0]);
    sccb_read_reg(bus, OV2640_SENSOR_MIDL,&read_value[1]);
    id = ((rt_uint16_t)(read_value[0] << 8) & 0xFF00);
    id |= ((rt_uint16_t)(read_value[1]) & 0x00FF);

    if (id != OV2640_MID)
    {
        LOG_E("ov2640 init error, mid: 0x%x    ", id);
        return RT_ERROR;
    }

    LOG_I("ov2640 read mid success, mid: 0x%x    ", id);

    sccb_read_reg(bus, OV2640_SENSOR_PIDH, &read_value[0]);
    sccb_read_reg(bus, OV2640_SENSOR_PIDL, &read_value[1]);
    id = ((rt_uint16_t)(read_value[0] << 8) & 0xFF00);
    id |= ((rt_uint16_t)(read_value[1]) & 0x00FF);

    if (id != OV2640_PID)
    {
        LOG_E("ov2640 init error, pid: 0x%04x     ", id);
        return RT_ERROR;
    }

    LOG_I("ov2640 read hid success, pid: 0x%04x", id);

    return RT_EOK;
}

/* change ov2640 to jpeg mode */
void ov2640_jpeg_mode(void)
{
    rt_uint16_t i=0;
    /* set yun422 mode */
    for (i = 0; i < (sizeof(ov2640_yuv422_reg_tbl) / 2); i++)
    {
        sccb_write_reg(i2c_bus, ov2640_yuv422_reg_tbl[i][0],ov2640_yuv422_reg_tbl[i][1]);
    }

    /* set jpeg mode */
    for(i=0;i<(sizeof(ov2640_jpeg_reg_tbl)/2);i++)
    {
        sccb_write_reg(i2c_bus, ov2640_jpeg_reg_tbl[i][0],ov2640_jpeg_reg_tbl[i][1]);
    }
}

/* change ov2640 to rgb565 mode */
void ov2640_rgb565_mode(void)
{
    rt_uint16_t i=0;
    for (i = 0; i < (sizeof(ov2640_rgb565_reg_tbl) / 2); i++)
    {
        sccb_write_reg(i2c_bus, ov2640_rgb565_reg_tbl[i][0],ov2640_rgb565_reg_tbl[i][1]);
    }
}



/* set auto exposure level value must be 0 ~4 */
void ov2640_set_auto_exposure(rt_uint8_t level)
{
    rt_uint8_t i = 0;
    rt_uint8_t *p = (rt_uint8_t*)OV2640_AUTOEXPOSURE_LEVEL[level];
    for (i = 0; i < 4; i++)
    {
        sccb_write_reg(i2c_bus, p[i*2],p[i*2+1]);
    }
}

/* set light mode
 * 0: auto
 * 1: sunny
 * 2: cloudy
 * 3: office
 * 4: home
 * */
void ov2640_set_light_mode(rt_uint8_t mode)
{
    rt_uint8_t regccval, regcdval, regceval;

    switch(mode)
    {
        case 0:
            sccb_write_reg(i2c_bus, 0xFF, 0x00);
            sccb_write_reg(i2c_bus, 0xC7, 0x10);
            return;

        case 2:
            regccval = 0x65;
            regcdval = 0x41;
            regceval = 0x4F;
            break;

        case 3:
            regccval = 0x52;
            regcdval = 0x41;
            regceval = 0x66;
            break;

        case 4:
            regccval = 0x42;
            regcdval = 0x3F;
            regceval = 0x71;
            break;

        default:
            regccval = 0x5E;
            regcdval = 0x41;
            regceval = 0x54;
            break;
    }

    sccb_write_reg(i2c_bus, 0xFF, 0x00);
    sccb_write_reg(i2c_bus, 0xC7, 0x40);
    sccb_write_reg(i2c_bus, 0xCC, regccval);
    sccb_write_reg(i2c_bus, 0xCD, regcdval);
    sccb_write_reg(i2c_bus, 0xCE, regceval);
}

/* set color saturation
 * 0: -2
 * 1: -1
 * 2: 0
 * 3: +1
 * 4: +2
 * */
void ov2640_set_color_saturation(rt_uint8_t sat)
{
    rt_uint8_t reg7dval = ((sat+2)<<4) | 0x08;
    sccb_write_reg(i2c_bus, 0xFF, 0X00);
    sccb_write_reg(i2c_bus, 0x7C, 0X00);
    sccb_write_reg(i2c_bus, 0x7D, 0X02);
    sccb_write_reg(i2c_bus, 0x7C, 0X03);
    sccb_write_reg(i2c_bus, 0x7D, reg7dval);
    sccb_write_reg(i2c_bus, 0x7D, reg7dval);
}

/* set brightness
 * 0: -2
 * 1: -1
 * 2: 0
 * 3: 1
 * 4: 2
 * */
void ov2640_set_brightness(rt_uint8_t bright)
{
  sccb_write_reg(i2c_bus, 0xff, 0x00);
  sccb_write_reg(i2c_bus, 0x7c, 0x00);
  sccb_write_reg(i2c_bus, 0x7d, 0x04);
  sccb_write_reg(i2c_bus, 0x7c, 0x09);
  sccb_write_reg(i2c_bus, 0x7d, bright << 4);
  sccb_write_reg(i2c_bus, 0x7d, 0x00);
}

/* set contrast
 * 0: -2
 * 1: -1
 * 2: 0
 * 3: 1
 * 4: 2
 * */
void ov2640_set_contrast(rt_uint8_t contrast)
{
    rt_uint8_t reg7d0val, reg7d1val;

    switch(contrast)
    {
        case 0:
            reg7d0val = 0x18;
            reg7d1val = 0x34;
            break;

        case 1:
            reg7d0val = 0x1C;
            reg7d1val = 0x2A;
            break;

        case 3:
            reg7d0val = 0x24;
            reg7d1val = 0x16;
            break;

        case 4:
            reg7d0val = 0x28;
            reg7d1val = 0x0C;
            break;

        default:
            reg7d0val = 0x20;
            reg7d1val = 0x20;
            break;
    }
    sccb_write_reg(i2c_bus, 0xff, 0x00);
    sccb_write_reg(i2c_bus, 0x7c, 0x00);
    sccb_write_reg(i2c_bus, 0x7d, 0x04);
    sccb_write_reg(i2c_bus, 0x7c, 0x07);
    sccb_write_reg(i2c_bus, 0x7d, 0x20);
    sccb_write_reg(i2c_bus, 0x7d, reg7d0val);
    sccb_write_reg(i2c_bus, 0x7d, reg7d1val);
    sccb_write_reg(i2c_bus, 0x7d, 0x06);
}

/* set special effects
 * 0: noraml
 * 1: negative film
 * 2: black-and-white
 * 3: the red
 * 4: the green
 * 5: the blue
 * 6: Retro
*/
void ov2640_set_special_effects(rt_uint8_t eft)
{
    rt_uint8_t reg7d0val, reg7d1val, reg7d2val;

    switch(eft)
    {
        case 1:
            reg7d0val = 0x40;
            break;
        case 2:
            reg7d0val = 0x18;
            break;
        case 3:
            reg7d0val = 0x18;
            reg7d1val = 0x40;
            reg7d2val = 0xC0;
            break;
        case 4:
            reg7d0val = 0x18;
            reg7d1val = 0x40;
            reg7d2val = 0x40;
            break;
        case 5:
            reg7d0val = 0x18;
            reg7d1val = 0xA0;
            reg7d2val = 0x40;
            break;
        case 6:
            reg7d0val = 0x18;
            reg7d1val = 0x40;
            reg7d2val = 0xA6;
            break;
        default:
            reg7d0val = 0x00;
            reg7d1val = 0x80;
            reg7d2val = 0x80;
            break;
    }
    sccb_write_reg(i2c_bus, 0xff, 0x00);
    sccb_write_reg(i2c_bus, 0x7c, 0x00);
    sccb_write_reg(i2c_bus, 0x7d, reg7d0val);
    sccb_write_reg(i2c_bus, 0x7c, 0x05);
    sccb_write_reg(i2c_bus, 0x7d, reg7d1val);
    sccb_write_reg(i2c_bus, 0x7d, reg7d2val);
}

/* set the image output window */
void ov2640_set_window_size(rt_uint16_t sx,rt_uint16_t sy,rt_uint16_t width,rt_uint16_t height)
{
    rt_uint16_t endx;
    rt_uint16_t endy;
    rt_uint8_t temp;
    endx = sx + width / 2;
    endy = sy + height / 2;

    sccb_write_reg(i2c_bus, 0xFF, 0x01);
    sccb_read_reg(i2c_bus, 0x03, &temp);
    temp &= 0xF0;
    temp |= ((endy & 0x03) << 2) | (sy & 0x03);
    sccb_write_reg(i2c_bus, 0x03, temp);
    sccb_write_reg(i2c_bus, 0x19, sy>>2);
    sccb_write_reg(i2c_bus, 0x1A, endy>>2);

    sccb_read_reg(i2c_bus, 0x32, &temp);
    temp &= 0xC0;
    temp |= ((endx & 0x07) << 3) | (sx & 0x07);
    sccb_write_reg(i2c_bus, 0x32, temp);
    sccb_write_reg(i2c_bus, 0x17, sx>>3);
    sccb_write_reg(i2c_bus, 0x18, endx>>3);
}

/* set the image output size */
rt_uint8_t ov2640_set_image_out_size(rt_uint16_t width,rt_uint16_t height)
{
    rt_uint16_t outh, outw;
    rt_uint8_t temp;

    if(width%4)return 1;
    if(height%4)return 2;
    outw = width /4;
    outh = height/4;
    sccb_write_reg(i2c_bus, 0xFF, 0x00);
    sccb_write_reg(i2c_bus, 0xE0, 0x04);
    sccb_write_reg(i2c_bus, 0x5A, outw & 0XFF);
    sccb_write_reg(i2c_bus, 0x5B, outh & 0XFF);
    temp = (outw >> 8) & 0x03;
    temp |= (outh >> 6) & 0x04;
    sccb_write_reg(i2c_bus, 0x5C, temp);
    sccb_write_reg(i2c_bus, 0xE0, 0X00);

    return RT_EOK;
}

/* set the image window size */
rt_uint8_t ov2640_set_image_window_size(rt_uint16_t offx, rt_uint16_t offy, rt_uint16_t width, rt_uint16_t height)
{
    rt_uint16_t hsize, vsize;
    rt_uint8_t temp;
    if ((width % 4) || (height%4))
    {
        return RT_ERROR;
    }
    hsize = width /4;
    vsize = height/4;
   sccb_write_reg(i2c_bus, 0XFF,0X00);
   sccb_write_reg(i2c_bus, 0XE0,0X04);
   sccb_write_reg(i2c_bus, 0X51,hsize&0XFF);
   sccb_write_reg(i2c_bus, 0X52,vsize&0XFF);
   sccb_write_reg(i2c_bus, 0X53,offx&0XFF);
   sccb_write_reg(i2c_bus, 0X54,offy&0XFF);
   temp=(vsize>>1)&0X80;
   temp|=(offy>>4)&0X70;
   temp|=(hsize>>5)&0X08;
   temp|=(offx>>8)&0X07;
   sccb_write_reg(i2c_bus, 0X55,temp);             //
   sccb_write_reg(i2c_bus, 0X57,(hsize>>2)&0X80);  //
   sccb_write_reg(i2c_bus, 0XE0,0X00);
   return 0;
}

/* set output resolution */
rt_uint8_t ov2640_set_image_size(rt_uint16_t width ,rt_uint16_t height)
{
   rt_uint8_t temp;
   sccb_write_reg(i2c_bus, 0xFF, 0x00);
   sccb_write_reg(i2c_bus, 0xE0, 0x04);
   sccb_write_reg(i2c_bus, 0xC0, (width >>3) & 0xFF);
   sccb_write_reg(i2c_bus, 0xC1, (height >> 3) & 0xFF);
   temp = (width & 0x07) << 3;
   temp |= height & 0x07;
   temp |= (width >> 4) & 0x80;
   sccb_write_reg(i2c_bus, 0x8C, temp);
   sccb_write_reg(i2c_bus, 0xE0, 0x00);

   return RT_EOK;
}


int rt_ov2640_readid_test()
{
    #ifdef SOC_FAMILY_STM32
    i2c_bus = rt_i2c_bus_device_find(I2C_NAME);
    if (i2c_bus == RT_NULL)
    {
        LOG_E("can't find %c deivce", I2C_NAME);
        return RT_ERROR;
    }
    #endif
    sccb_write_reg(i2c_bus, OV2640_DSP_RA_DLMT, 0x01);
    rt_thread_delay(10);
    sccb_write_reg(i2c_bus, OV2640_SENSOR_COM7, 0x80);

    ov2640_read_id(i2c_bus);
    return RT_EOK;
}

rt_uint32_t rt_ov2640_calculate_jpeg_len(rt_uint8_t* pdata,rt_uint32_t maxlength)
{
    rt_uint32_t length = 0;
    if((*pdata != 0xFF)&&(*(pdata+1) != 0xD8))
    {
        LOG_E("the data is error this time ");
        return 0;
    }
    for(;length<maxlength;length++)
    {
        if((*(pdata+length)== 0xFF)&&(*(pdata+length+1) == 0xD9))
        {
            return length+1+1;
        }
    }
    LOG_E("the data tail is error this time ");
    return 0;
}

#ifdef SOC_FAMILY_STM32
/*
Different hardware circuits have different reset modes
*/
#ifdef OV2640_ZDYZ
void rt_stm32407_atk_miniexpolre()
{
    
    /*
        no pull-up resistance
    */
    GPIO_InitTypeDef GPIO_Initure = {0};
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();  //
    GPIO_Initure.Pin=GPIO_PIN_6;
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  
    GPIO_Initure.Pull=GPIO_PULLUP;          
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;     
    HAL_GPIO_Init(GPIOD,&GPIO_Initure); 
    GPIO_Initure.Pin=GPIO_PIN_7;
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_OD;  
    GPIO_Initure.Pull=GPIO_PULLUP;          
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;     
    HAL_GPIO_Init(GPIOD,&GPIO_Initure); 
         
    GPIO_Initure.Pin=GPIO_PIN_9|GPIO_PIN_15;//PG9,15  PG9---->PWDN  PG15------>RST
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  
    GPIO_Initure.Pull=GPIO_PULLUP;          
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     
    HAL_GPIO_Init(GPIOG,&GPIO_Initure);     
    rt_thread_mdelay(10);
    HAL_GPIO_WritePin(GPIOG,GPIO_PIN_9,GPIO_PIN_RESET);
    rt_thread_mdelay(10);
    HAL_GPIO_WritePin(GPIOG,GPIO_PIN_15,GPIO_PIN_RESET);
    rt_thread_mdelay(10);
    HAL_GPIO_WritePin(GPIOG,GPIO_PIN_15,GPIO_PIN_SET);
 
  
   
}
#else
void rt_ov2460_rcc_int(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
   __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);
}
#endif

#endif

void ov2640_set_resolution(int value)
{
    int i  = 0;
    if(value < 0 || value >13)
    {
        LOG_E("resolution parameter is error!");
        return;
    }
    if(13 == value)
    {
        LOG_I("set image :");
        LOG_I("       160,120,    QQVGA 0 ");    
        LOG_I("       176,144,	  QCIF  1 ");
        LOG_I("       320,240,    QVGA  2" );
        LOG_I("       400,240,    WQVGA 3 ");
        LOG_I("       352,288,    CIF   4" );
        LOG_I("       640,480,    VGA   5 ");
        LOG_I("       800,600,    SVGA  6");
        LOG_I("       1024,768,   XGA   7 ");
        LOG_I("       1280,800,   WXGA  8 ");
        LOG_I("       1280,960,   XVGA  9 ");
        LOG_I("       1440,900,   WXGA+ 10");
        LOG_I("       1280,1024,  SXGA  11");
        LOG_I("       1280,1024,  UXGA  12"); 
        return;
    }
    g_ov2640_reso_level = value;
    #ifdef SOC_FAMILY_STM32
        #ifdef OV2640_ZDYZ
        rt_stm32407_atk_miniexpolre();
        #else
        /*
           At present, the design of RST reset pin is not carried out by Micro Snow Electronics, 
           which can be added later. Therefore, there will be problems if the micro snow electronic setting is connected here
        */
        rt_ov2460_rcc_int();
        #endif
    #endif
    sccb_write_reg(i2c_bus, OV2640_DSP_RA_DLMT, 0x01);
    sccb_write_reg(i2c_bus, OV2640_SENSOR_COM7, 0x80);
    ov2640_read_id(i2c_bus);
    if(g_ov2640_reso_level > LARGE_PHOTO_MODE)
    {
        for (i = 0; i < sizeof(ov2640_sxga_init_reg_tbl) / 2; i++)
        {
            sccb_write_reg(i2c_bus, ov2640_sxga_init_reg_tbl[i][0], ov2640_sxga_init_reg_tbl[i][1]);
        }
        LOG_I("out put big picture mode ");
    }
    else
    {
        for (i = 0; i < sizeof(ov2640_svga_init_reg_tbl) / 2; i++)
        {
            sccb_write_reg(i2c_bus, ov2640_svga_init_reg_tbl[i][0], ov2640_svga_init_reg_tbl[i][1]);
        }
        LOG_I("out put small picture mode ");
    }
    ov2640_rgb565_mode(); 
    ov2640_jpeg_mode();
    ov2640_set_image_window_size(0, 0, jpeg_img_size_tbl[g_ov2640_reso_level ][0],jpeg_img_size_tbl[g_ov2640_reso_level ][1]);	
    ov2640_set_image_out_size(jpeg_img_size_tbl[g_ov2640_reso_level ][0], jpeg_img_size_tbl[g_ov2640_reso_level ][1]);
    LOG_I("set image resolution is %d * %d  success\n\r",jpeg_img_size_tbl[g_ov2640_reso_level ][0],jpeg_img_size_tbl[g_ov2640_reso_level ][1]);
}


/*
set ov2640 configuration function 
*/
void set_ov2640_config(int argc, char **argv)
{
    rt_int8_t cmd = 0;
    rt_int16_t value = 0;
    LOG_I("set ov2640 configuration :");
    if(argc < 2)
    {
         LOG_E("Usage: set_ov2640_config  grade");
         LOG_E("like: set_ov2640_config  25(CMD)  2(value)  (cmd 25 (color saturation) value is 2 (0~4)");
         return;
    }
    
    if(NULL != strstr(argv[1],"help"))
    {
        LOG_I("  set_ov2640_config CMD value");
        #ifdef SOC_FAMILY_STM32
        LOG_I(" CMD 23 is setting resolution,value should be 0~12(0-12 is resolution level value 13 display particulars)");
        #endif
        LOG_I(" CMD 24 is setting light mode,value should be 0~4(0: auto 1: sunny,2: cloudy,3: office,4:home)");
        LOG_I(" CMD 25 is setting color saturation,value should be 0~4(0: -2 1:-1,2:0,3:1,4: 2)");
        LOG_I(" CMD 26 is setting brightness,value should be 0~4(0: -2 1:-1,2:0,3:1,4: 2)");
        LOG_I(" CMD 27 is setting contrast,value should be 0~4(0: -2 1:-1,2:0,3:1,4: 2)");
        LOG_I(" CMD 28 is setting effects,value should be 0~6:");
        LOG_I("(0: noraml 1: negative film 2: black-and-white 3: the red 4: the green 5: the blue 6: Retro)");
        LOG_I(" CMD 29 is setting exposure,value should be 0~4(0: -2 1:-1,2:0,3:1,4: 2)");
    }
    cmd = strtoul(argv[1], 0, 10);
    value = strtoul(argv[2], 0, 10);
    LOG_I("CMD is %d vaule is %d \n",cmd,value);
    if(cmd < IOCTRL_CAMERA_SET_RESO || cmd > IOCTRL_CAMERA_SET_EXPOSURE)
    {
        LOG_I("CMD value should be 24 ~29");
        return ;
    }
    switch(cmd)
    {
        #ifdef SOC_FAMILY_STM32
        case IOCTRL_CAMERA_SET_RESO:
            if(value > 13 || value < 0)
            {
                LOG_I("value should be 0 ~13");
                return ;
            }
            ov2640_set_resolution(value);
            break;
       #endif
        case IOCTRL_CAMERA_SET_LIGHT:
            if(value > 4 || value < 0)
            {
                LOG_I("value should be 0 ~13");
                return ;
            }
            ov2640_set_light_mode(value);
            break;
        case IOCTRL_CAMERA_SET_COLOR:
            if(value > 4 || value < 0)
            {
                LOG_I("value should be 0 ~13");
                return ;
            }
            ov2640_set_color_saturation(value);
            break;
        case IOCTRL_CAMERA_SET_BRIGHTNESS:
            if(value > 4 || value < 0)
            {
                LOG_I("value should be 0 ~13");
                return ;
            }
            ov2640_set_brightness(value);
            break;
        case IOCTRL_CAMERA_SET_CONTRAST:
            if(value > 4 || value < 0)
            {
                LOG_I("value should be 0 ~13");
                return ;
            }
            ov2640_set_contrast(value);
            break;
        case IOCTRL_CAMERA_SET_EFFECT:
            if(value > 6 || value < 0)
            {
                LOG_I("value should be 0 ~13");
                return ;
            }
            ov2640_set_special_effects(value);
            break;
        case IOCTRL_CAMERA_SET_EXPOSURE:
            if(value > 4 || value < 0)
            {
                LOG_I("value should be 0 ~13");
                return ;
            }
            ov2640_set_auto_exposure(value);
            break;
        default:
            break;
    }
}
MSH_CMD_EXPORT(set_ov2640_config,set ov2640 camera configuration);








void rt_ov2640_frame_irq_event()
{
    rt_event_send(&g_rec_photo_event, RECV_EVENT_FLAG); 
}


rt_err_t rt_ov2640_set_irq_hander(void (*p)(void))
{
    
    #ifdef SOC_FAMILY_STM32
    return rt_set_irq_dcmi_callback_hander(p);
    #elif defined BOARD_K210_EVB
    return rt_set_irq_dvp_callback_hander(p);
    #endif
}

rt_err_t rt_ov2640_start_shoot(uint32_t pdata, uint32_t length)
{
    
    rt_mutex_take(g_ov2640_mutex, RT_WAITING_FOREVER);
    #ifdef SOC_FAMILY_STM32
    rt_dcmi_start(pdata,length);    
    #elif defined BOARD_K210_EVB    
    rt_dvp_start(pdata,length);    
    #endif
    if (rt_event_recv(&g_rec_photo_event, RECV_EVENT_FLAG,
                      RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                       rt_tick_from_millisecond(1000*10), RT_NULL) == RT_EOK)
    {
        //LOG_I("receive the ov2640 data successfully.");
        rt_mutex_release(g_ov2640_mutex);
        return RT_EOK;
    }
    else
    {
        LOG_E("receive the ov2640 timeout(10s)");
        rt_mutex_release(g_ov2640_mutex);
        return RT_ERROR;
    }     
    
}



static rt_err_t rt_ov2640_open(rt_device_t dev, rt_uint16_t oflag)
{
    RT_ASSERT(dev != RT_NULL);

    return RT_EOK;
}

static rt_err_t rt_ov2640_close(rt_device_t dev)
{
    RT_ASSERT(dev != RT_NULL);
    
    return RT_EOK;
}

static rt_err_t rt_ov2640_control(rt_device_t dev, int cmd, void *args)
{
    RT_ASSERT(dev != RT_NULL);
    rt_err_t ret = RT_EOK;
    if(cmd < IOCTRL_CAMERA_START_SHOT || cmd > IOCTRL_CAMERA_SET_EXPOSURE)
    {
        LOG_E("CMD value should be 22 ~29");
        return RT_ERROR;
    }    
    int value = 0;
    _ioctl_shoot_para shoot_para = {0};
    
    if(IOCTRL_CAMERA_START_SHOT == cmd)
    {
        shoot_para = *((_ioctl_shoot_para*)args);
        ret = rt_ov2640_start_shoot(shoot_para.pdata,shoot_para.length);
        return ret;
    }
    else
    {
        value = *((int*)args);
        switch(cmd)
        {
        #ifdef SOC_FAMILY_STM32
            case IOCTRL_CAMERA_SET_RESO:
                if(value > 13 || value < 0)
                {
                    LOG_I("value should be 0 ~13");
                    return RT_EINVAL ;
                }
                ov2640_set_resolution(value);
                break;
       #endif
            case IOCTRL_CAMERA_SET_LIGHT:
                if(value > 4 || value < 0)
                {
                    LOG_I("value should be 0 ~13");
                    return RT_EINVAL ;
                }
                ov2640_set_light_mode(value);
                break;
            case IOCTRL_CAMERA_SET_COLOR:
                if(value > 4 || value < 0)
                {
                    LOG_I("value should be 0 ~13");
                    return RT_EINVAL ;
                }
                ov2640_set_color_saturation(value);
                break;
            case IOCTRL_CAMERA_SET_BRIGHTNESS:
                if(value > 4 || value < 0)
                {
                    LOG_I("value should be 0 ~13");
                    return RT_EINVAL ;
                }
                ov2640_set_brightness(value);
                break;
            case IOCTRL_CAMERA_SET_CONTRAST:
                if(value > 4 || value < 0)
                {
                    LOG_I("value should be 0 ~13");
                    return RT_EINVAL ;
                }
                ov2640_set_contrast(value);
                break;
            case IOCTRL_CAMERA_SET_EFFECT:
                if(value > 6 || value < 0)
                {
                    LOG_I("value should be 0 ~13");
                    return RT_EINVAL ;
                }
                ov2640_set_special_effects(value);
                break;
            case IOCTRL_CAMERA_SET_EXPOSURE:
                if(value > 4 || value < 0)
                {
                    LOG_I("value should be 0 ~13");
                    return RT_EINVAL ;
                }
                ov2640_set_auto_exposure(value);
                break;
            default:
                break;
        }        
        return RT_EOK;
    }
}

static rt_size_t rt_ov2640_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    RT_ASSERT(dev != RT_NULL);

    return RT_EOK;
}

static rt_size_t rt_ov2640_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    RT_ASSERT(dev != RT_NULL);
    
    return RT_EOK;
}

static rt_err_t rt_ov2640_init(rt_device_t dev)
{
    RT_ASSERT(dev != RT_NULL);
    rt_uint16_t i = 0;
    int ret = RT_EOK;
    #ifdef SOC_FAMILY_STM32
        #ifdef OV2640_ZDYZ
        //zheng dian yuan zi manufacture ov2640
        rt_stm32407_atk_miniexpolre();
        #else
        //weixue manufacture ov2640
        rt_ov2460_rcc_int();
        #endif
    i2c_bus = rt_i2c_bus_device_find(I2C_NAME);
    if (i2c_bus == RT_NULL)
    {
        LOG_E("can't find %c deivce", I2C_NAME);
        return RT_ERROR;
    }
    #endif
    sccb_write_reg(i2c_bus, OV2640_DSP_RA_DLMT, 0x01);
    sccb_write_reg(i2c_bus, OV2640_SENSOR_COM7, 0x80);
    rt_thread_mdelay(50);
    ov2640_read_id(i2c_bus);
    /*
        set ov2640 register value
    */
    if(g_ov2640_reso_level >LARGE_PHOTO_MODE )//large photo mode
    {
        for (i = 0; i < sizeof(ov2640_sxga_init_reg_tbl) / 2; i++)
        {
            sccb_write_reg(i2c_bus, ov2640_sxga_init_reg_tbl[i][0], ov2640_sxga_init_reg_tbl[i][1]);
        }
    }
    else
    {
        for (i = 0; i < sizeof(ov2640_svga_init_reg_tbl) / 2; i++)
        {
            sccb_write_reg(i2c_bus, ov2640_svga_init_reg_tbl[i][0], ov2640_svga_init_reg_tbl[i][1]);
        }
    }
    ov2640_rgb565_mode();
    ov2640_set_light_mode(0);
    ov2640_set_color_saturation(0);
    ov2640_set_brightness(2);
    ov2640_set_contrast(1);
    #ifdef SOC_FAMILY_STM32
     LOG_I("set ov2640 jpeg mode on stm32 board");
    ov2640_jpeg_mode();
    ov2640_set_image_window_size(0, 0, jpeg_img_size_tbl[g_ov2640_reso_level][0],jpeg_img_size_tbl[g_ov2640_reso_level][1]);	
    ov2640_set_image_out_size(jpeg_img_size_tbl[g_ov2640_reso_level][0], jpeg_img_size_tbl[g_ov2640_reso_level][1]);
    LOG_I("set image resolution is %d * %d ",jpeg_img_size_tbl[g_ov2640_reso_level][0],jpeg_img_size_tbl[g_ov2640_reso_level][1]);
    #elif defined BOARD_K210_EVB
    ov2640_set_image_window_size(0, 0, jpeg_img_size_tbl[5][0],jpeg_img_size_tbl[5][1]);	
    ov2640_set_image_out_size(jpeg_img_size_tbl[2][0], jpeg_img_size_tbl[2][1]);
    LOG_I("set ov2640 rgb565 mode on K210 board and set reselotion QVGA 320,240");
    #endif
    /*
        initialize rt_mutex about the using of ov2640
    */
    g_ov2640_mutex = rt_mutex_create("ov2640", RT_IPC_FLAG_FIFO);
    if (g_ov2640_mutex == RT_NULL)
    {
        LOG_E("create g_ov2640_mutex mutex failed.\n");
        return -RT_ERROR;
    }
    /*
        initialize receive event object
    */
    ret = rt_event_init(&g_rec_photo_event, "r_photo", RT_IPC_FLAG_FIFO);
    if (RT_EOK!= ret)
    {
        LOG_E("init rec_photo_event failed.\n");
        return -RT_ERROR;
    }
    /*
        set dcmi or dvp interrupt callback function
    */
    ret = rt_ov2640_set_irq_hander(rt_ov2640_frame_irq_event);
    if(RT_EOK == ret)
    {
        LOG_I("ov2640 set irq hander successfully.");
    }
    else
    {
        LOG_E("ov2640 set irq hander failed");
    }
    return RT_EOK;
}

#ifdef RT_USING_POSIX
static int ov2640_fops_open(struct dfs_fd *fd)
{
    
    rt_err_t ret = RT_EOK;
    rt_uint16_t flags = 0;
    rt_device_t device;
    device = (rt_device_t)fd->data;
    RT_ASSERT(device != RT_NULL);
    flags = RT_DEVICE_FLAG_RDONLY;
    ret = rt_device_open(device, flags);
    if (ret == RT_EOK) 
    {
        return RT_EOK;
    }
    return RT_ERROR;
}
static int ov2640_fops_close(struct dfs_fd *fd)
{
    rt_device_t device;
    device = (rt_device_t)fd->data;
    rt_device_set_rx_indicate(device, RT_NULL);
    rt_device_close(device);
    return RT_EOK;

}
static int ov2640_fops_ioctl(struct dfs_fd *fd, int cmd, void *args)
{
    rt_device_t device;
    device = (rt_device_t)fd->data;
    return rt_device_control(device, cmd, args);
}
const static struct dfs_file_ops ov2640_fops =
{
    ov2640_fops_open,
    ov2640_fops_close,
    ov2640_fops_ioctl,
    RT_NULL,
    RT_NULL,
    RT_NULL, /* flush */
    RT_NULL, /* lseek */
    RT_NULL, /* getdents */
    RT_NULL,/*poll*/
};
#endif



int camera_ov2640_init()
{
    rt_err_t ret = RT_EOK;
    _ov2640_device.parent.type      = RT_Device_Class_Miscellaneous;
    _ov2640_device.parent.init      = rt_ov2640_init;
    _ov2640_device.parent.open      = rt_ov2640_open;
    _ov2640_device.parent.close     = rt_ov2640_close;
    _ov2640_device.parent.read      = rt_ov2640_read;
    _ov2640_device.parent.write     = rt_ov2640_write;
    _ov2640_device.parent.control   = rt_ov2640_control;
    _ov2640_device.parent.user_data = RT_NULL;
    ret = rt_device_register(&_ov2640_device.parent, "ov2640", RT_DEVICE_FLAG_RDONLY | RT_DEVICE_FLAG_REMOVABLE | RT_DEVICE_FLAG_STANDALONE);
    #ifdef RT_USING_POSIX
    _ov2640_device.parent.fops = &ov2640_fops;
    #endif
    if(ret != RT_EOK)
    {
        LOG_E("ov2640 register fail!!\n\r");
        return -RT_ERROR;
    }
    LOG_I("ov2640 register successfully");
    return RT_EOK;
}
INIT_APP_EXPORT(camera_ov2640_init);

#endif





