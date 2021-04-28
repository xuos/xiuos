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
* @file TestRealtime.c
* @brief support to test realtime function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <xiuos.h>
#include <string.h>
#include <dev_pin.h>

extern unsigned int usleep(unsigned int seconds);
static BusType pin; 

#ifdef ARCH_ARM
#include <hardware_gpio.h>
#define GPIO_C13 7
#define GPIO_C2 17

void PinIrqIsr(void *args)
{
    *(volatile unsigned  int *)0x40020818 = 0x2000;
    
    *(volatile unsigned  int  *)0x4002081a = 0x2000;     
}

int RealtimeIrqTest()
{
    struct PinParam testpin_1;
    struct PinStat testpin_1_stat;
    struct PinParam testpin_2;
    int ret = 0;

	struct BusConfigureInfo configure_info_1;
    struct BusConfigureInfo configure_info_2;
	struct BusBlockWriteParam write_param_1;

    configure_info_1.configure_cmd = OPE_CFG;
    configure_info_1.private_data = (void *)&testpin_1;
    write_param_1.buffer = (void *)&testpin_1_stat;

    configure_info_2.configure_cmd = OPE_CFG;
    configure_info_2.private_data = (void *)&testpin_2;

    KPrintf("%s irq test\n",__func__);
    /* config test pin 1 as output*/
    testpin_1.cmd = GPIO_CONFIG_MODE;
    testpin_1.pin = GPIO_C13;
    testpin_1.mode = GPIO_CFG_OUTPUT;

    ret = BusDrvConfigure(pin->owner_driver, &configure_info_1);
    if (ret != EOK) {
        KPrintf("config testpin_1  %d failed!\n", GPIO_C13);
        return -ERROR;
    }

    /* set test pin 1 as high*/
    testpin_1_stat.pin = GPIO_C13;
    testpin_1_stat.val = GPIO_LOW;
    BusDevWriteData(pin->owner_haldev, &write_param_1);

    /* config test pin 2 as input*/
    testpin_2.cmd = GPIO_CONFIG_MODE;
    testpin_2.pin = GPIO_C2;
    testpin_2.mode = GPIO_CFG_INPUT;

    ret = BusDrvConfigure(pin->owner_driver, &configure_info_2);
    if (ret != EOK) {
        KPrintf("config testpin_2  %d input failed!\n", testpin_2.pin);
        return -ERROR;
    }

    testpin_2.cmd = GPIO_IRQ_REGISTER;
    testpin_2.pin = GPIO_C2;
    testpin_2.irq_set.irq_mode = GPIO_IRQ_EDGE_BOTH;
    testpin_2.irq_set.hdr = PinIrqIsr;
    testpin_2.irq_set.args = NONE;

    ret = BusDrvConfigure(pin->owner_driver, &configure_info_2);
    if (ret != EOK) {
        KPrintf("register testpin_2  %d  irq failed!\n", testpin_2.pin);
        return -ERROR;
    }

    testpin_2.cmd = GPIO_IRQ_ENABLE;
    testpin_2.pin = GPIO_C2;

    ret = BusDrvConfigure(pin->owner_driver, &configure_info_2);
    if (ret != EOK) {
        KPrintf("enable testpin_2  %d  irq failed!\n", testpin_2.pin);
        return -ERROR;
    }
    KPrintf("%s irq test\n",__func__);
    
	return 0;
}

void RealtimeTaskSwitchTest()
{
    struct PinParam testpin_1;
    struct PinStat testpin_1_stat;
    int ret = 0;

	struct BusConfigureInfo configure_info_1;
	struct BusBlockWriteParam write_param_1;

    configure_info_1.configure_cmd = OPE_CFG;
    configure_info_1.private_data = (void *)&testpin_1;
    write_param_1.buffer = (void *)&testpin_1_stat;

    /* config test pin 1 as output*/
    testpin_1.cmd = GPIO_CONFIG_MODE;
    testpin_1.pin = GPIO_C13;
    testpin_1.mode = GPIO_CFG_OUTPUT;

    ret = BusDrvConfigure(pin->owner_driver, &configure_info_1);
    if (ret != EOK) {
        KPrintf("config testpin_1  %d failed!\n", GPIO_C13);
        return ;
    }

    /* set test pin 1 as low*/
    testpin_1_stat.pin = GPIO_C13;
    testpin_1_stat.val = GPIO_LOW;
    BusDevWriteData(pin->owner_haldev, &write_param_1);

    while (RET_TRUE) {
        DelayKTask(1);
    }
}

void GpioSpeedTest()
{
    struct PinParam testpin_1;
    struct PinStat testpin_1_stat;
    struct PinParam testpin_2;
    int ret = 0;

	struct BusConfigureInfo configure_info_1;
    struct BusConfigureInfo configure_info_2;
	struct BusBlockWriteParam write_param_1;

    configure_info_1.configure_cmd = OPE_CFG;
    configure_info_1.private_data = (void *)&testpin_1;
    write_param_1.buffer = (void *)&testpin_1_stat;

    configure_info_2.configure_cmd = OPE_CFG;
    configure_info_2.private_data = (void *)&testpin_2;
    
    /* config test pin 1 as output*/
    testpin_1.cmd = GPIO_CONFIG_MODE;
    testpin_1.pin = GPIO_C13;
    testpin_1.mode = GPIO_CFG_OUTPUT;

    ret = BusDrvConfigure(pin->owner_driver, &configure_info_1);
    if (ret != EOK) {
        KPrintf("config testpin_1  %d failed!\n", GPIO_C13);
        return ;
    }

    testpin_2.cmd = GPIO_CONFIG_MODE;
    testpin_2.pin = GPIO_C2;
    testpin_2.mode = GPIO_CFG_INPUT;
    
    ret = BusDrvConfigure(pin->owner_driver, &configure_info_2);

    /* set test pin 1 as low*/
    testpin_1_stat.pin = GPIO_C13;
    testpin_1_stat.val = GPIO_LOW;
    BusDevWriteData(pin->owner_haldev, &write_param_1);

    while (RET_TRUE) {
        *(volatile unsigned  int *)0x40020818 = 0x2000;

        *(volatile unsigned int*)0x4002081a = 0x2000; 
    }
}

#else

#define GPIO_18 18
#define GPIO_19 19

void PinIrqIsr(void *args)
{
    *(volatile unsigned  int  *)0x3800100c |= 0x5;

    *(volatile unsigned  int  *)0x3800100c &= ~0x5;
}

int RealtimeIrqTest()
{
    struct PinParam testpin_1;
    struct PinStat testpin_1_stat;
    struct PinParam testpin_2;

	struct BusConfigureInfo configure_info_1;
    struct BusConfigureInfo configure_info_2;
	struct BusBlockWriteParam write_param_1;

    configure_info_1.configure_cmd = OPE_CFG;
    configure_info_1.private_data = (void *)&testpin_1;
    write_param_1.buffer = (void *)&testpin_1_stat;

    configure_info_2.configure_cmd = OPE_CFG;
    configure_info_2.private_data = (void *)&testpin_2;

    KPrintf("%s irq test\n",__func__);
    /* config GPIO18 as output and set as low */
    testpin_1.cmd = GPIO_CONFIG_MODE;
    testpin_1.pin = GPIO_18;
    testpin_1.mode = GPIO_CFG_OUTPUT;
    BusDrvConfigure(pin->owner_driver, &configure_info_1);

    testpin_1_stat.pin = GPIO_18;
    testpin_1_stat.val = GPIO_LOW;
    BusDevWriteData(pin->owner_haldev, &write_param_1);

    /* config GPIO18 as input */
    testpin_2.cmd = GPIO_CONFIG_MODE;
    testpin_2.pin = GPIO_19;
    testpin_2.mode = GPIO_CFG_INPUT;
    BusDrvConfigure(pin->owner_driver, &configure_info_2);

    testpin_2.cmd = GPIO_IRQ_REGISTER;
    testpin_2.pin = GPIO_19;
    testpin_2.irq_set.irq_mode = GPIO_IRQ_EDGE_RISING;
    testpin_2.irq_set.hdr = PinIrqIsr;
    testpin_2.irq_set.args = NONE;
    BusDrvConfigure(pin->owner_driver, &configure_info_2);

    testpin_2.cmd = GPIO_IRQ_ENABLE;
    testpin_2.pin = GPIO_19;
    BusDrvConfigure(pin->owner_driver, &configure_info_2);

    return 0;
}

void RealtimeTaskSwitchTest()
{
    struct PinParam testpin_1;
    struct PinStat testpin_1_stat;

	struct BusConfigureInfo configure_info_1;
	struct BusBlockWriteParam write_param_1;

    configure_info_1.configure_cmd = OPE_CFG;
    configure_info_1.private_data = (void *)&testpin_1;
    write_param_1.buffer = (void *)&testpin_1_stat;

    testpin_1.cmd = GPIO_CONFIG_MODE;
    testpin_1.pin = GPIO_18;
    testpin_1.mode = GPIO_CFG_OUTPUT;
    BusDrvConfigure(pin->owner_driver, &configure_info_1);

    testpin_1_stat.pin = GPIO_18;
    testpin_1_stat.val = GPIO_LOW;
    BusDevWriteData(pin->owner_haldev, &write_param_1);

    while (RET_TRUE) {
        DelayKTask(10);
    }
}

void GpioSpeedTest()
{
    struct PinParam testpin_1;
    struct PinStat testpin_1_stat;

	struct BusConfigureInfo configure_info_1;
	struct BusBlockWriteParam write_param_1;

    configure_info_1.configure_cmd = OPE_CFG;
    configure_info_1.private_data = (void *)&testpin_1;
    write_param_1.buffer = (void *)&testpin_1_stat;

    testpin_1.cmd = GPIO_CONFIG_MODE;
    testpin_1.pin = GPIO_18;
    testpin_1.mode = GPIO_CFG_OUTPUT;
    BusDrvConfigure(pin->owner_driver, &configure_info_1);

    testpin_1_stat.pin = GPIO_18;
    testpin_1_stat.val = GPIO_LOW;
    BusDevWriteData(pin->owner_haldev, &write_param_1);

    while (RET_TRUE) {
        *(volatile  unsigned  int  *)0x3800100c |= 0x5;
        *(volatile unsigned  int  *)0x3800100c &= ~0x5;
    }
}
#endif

/********************************************************************/
static void UsageHelp(void) 
{
	KPrintf("TestRealtime.\n");
}

int TestRealtime(int argc, char * argv[])
{
    int ret = 0;
    struct BusConfigureInfo configure_info;

	if (NONE == argv || 0 == strncmp("-h", argv[0], strlen("-h")) || 0 == strncmp("usage", argv[0], strlen("usage"))) {
		UsageHelp();
		return -EINVALED;
	}

	pin = BusFind(PIN_BUS_NAME);
    if (!pin) {
        KPrintf("find %s failed!\n", PIN_BUS_NAME);
        return -ERROR;
    }

    pin->owner_driver = BusFindDriver(pin, PIN_DRIVER_NAME);
    pin->owner_haldev = BusFindDevice(pin, PIN_DEVICE_NAME);

	configure_info.configure_cmd = OPE_INT;
    ret = BusDrvConfigure(pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("initialize %s failed!\n", PIN_BUS_NAME);
        return -ERROR;
    }

	if (0 == strncmp("-irq",argv[0],strlen("-irq")) ) {
		RealtimeIrqTest(); ///< static creat single sem test 
	} 
	
	if (0 == strncmp("-task",argv[0],strlen("-task")) ) {
		RealtimeTaskSwitchTest();
	}

    if (0 == strncmp("-gpio",argv[0],strlen("-gpio")) ) {
		GpioSpeedTest();
	}
 
	return 0;
}
