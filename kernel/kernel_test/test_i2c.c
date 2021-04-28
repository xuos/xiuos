/*
* Copyright (c) 2020 AIIT XUOS Lab
* XiUOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

/**
* @file test_i2c.c
* @brief support to test i2c function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <xiuos.h>
#include <device.h>
#include <bus.h>

/*********************************************************************************************************************************************************/
/*
 * function：I2C device sample support reading temperature and humidity sensor data and printfing on the terminal
 * shell cmd：i2c_HS3000_sample i2c1
 * shell cmd param：i2c device name,if null means default i2c device name
*/

#define    HS_I2C_BUS_NAME                          I2C_BUS_NAME_1   /* I2C bus name */
#define    HS_I2C_DEV_NAME                           I2C_1_DEVICE_NAME_0/* I2C device name */
#define    HS_I2C_DRV_NAME                           I2C_DRV_NAME_1    /* I2C driver name */
#define    ADDR                                                         0x44    /* slave address */

static struct Bus *i2c_bus = NONE;     /* I2C bus handle */

typedef struct Hs300xData
{
    int humi_high;
    int humi_low;
    int temp_high;
    int temp_low;
}Hs300xDataType;

Hs300xDataType g_hs300x_data;

static x_err_t WriteReg(struct HardwareDev *dev)
{
    struct BusBlockWriteParam write_param;

    /* use I2C device API transfer data */
    if(1 == BusDevWriteData(dev, &write_param)) {
        return EOK;
    }
    else{
        return -ERROR; 
    }
}

/* read sensor register data */
static x_err_t ReadRegs(struct HardwareDev *dev, uint8 len, uint8 *buf)
{
    struct BusBlockReadParam read_param;

    read_param.buffer = (void *)buf;
    read_param.size = len;

    /* use I2C device API transfer data */
    if(1 == BusDevReadData(dev, &read_param)){
        return EOK;
    }
    else{
        return -ERROR;
    }
}

static void read_temp_humi(float *cur_temp, float *cur_humi)
{
    uint8 temp[4],ret=0;
    MdelayKTask(15);
    ret  =  WriteReg(i2c_bus->owner_haldev);                            //reset
    if(EOK != ret){
        KPrintf("read_temp_humi WriteReg failed!\n");
    }
    MdelayKTask(50);
    ret = ReadRegs(i2c_bus->owner_haldev, 4, temp);          /* get sensor data */
    if(EOK != ret){
        KPrintf("read_temp_humi ReadRegs failed\n");
    }
 
    *cur_humi = ((temp[0] <<8 | temp[1] )& 0x3fff ) * 100.0 / ( (1 << 14) - 1);                  /* humidity data */

    *cur_temp = ((temp[2]  << 8 | temp[3]) >> 2) * 165.0 /( (1 << 14) - 1) -  40.0;     /* temperature data */
}

static void _HS3000Init(const char *bus_name, const char *dev_name, const char  *drv_name)
{
    /* find I2C device and get I2C handle */
    i2c_bus = BusFind(bus_name);
    if (NONE == i2c_bus){
        KPrintf("_HS3000Init can't find %s bus!\n", bus_name);
    }
    else{
        KPrintf("_HS3000Init find %s bus!\n", bus_name);
    }

    i2c_bus->owner_haldev = BusFindDevice(i2c_bus, dev_name);
    i2c_bus->owner_driver = BusFindDriver(i2c_bus, drv_name);

    if(i2c_bus->match(i2c_bus->owner_driver, i2c_bus->owner_haldev)){
        KPrintf("i2c match drv %s  %p dev %s %p error\n", drv_name, i2c_bus->owner_driver, dev_name, i2c_bus->owner_haldev);
    }
    else{
        KPrintf("_HS3000Init successfully!write %p read %p\n", 
            i2c_bus->owner_haldev->dev_done->write,
            i2c_bus->owner_haldev->dev_done->read);
    }
}

void Hs300xInit(void)
{
    _HS3000Init(HS_I2C_BUS_NAME, HS_I2C_DEV_NAME, HS_I2C_DRV_NAME);        /* init sensor */
}

void Hs300xRead(Hs300xDataType *Hs300xDataType)
{
    float humidity = 0.0, temperature   =  0.0;
    read_temp_humi(&temperature, &humidity);       /* read temperature and humidity sensor data */
    Hs300xDataType->humi_high = (int)humidity;
    Hs300xDataType->humi_low = (int)(humidity*10)%10;
    if( temperature >= 0 ) {
        Hs300xDataType->temp_high = (int)temperature;
        Hs300xDataType->temp_low = (int)(temperature*10)%10;
    }
    else{
        Hs300xDataType->temp_high = (int)temperature;
        Hs300xDataType->temp_low = (int)(-temperature*10)%10;
    }
}

void Tsk_hs300x_test()
{
    memset(&g_hs300x_data, 0, sizeof(Hs300xDataType));
    KPrintf("Tsk create successfully!\n");
    
    while(1)
    {
        Hs300xRead(&g_hs300x_data);

        KPrintf("HS300X:I2C humidity:%d.%d temperature:%d.%d\n", 
            g_hs300x_data.humi_high, 
            g_hs300x_data.humi_low, 
            g_hs300x_data.temp_high, 
            g_hs300x_data.temp_low);

        MdelayKTask(1000);
    }

    return;
}

void Hs300xI2cTest(void)
{
    Hs300xInit();
    MdelayKTask(1000);

    x_err_t flag;
    int32 Tsk_hs300x = KTaskCreate("Tsk_hs300x", Tsk_hs300x_test, NONE, 2048, 10); 
	flag = StartupKTask(Tsk_hs300x);
    if (EOK != flag){
		KPrintf("Hs300xI2cTest StartupKTask failed!\n");
		return;
	}
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0),
                                                Hs300xI2cTest, Hs300xI2cTest, Test the HS300X using I2C);
