/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-07-27     thread-liu   first version
 * 2021-1-15      xiaoyu       add set resolution function and the way of reset different manufactures's ov2640 egg:zhendianyuanzi and weixue
 */

#ifndef __DRV_OV2640_H__
#define __DRV_OV2640_H__
#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SOC_FAMILY_STM32
#include<drv_dcmi.h>
#elif defined BOARD_K210_EVB
#include<drv_dvp.h>
#endif


#ifdef SOC_FAMILY_STM32   
#define DEV_ADDRESS      0x30 /* OV2640 address with rt-thread iic  */
#elif defined BOARD_K210_EVB
#define DEV_ADDRESS      0x60 /* OV2640 address */
#endif

#define LARGE_PHOTO_MODE        (8)
#define I2C_NAME        "i2c1"
#define RECV_EVENT_FLAG (1 << 1)        //flag about event control block

#define OV2640_MID              0X7FA2
#define OV2640_PID              0X2642
#define OV2640_ZDYZ             //The macro definition of the manufacturer's product

//DSP register address map table for OV2640 when DSP address (0xFF =0X00) is selected
#define OV2640_DSP_R_BYPASS     0x05
#define OV2640_DSP_Qs           0x44
#define OV2640_DSP_CTRL         0x50
#define OV2640_DSP_HSIZE1       0x51
#define OV2640_DSP_VSIZE1       0x52
#define OV2640_DSP_XOFFL        0x53
#define OV2640_DSP_YOFFL        0x54
#define OV2640_DSP_VHYX         0x55
#define OV2640_DSP_DPRP         0x56
#define OV2640_DSP_TEST         0x57
#define OV2640_DSP_ZMOW         0x5A
#define OV2640_DSP_ZMOH         0x5B
#define OV2640_DSP_ZMHH         0x5C
#define OV2640_DSP_BPADDR       0x7C
#define OV2640_DSP_BPDATA       0x7D
#define OV2640_DSP_CTRL2        0x86
#define OV2640_DSP_CTRL3        0x87
#define OV2640_DSP_SIZEL        0x8C
#define OV2640_DSP_HSIZE2       0xC0
#define OV2640_DSP_VSIZE2       0xC1
#define OV2640_DSP_CTRL0        0xC2
#define OV2640_DSP_CTRL1        0xC3
#define OV2640_DSP_R_DVP_SP     0xD3
#define OV2640_DSP_IMAGE_MODE   0xDA
#define OV2640_DSP_RESET        0xE0
#define OV2640_DSP_MS_SP        0xF0
#define OV2640_DSP_SS_ID        0x7F
#define OV2640_DSP_SS_CTRL      0xF8
#define OV2640_DSP_MC_BIST      0xF9
#define OV2640_DSP_MC_AL        0xFA
#define OV2640_DSP_MC_AH        0xFB
#define OV2640_DSP_MC_D         0xFC
#define OV2640_DSP_P_STATUS     0xFE
#define OV2640_DSP_RA_DLMT      0xFF


//DSP register address map table for OV2640 when DSP address (0xFF =0X01) is selected
#define OV2640_SENSOR_GAIN       0x00
#define OV2640_SENSOR_COM1       0x03
#define OV2640_SENSOR_REG04      0x04
#define OV2640_SENSOR_REG08      0x08
#define OV2640_SENSOR_COM2       0x09
#define OV2640_SENSOR_PIDH       0x0A
#define OV2640_SENSOR_PIDL       0x0B
#define OV2640_SENSOR_COM3       0x0C
#define OV2640_SENSOR_COM4       0x0D
#define OV2640_SENSOR_AEC        0x10
#define OV2640_SENSOR_CLKRC      0x11
#define OV2640_SENSOR_COM7       0x12
#define OV2640_SENSOR_COM8       0x13
#define OV2640_SENSOR_COM9       0x14
#define OV2640_SENSOR_COM10      0x15
#define OV2640_SENSOR_HREFST     0x17
#define OV2640_SENSOR_HREFEND    0x18
#define OV2640_SENSOR_VSTART     0x19
#define OV2640_SENSOR_VEND       0x1A
#define OV2640_SENSOR_MIDH       0x1C
#define OV2640_SENSOR_MIDL       0x1D
#define OV2640_SENSOR_AEW        0x24
#define OV2640_SENSOR_AEB        0x25
#define OV2640_SENSOR_W          0x26
#define OV2640_SENSOR_REG2A      0x2A
#define OV2640_SENSOR_FRARL      0x2B
#define OV2640_SENSOR_ADDVSL     0x2D
#define OV2640_SENSOR_ADDVHS     0x2E
#define OV2640_SENSOR_YAVG       0x2F
#define OV2640_SENSOR_REG32      0x32
#define OV2640_SENSOR_ARCOM2     0x34
#define OV2640_SENSOR_REG45      0x45
#define OV2640_SENSOR_FLL        0x46
#define OV2640_SENSOR_FLH        0x47
#define OV2640_SENSOR_COM19      0x48
#define OV2640_SENSOR_ZOOMS      0x49
#define OV2640_SENSOR_COM22      0x4B
#define OV2640_SENSOR_COM25      0x4E
#define OV2640_SENSOR_BD50       0x4F
#define OV2640_SENSOR_BD60       0x50
#define OV2640_SENSOR_REG5D      0x5D
#define OV2640_SENSOR_REG5E      0x5E
#define OV2640_SENSOR_REG5F      0x5F
#define OV2640_SENSOR_REG60      0x60
#define OV2640_SENSOR_HISTO_LOW  0x61
#define OV2640_SENSOR_HISTO_HIGH 0x62





#define IOCTRL_CAMERA_START_SHOT       (22)     // start shoot
#define IOCTRL_CAMERA_SET_RESO         (23)     //set resolution
#define IOCTRL_CAMERA_SET_LIGHT        (24)     //set light mode
#define IOCTRL_CAMERA_SET_COLOR        (25)     //set color saturation
#define IOCTRL_CAMERA_SET_BRIGHTNESS   (26)     //set color brightness
#define IOCTRL_CAMERA_SET_CONTRAST     (27)     //set contrast
#define IOCTRL_CAMERA_SET_EFFECT       (28)     //set effect
#define IOCTRL_CAMERA_SET_EXPOSURE     (29)     //set auto exposure


struct camera_device
{
    struct rt_device   parent;      /**< RT-Thread device struct */
};

typedef struct 
{
    uint32_t pdata; 
    uint32_t length;
}_ioctl_shoot_para;

extern rt_uint32_t rt_ov2640_calculate_jpeg_len(rt_uint8_t* pdata,rt_uint32_t maxlength);    
extern  rt_err_t rt_ov2640_start_shoot(uint32_t pdata, uint32_t length);


#ifdef __cplusplus
}
#endif

#endif









