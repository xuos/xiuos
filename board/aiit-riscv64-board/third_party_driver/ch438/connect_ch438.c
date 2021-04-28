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
* @file connect_ch438.c
* @brief support to register ch438 pointer and function
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#include <connect_ch438.h>
#include <drv_io_config.h>
#include <gpiohs.h>
#include <sleep.h>

static uint8 offsetadd[] = {0x00,0x10,0x20,0x30,0x08,0x18,0x28,0x38,};		/* Offset address of serial port number */
static uint8 Interruptnum[8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,};

static BusType ch438_pin;
static int Ch438Sem = NONE;

static plic_irq_callback_t Ch438Irq(void *parameter)
{
	KSemaphoreAbandon(Ch438Sem);
}

static void CH438SetOutput(void)
{
	struct PinParam PinCfg;	
	int ret = 0;

	struct BusConfigureInfo configure_info;
	configure_info.configure_cmd = OPE_CFG;
	configure_info.private_data = (void *)&PinCfg;

    PinCfg.cmd = GPIO_CONFIG_MODE;
    PinCfg.pin = BSP_CH438_D0_PIN;
    PinCfg.mode = GPIO_CFG_OUTPUT;

	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config CH438_D0_PIN pin %d failed!\n", BSP_CH438_D0_PIN);
        return ;
    }

	PinCfg.pin = BSP_CH438_D1_PIN;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config CH438_D1_PIN pin %d failed!\n", BSP_CH438_D1_PIN);
        return ;
    }

	PinCfg.pin = BSP_CH438_D2_PIN;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config CH438_D2_PIN pin %d failed!\n", BSP_CH438_D2_PIN);
        return ;
    }

	PinCfg.pin = BSP_CH438_D3_PIN;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config CH438_D3_PIN pin %d failed!\n", BSP_CH438_D3_PIN);
        return ;
    }

	PinCfg.pin = BSP_CH438_D4_PIN;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config CH438_D4_PIN pin %d failed!\n", BSP_CH438_D4_PIN);
        return ;
    }

	PinCfg.pin = BSP_CH438_D5_PIN;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config CH438_D5_PIN pin %d failed!\n", BSP_CH438_D5_PIN);
        return ;
    }

	PinCfg.pin = BSP_CH438_D6_PIN;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config CH438_D6_PIN pin %d failed!\n", BSP_CH438_D6_PIN);
        return ;
    }

	PinCfg.pin = BSP_CH438_D7_PIN;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config CH438_D7_PIN pin %d failed!\n", BSP_CH438_D7_PIN);
        return ;
    }
}

static void CH438SetInput(void)
{
	struct PinParam PinCfg;	
	int ret = 0;

	struct BusConfigureInfo configure_info;
	configure_info.configure_cmd = OPE_CFG;
	configure_info.private_data = (void *)&PinCfg;

    PinCfg.cmd = GPIO_CONFIG_MODE;
    PinCfg.pin = BSP_CH438_D0_PIN;
    PinCfg.mode = GPIO_CFG_INPUT_PULLUP;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config CH438_D0_PIN pin %d INPUT_PULLUP failed!\n", BSP_CH438_D0_PIN);
        return ;
    }

	PinCfg.pin = BSP_CH438_D1_PIN;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config CH438_D1_PIN pin %d INPUT_PULLUP failed!\n", BSP_CH438_D1_PIN);
        return ;
    }

	PinCfg.pin = BSP_CH438_D2_PIN;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config CH438_D2_PIN pin %d INPUT_PULLUP failed!\n", BSP_CH438_D2_PIN);
        return ;
    }

	PinCfg.pin = BSP_CH438_D3_PIN;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config CH438_D3_PIN pin %d INPUT_PULLUP failed!\n", BSP_CH438_D3_PIN);
        return ;
    }

	PinCfg.pin = BSP_CH438_D4_PIN;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config CH438_D4_PIN pin %d INPUT_PULLUP failed!\n", BSP_CH438_D4_PIN);
        return ;
    }

	PinCfg.pin = BSP_CH438_D5_PIN;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config CH438_D5_PIN pin %d INPUT_PULLUP failed!\n", BSP_CH438_D5_PIN);
        return ;
    }

	PinCfg.pin = BSP_CH438_D6_PIN;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config CH438_D6_PIN pin %d INPUT_PULLUP failed!\n", BSP_CH438_D6_PIN);
        return ;
    }

	PinCfg.pin = BSP_CH438_D7_PIN;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config CH438_D7_PIN pin %d INPUT_PULLUP failed!\n", BSP_CH438_D7_PIN);
        return ;
    }
}

void Set485Input(uint8 ch_no)
{
	struct PinStat pin_stat;
	struct BusBlockWriteParam write_param;
	write_param.buffer = (void *)&pin_stat;
	
	if (ch_no == 1) {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_485_dir;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}
}

void Set485Output(uint8	ch_no)
{
	struct PinStat pin_stat;
	struct BusBlockWriteParam write_param;
	write_param.buffer = (void *)&pin_stat;
	if (ch_no == 1) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_485_dir;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}
}

uint8 ReadCH438Data(uint8 addr )
{
	uint8 dat;
	struct PinStat pin_stat;
	struct BusBlockWriteParam write_param;
	struct BusBlockReadParam read_param;
	write_param.buffer = (void *)&pin_stat;
	read_param.buffer = (void *)&pin_stat;
	
	pin_stat.val = GPIO_HIGH;

    pin_stat.pin = BSP_CH438_NWR_PIN;
    BusDevWriteData(ch438_pin->owner_haldev, &write_param);

	pin_stat.pin = BSP_CH438_NRD_PIN;
    BusDevWriteData(ch438_pin->owner_haldev, &write_param);

	pin_stat.pin = BSP_CH438_ALE_PIN;
    BusDevWriteData(ch438_pin->owner_haldev, &write_param);

	CH438SetOutput();
	usleep(1);

	if (addr &0x80) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D7_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D7_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (addr &0x40) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D6_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D6_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (addr &0x20) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D5_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D5_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (addr &0x10) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D4_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D4_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (addr &0x08) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D3_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D3_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (addr &0x04) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D2_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D2_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (addr &0x02) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D1_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D1_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (addr &0x01) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D0_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D0_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	usleep(1);

	pin_stat.val = GPIO_LOW;
    pin_stat.pin = BSP_CH438_ALE_PIN;
    BusDevWriteData(ch438_pin->owner_haldev, &write_param);

	usleep(1);		

	CH438SetInput();
	usleep(1);

	pin_stat.val = GPIO_LOW;
    pin_stat.pin = BSP_CH438_NRD_PIN;
    BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	
	usleep(1);	
	
	dat = 0;
	pin_stat.pin = BSP_CH438_D7_PIN;
    if (BusDevReadData(ch438_pin->owner_haldev, &read_param)) 
		dat |= 0x80;

	pin_stat.pin = BSP_CH438_D6_PIN;
	if (BusDevReadData(ch438_pin->owner_haldev, &read_param))	
		dat |= 0x40;

	pin_stat.pin = BSP_CH438_D5_PIN;
	if (BusDevReadData(ch438_pin->owner_haldev, &read_param))	
		dat |= 0x20;

	pin_stat.pin = BSP_CH438_D4_PIN;
	if (BusDevReadData(ch438_pin->owner_haldev, &read_param))	
		dat |= 0x10;

	pin_stat.pin = BSP_CH438_D3_PIN;
	if (BusDevReadData(ch438_pin->owner_haldev, &read_param))	
		dat |= 0x08;

	pin_stat.pin = BSP_CH438_D2_PIN;
	if (BusDevReadData(ch438_pin->owner_haldev, &read_param))	
		dat |= 0x04;

	pin_stat.pin = BSP_CH438_D1_PIN;
	if (BusDevReadData(ch438_pin->owner_haldev, &read_param))	
		dat |= 0x02;

	pin_stat.pin = BSP_CH438_D0_PIN;
	if (BusDevReadData(ch438_pin->owner_haldev, &read_param))	
		dat |= 0x01;

	pin_stat.val = GPIO_HIGH;
    pin_stat.pin = BSP_CH438_NRD_PIN;
    BusDevWriteData(ch438_pin->owner_haldev, &write_param);

	pin_stat.val = GPIO_HIGH;
    pin_stat.pin = BSP_CH438_ALE_PIN;
    BusDevWriteData(ch438_pin->owner_haldev, &write_param);

	usleep(1);

	return dat;
}

static void WriteCH438Data(uint8 addr, uint8 dat)
{
	struct PinStat pin_stat;
	struct BusBlockWriteParam write_param;
	write_param.buffer = (void *)&pin_stat;

	pin_stat.val = GPIO_HIGH;
    pin_stat.pin = BSP_CH438_ALE_PIN;
    BusDevWriteData(ch438_pin->owner_haldev, &write_param);

	pin_stat.val = GPIO_HIGH;
    pin_stat.pin = BSP_CH438_NRD_PIN;
    BusDevWriteData(ch438_pin->owner_haldev, &write_param);

	pin_stat.val = GPIO_HIGH;
    pin_stat.pin = BSP_CH438_NWR_PIN;
    BusDevWriteData(ch438_pin->owner_haldev, &write_param);

	CH438SetOutput();
	usleep(1);

	if (addr &0x80) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D7_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D7_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (addr &0x40) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D6_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D6_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (addr &0x20) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D5_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D5_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (addr &0x10) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D4_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D4_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (addr &0x08) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D3_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D3_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (addr &0x04) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D2_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D2_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (addr &0x02) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D1_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D1_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (addr &0x01) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D0_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D0_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}	

	usleep(1);

	pin_stat.val = GPIO_LOW;
    pin_stat.pin = BSP_CH438_ALE_PIN;
    BusDevWriteData(ch438_pin->owner_haldev, &write_param);

	usleep(1);

	if (dat &0x80) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D7_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D7_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (dat &0x40) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D6_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D6_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (dat &0x20) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D5_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D5_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (dat &0x10) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D4_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D4_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (dat &0x08) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D3_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D3_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (dat &0x04) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D2_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D2_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (dat &0x02) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D1_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D1_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	if (dat &0x01) {
		pin_stat.val = GPIO_HIGH;
    	pin_stat.pin = BSP_CH438_D0_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	} else {
		pin_stat.val = GPIO_LOW;
    	pin_stat.pin = BSP_CH438_D0_PIN;
    	BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	}

	usleep(1);

	pin_stat.val = GPIO_LOW;
    pin_stat.pin = BSP_CH438_NWR_PIN;
    BusDevWriteData(ch438_pin->owner_haldev, &write_param);

	usleep(1);

	pin_stat.val = GPIO_HIGH;
    pin_stat.pin = BSP_CH438_NWR_PIN;
    BusDevWriteData(ch438_pin->owner_haldev, &write_param);

	pin_stat.val = GPIO_HIGH;
    pin_stat.pin = BSP_CH438_ALE_PIN;
    BusDevWriteData(ch438_pin->owner_haldev, &write_param);
	
	usleep(1);	

	CH438SetInput();

	return;
}

static void WriteCH438Block( uint8 mAddr, uint8 mLen, uint8 *mBuf )   
{
    while (mLen--) 	
	{
		WriteCH438Data(mAddr, *mBuf++);
	}
}

void CH438UartSend( uint8	ext_uart_no,uint8 *Data, uint8 Num )
{
	uint8	REG_LSR_ADDR,REG_THR_ADDR;
	
	REG_LSR_ADDR = offsetadd[ext_uart_no] | REG_LSR0_ADDR;
	REG_THR_ADDR = offsetadd[ext_uart_no] | REG_THR0_ADDR;
			
   while (1)
   {
       while((ReadCH438Data(REG_LSR_ADDR) & BIT_LSR_TEMT) == 0);

       if (Num <= 128) {
           WriteCH438Block(REG_THR_ADDR, Num, Data);
           break;
       } else {
           WriteCH438Block(REG_THR_ADDR, 128, Data);
           Num -= 128;
           Data += 128;
       }
   }
}

uint8 CH438UARTRcv(uint8 ext_uart_no, uint8 *buf, x_size_t size )
{
    x_size_t rcv_num = 0;
	uint8 dat = 0;
	uint8 REG_LSR_ADDR, REG_RBR_ADDR;
	uint8 *read_buffer;
	x_size_t buffer_index = 0;
	
	REG_LSR_ADDR = offsetadd[ext_uart_no] | REG_LSR0_ADDR;
	REG_RBR_ADDR = offsetadd[ext_uart_no] | REG_RBR0_ADDR;

	read_buffer = buf;
			
	while ((ReadCH438Data(REG_LSR_ADDR) & BIT_LSR_DATARDY) == 0);

    while (((ReadCH438Data(REG_LSR_ADDR) & BIT_LSR_DATARDY) == 0x01) && (size != 0))
    {
		dat =  ReadCH438Data(REG_RBR_ADDR);

		*read_buffer = dat;
		read_buffer++;
		buffer_index++;

		if (BUFFSIZE == buffer_index) {
			buffer_index = 0;
			read_buffer = buf;
		}
			
        rcv_num = rcv_num + 1;
		--size;
    }

    return rcv_num;
}

void CH438_PORT_INIT( uint8 ext_uart_no,uint32	BaudRate )
{
	uint32	div;
	uint8	DLL,DLM,dlab;
	uint8	REG_LCR_ADDR;
	uint8	REG_DLL_ADDR;
	uint8	REG_DLM_ADDR;
	uint8	REG_IER_ADDR;
	uint8	REG_MCR_ADDR;
	uint8	REG_FCR_ADDR;
	uint8	REG_RBR_ADDR;
	uint8	REG_THR_ADDR;
	uint8	REG_IIR_ADDR;
	
	REG_LCR_ADDR = offsetadd[ext_uart_no] | REG_LCR0_ADDR;
	REG_DLL_ADDR = offsetadd[ext_uart_no] | REG_DLL0_ADDR;
	REG_DLM_ADDR = offsetadd[ext_uart_no] | REG_DLM0_ADDR;
	REG_IER_ADDR = offsetadd[ext_uart_no] | REG_IER0_ADDR;
	REG_MCR_ADDR = offsetadd[ext_uart_no] | REG_MCR0_ADDR;
	REG_FCR_ADDR = offsetadd[ext_uart_no] | REG_FCR0_ADDR;
	REG_RBR_ADDR = offsetadd[ext_uart_no] | REG_RBR0_ADDR;
	REG_THR_ADDR = offsetadd[ext_uart_no] | REG_THR0_ADDR;
	REG_IIR_ADDR = offsetadd[ext_uart_no] | REG_IIR0_ADDR;
			
    WriteCH438Data(REG_IER_ADDR, BIT_IER_RESET);             /* Reset the serial port */
	MdelayKTask(50);
	
	dlab = ReadCH438Data(REG_IER_ADDR);
	dlab &= 0xDF;
	WriteCH438Data(REG_IER_ADDR, dlab);
	
	dlab = ReadCH438Data(REG_LCR_ADDR);
	dlab |= 0x80;
	WriteCH438Data(REG_LCR_ADDR, dlab);

    div = (Fpclk >> 4) / BaudRate;
    DLM = div >> 8;
    DLL = div & 0xff;
	WriteCH438Data(REG_DLL_ADDR, DLL);/* Set baud rate */
    WriteCH438Data(REG_DLM_ADDR, DLM);
	WriteCH438Data(REG_FCR_ADDR, BIT_FCR_RECVTG1 | BIT_FCR_RECVTG0 | BIT_FCR_FIFOEN);/* Set FIFO mode */

    WriteCH438Data(REG_LCR_ADDR, BIT_LCR_WORDSZ1 | BIT_LCR_WORDSZ0);

    WriteCH438Data(REG_IER_ADDR, BIT_IER_IERECV);

    WriteCH438Data(REG_MCR_ADDR, BIT_MCR_OUT2);

	WriteCH438Data(REG_FCR_ADDR, ReadCH438Data(REG_FCR_ADDR) | BIT_FCR_TFIFORST);
}

void CH438PortInitParityCheck(uint8 ext_uart_no,uint32	BaudRate)
{
	uint32 div;
	uint8 DLL,DLM,dlab;
	uint8 REG_LCR_ADDR;
	uint8 REG_DLL_ADDR;
	uint8 REG_DLM_ADDR;
	uint8 REG_IER_ADDR;
	uint8 REG_MCR_ADDR;
	uint8 REG_FCR_ADDR;
	uint8 REG_RBR_ADDR;
	uint8 REG_THR_ADDR;
	uint8 REG_IIR_ADDR;
	
	REG_LCR_ADDR = offsetadd[ext_uart_no] | REG_LCR0_ADDR;
	REG_DLL_ADDR = offsetadd[ext_uart_no] | REG_DLL0_ADDR;
	REG_DLM_ADDR = offsetadd[ext_uart_no] | REG_DLM0_ADDR;
	REG_IER_ADDR = offsetadd[ext_uart_no] | REG_IER0_ADDR;
	REG_MCR_ADDR = offsetadd[ext_uart_no] | REG_MCR0_ADDR;
	REG_FCR_ADDR = offsetadd[ext_uart_no] | REG_FCR0_ADDR;
	REG_RBR_ADDR = offsetadd[ext_uart_no] | REG_RBR0_ADDR;
	REG_THR_ADDR = offsetadd[ext_uart_no] | REG_THR0_ADDR;
	REG_IIR_ADDR = offsetadd[ext_uart_no] | REG_IIR0_ADDR;
			
    WriteCH438Data(REG_IER_ADDR, BIT_IER_RESET);/* Reset the serial port */
	MdelayKTask(50);
	
	dlab = ReadCH438Data(REG_IER_ADDR);
	dlab &= 0xDF;
	WriteCH438Data(REG_IER_ADDR, dlab);
	
	dlab = ReadCH438Data(REG_LCR_ADDR);
	dlab |= 0x80;
	WriteCH438Data(REG_LCR_ADDR, dlab);

    div = (Fpclk >> 4) / BaudRate;
    DLM = div >> 8;
    DLL = div & 0xff;
	WriteCH438Data(REG_DLL_ADDR, DLL);/* Set baud rate */
    WriteCH438Data(REG_DLM_ADDR, DLM);
	WriteCH438Data(REG_FCR_ADDR, BIT_FCR_RECVTG1 |  BIT_FCR_FIFOEN);/* Set FIFO mode */

    WriteCH438Data(REG_LCR_ADDR, BIT_LCR_WORDSZ1 | BIT_LCR_WORDSZ0 | BIT_LCR_PAREN | BIT_LCR_PARMODE0);

    WriteCH438Data(REG_IER_ADDR, BIT_IER_IERECV);

    WriteCH438Data(REG_MCR_ADDR, BIT_MCR_OUT2);

	WriteCH438Data(REG_FCR_ADDR, ReadCH438Data(REG_FCR_ADDR) | BIT_FCR_TFIFORST);
}

void CH438_PORT_DISABLE(uint8 ext_uart_no, uint32 BaudRate)
{
	uint32 div;
	uint8 DLL,DLM,dlab;
	uint8 REG_LCR_ADDR;
	uint8 REG_DLL_ADDR;
	uint8 REG_DLM_ADDR;
	uint8 REG_IER_ADDR;
	uint8 REG_MCR_ADDR;
	uint8 REG_FCR_ADDR;
	uint8 REG_RBR_ADDR;
	uint8 REG_THR_ADDR;
	uint8 REG_IIR_ADDR;
	
	REG_LCR_ADDR = offsetadd[ext_uart_no] | REG_LCR0_ADDR;
	REG_DLL_ADDR = offsetadd[ext_uart_no] | REG_DLL0_ADDR;
	REG_DLM_ADDR = offsetadd[ext_uart_no] | REG_DLM0_ADDR;
	REG_IER_ADDR = offsetadd[ext_uart_no] | REG_IER0_ADDR;
	REG_MCR_ADDR = offsetadd[ext_uart_no] | REG_MCR0_ADDR;
	REG_FCR_ADDR = offsetadd[ext_uart_no] | REG_FCR0_ADDR;
	REG_RBR_ADDR = offsetadd[ext_uart_no] | REG_RBR0_ADDR;
	REG_THR_ADDR = offsetadd[ext_uart_no] | REG_THR0_ADDR;
	REG_IIR_ADDR = offsetadd[ext_uart_no] | REG_IIR0_ADDR;
			
    WriteCH438Data(REG_IER_ADDR, BIT_IER_RESET);             /* Reset the serial port */
	MdelayKTask(50);
	
	dlab = ReadCH438Data(REG_IER_ADDR);
	dlab &= 0xDF;
	WriteCH438Data(REG_IER_ADDR, dlab);
	
	dlab = ReadCH438Data(REG_LCR_ADDR);
	dlab |= 0x80;
	WriteCH438Data(REG_LCR_ADDR, dlab);

    div = (Fpclk >> 4) / BaudRate;
    DLM = div >> 8;
    DLL = div & 0xff;
	WriteCH438Data(REG_DLL_ADDR, DLL);/* Set baud rate */
    WriteCH438Data(REG_DLM_ADDR, DLM);
	WriteCH438Data(REG_FCR_ADDR, BIT_FCR_RECVTG1 | BIT_FCR_RECVTG0 | BIT_FCR_FIFOEN );/* Set FIFO mode */

    WriteCH438Data(REG_LCR_ADDR, BIT_LCR_WORDSZ1 | BIT_LCR_WORDSZ0);

    WriteCH438Data(REG_IER_ADDR, 0);

    WriteCH438Data(REG_MCR_ADDR, BIT_MCR_OUT2);

	WriteCH438Data(REG_FCR_ADDR, ReadCH438Data(REG_FCR_ADDR) | BIT_FCR_TFIFORST);
}

void CH438_PORT6_INIT(uint8 ext_uart_no, uint32	BaudRate)
{
	uint32 div;
	uint8 DLL,DLM,dlab;
	uint8 REG_LCR_ADDR;
	uint8 REG_DLL_ADDR;
	uint8 REG_DLM_ADDR;
	uint8 REG_IER_ADDR;
	uint8 REG_MCR_ADDR;
	uint8 REG_FCR_ADDR;
	uint8 REG_RBR_ADDR;
	uint8 REG_THR_ADDR;
	uint8 REG_IIR_ADDR;
	
	REG_LCR_ADDR = offsetadd[ext_uart_no] | REG_LCR0_ADDR;
	REG_DLL_ADDR = offsetadd[ext_uart_no] | REG_DLL0_ADDR;
	REG_DLM_ADDR = offsetadd[ext_uart_no] | REG_DLM0_ADDR;
	REG_IER_ADDR = offsetadd[ext_uart_no] | REG_IER0_ADDR;
	REG_MCR_ADDR = offsetadd[ext_uart_no] | REG_MCR0_ADDR;
	REG_FCR_ADDR = offsetadd[ext_uart_no] | REG_FCR0_ADDR;
	REG_RBR_ADDR = offsetadd[ext_uart_no] | REG_RBR0_ADDR;
	REG_THR_ADDR = offsetadd[ext_uart_no] | REG_THR0_ADDR;
	REG_IIR_ADDR = offsetadd[ext_uart_no] | REG_IIR0_ADDR;
			
    WriteCH438Data(REG_IER_ADDR, BIT_IER_RESET);/* Reset the serial port */
	MdelayKTask(50);
	
	dlab = ReadCH438Data(REG_IER_ADDR);
	dlab &= 0xDF;
	WriteCH438Data(REG_IER_ADDR, dlab);
	
	dlab = ReadCH438Data(REG_LCR_ADDR);
	dlab |= 0x80;
	WriteCH438Data(REG_LCR_ADDR, dlab);

    //div = ( 22118400 >> 4 ) / BaudRate;
	div = ( 44236800 >> 4 ) / BaudRate;
    DLM = div >> 8;
    DLL = div & 0xff;
	WriteCH438Data(REG_DLL_ADDR, DLL);/* Set baud rate */
    WriteCH438Data(REG_DLM_ADDR, DLM);
	WriteCH438Data(REG_FCR_ADDR, BIT_FCR_RECVTG1 | BIT_FCR_RECVTG0 | BIT_FCR_FIFOEN);/* Set FIFO mode */

    WriteCH438Data(REG_LCR_ADDR, BIT_LCR_WORDSZ1 | BIT_LCR_WORDSZ0);

    WriteCH438Data(REG_IER_ADDR, BIT_IER_IERECV | BIT_IER1_CK2X);

    WriteCH438Data(REG_MCR_ADDR, BIT_MCR_OUT2);

	WriteCH438Data(REG_FCR_ADDR, ReadCH438Data(REG_FCR_ADDR) | BIT_FCR_TFIFORST);
}

static uint32 Ch438Configure(struct SerialCfgParam *ext_serial_cfg)
{
	NULL_PARAM_CHECK(ext_serial_cfg);

	switch (ext_serial_cfg->data_cfg.port_configure)
	{
		case PORT_CFG_INIT:
			CH438_PORT_INIT(ext_serial_cfg->data_cfg.ext_uart_no, ext_serial_cfg->data_cfg.serial_baud_rate);
			break;
		case PORT_CFG_PARITY_CHECK:
			CH438PortInitParityCheck(ext_serial_cfg->data_cfg.ext_uart_no, ext_serial_cfg->data_cfg.serial_baud_rate);
			break;
		case PORT_CFG_DISABLE:
			CH438_PORT_DISABLE(ext_serial_cfg->data_cfg.ext_uart_no, ext_serial_cfg->data_cfg.serial_baud_rate);
			break;		
		case PORT_CFG_DIV:
			CH438_PORT6_INIT(ext_serial_cfg->data_cfg.ext_uart_no, ext_serial_cfg->data_cfg.serial_baud_rate);
			break;	
		default:
			KPrintf("Ch438Configure do not support configure %d\n", ext_serial_cfg->data_cfg.port_configure);
			return ERROR;
			break;
	}

	return EOK;
}

static uint32 Ch438Init(struct SerialDriver *serial_drv, struct SerialCfgParam *ext_serial_cfg)
{
	NULL_PARAM_CHECK(serial_drv);

	struct PinParam PinCfg;
	struct PinStat pin_stat;
	int ret = 0;

	struct BusConfigureInfo configure_info;
	configure_info.configure_cmd = OPE_CFG;
	configure_info.private_data = (void *)&PinCfg;

	struct BusBlockWriteParam write_param;
	write_param.buffer = (void *)&pin_stat;

	ch438_pin = PinBusInitGet();

	CH438SetOutput();

	/* config NWR pin as output*/
    PinCfg.cmd = GPIO_CONFIG_MODE;
    PinCfg.pin = BSP_CH438_NWR_PIN;
    PinCfg.mode = GPIO_CFG_OUTPUT;

	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config BSP_CH438_NWR_PIN pin %d failed!\n", BSP_CH438_NWR_PIN);
        return ERROR;
    }

	/* config NRD pin as output*/
	PinCfg.pin = BSP_CH438_NRD_PIN;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config NRD pin %d failed!\n", BSP_CH438_NRD_PIN);
        return ERROR;
    }

	/* config ALE pin as output*/
	PinCfg.pin = BSP_CH438_ALE_PIN;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config ALE pin %d failed!\n", BSP_CH438_ALE_PIN);
        return ERROR;
    }

	/* config ALE pin as output*/
	PinCfg.pin = BSP_485_dir;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config ALE pin %d failed!\n", BSP_485_dir);
        return ERROR;
    }

	/* config ALE pin as input pullup*/
	PinCfg.pin = BSP_CH438_INT_PIN;
	PinCfg.mode = GPIO_CFG_INPUT_PULLUP;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config INIT pin %d failed!\n", BSP_CH438_INT_PIN);
        return ERROR;
    }

	/* set NWR pin as high*/
    pin_stat.pin = BSP_CH438_NWR_PIN;
    pin_stat.val = GPIO_HIGH;
    BusDevWriteData(ch438_pin->owner_haldev, &write_param);

	/* set NRD pin as high*/
    pin_stat.pin = BSP_CH438_NRD_PIN;
    pin_stat.val = GPIO_HIGH;
    BusDevWriteData(ch438_pin->owner_haldev, &write_param);

	/* set ALE pin as high*/
    pin_stat.pin = BSP_CH438_ALE_PIN;
    pin_stat.val = GPIO_HIGH;
    BusDevWriteData(ch438_pin->owner_haldev, &write_param);

	return Ch438Configure(ext_serial_cfg);
}

static uint32 Ch438DrvConfigure(void *drv, struct BusConfigureInfo *configure_info)
{
    NULL_PARAM_CHECK(drv);
    NULL_PARAM_CHECK(configure_info);

    x_err_t ret = EOK;

    struct SerialDriver *serial_drv = (struct SerialDriver *)drv;
	struct SerialCfgParam *ext_serial_cfg = (struct SerialCfgParam *)configure_info->private_data;

    switch (configure_info->configure_cmd)
    {
    case OPE_INT:
        ret = Ch438Init(serial_drv, ext_serial_cfg);
        break;
    default:
        break;
    }

    return ret;
}

static uint32 Ch438WriteData(void *dev, struct BusBlockWriteParam *write_param)
{
	NULL_PARAM_CHECK(dev);
	NULL_PARAM_CHECK(write_param);

	struct SerialHardwareDevice *serial_dev = (struct SerialHardwareDevice *)dev;
	struct SerialDevParam *dev_param = (struct SerialDevParam *)serial_dev->haldev.private_data;

	CH438UartSend(dev_param->ext_uart_no, (uint8 *)write_param->buffer, write_param->size);

	return EOK;
}

static uint32 Ch438ReadData(void *dev, struct BusBlockReadParam *read_param)
{
	NULL_PARAM_CHECK(dev);
	NULL_PARAM_CHECK(read_param);

	x_err_t result;
	uint8 rcv_num = 0;
	uint8 gInterruptStatus;
	uint8 InterruptStatus;
	static uint8 dat;
	uint8 REG_LCR_ADDR;
	uint8 REG_DLL_ADDR;
	uint8 REG_DLM_ADDR;
	uint8 REG_IER_ADDR;
	uint8 REG_MCR_ADDR;
	uint8 REG_FCR_ADDR;
	uint8 REG_RBR_ADDR;
	uint8 REG_THR_ADDR;
	uint8 REG_IIR_ADDR;
	uint8 REG_LSR_ADDR;
	uint8 REG_MSR_ADDR;

	struct SerialHardwareDevice *serial_dev = (struct SerialHardwareDevice *)dev;
	struct SerialDevParam *dev_param = (struct SerialDevParam *)serial_dev->haldev.private_data;
	
	result = KSemaphoreObtain(Ch438Sem, WAITING_FOREVER);
	if (EOK == result) {
   		gInterruptStatus = ReadCH438Data(REG_SSR_ADDR);

		if(!gInterruptStatus) { 
			dat = ReadCH438Data(REG_LCR0_ADDR);
			dat = ReadCH438Data(REG_IER0_ADDR);
			dat = ReadCH438Data(REG_MCR0_ADDR);
			dat = ReadCH438Data(REG_LSR0_ADDR);
			dat = ReadCH438Data(REG_MSR0_ADDR);
			dat = ReadCH438Data(REG_RBR0_ADDR);
			dat = ReadCH438Data(REG_THR0_ADDR);
			dat = ReadCH438Data(REG_IIR0_ADDR);
			dat = dat;
		} else {
			if ( gInterruptStatus & Interruptnum[dev_param->ext_uart_no]) {   /* Detect which serial port is interrupted */ 
				REG_LCR_ADDR = offsetadd[dev_param->ext_uart_no] | REG_LCR0_ADDR;
				REG_DLL_ADDR = offsetadd[dev_param->ext_uart_no] | REG_DLL0_ADDR;
				REG_DLM_ADDR = offsetadd[dev_param->ext_uart_no] | REG_DLM0_ADDR;
				REG_IER_ADDR = offsetadd[dev_param->ext_uart_no] | REG_IER0_ADDR;
				REG_MCR_ADDR = offsetadd[dev_param->ext_uart_no] | REG_MCR0_ADDR;
				REG_FCR_ADDR = offsetadd[dev_param->ext_uart_no] | REG_FCR0_ADDR;
				REG_RBR_ADDR = offsetadd[dev_param->ext_uart_no] | REG_RBR0_ADDR;
				REG_THR_ADDR = offsetadd[dev_param->ext_uart_no] | REG_THR0_ADDR;
				REG_IIR_ADDR = offsetadd[dev_param->ext_uart_no] | REG_IIR0_ADDR;
				REG_LSR_ADDR = offsetadd[dev_param->ext_uart_no] | REG_LSR0_ADDR;
				REG_MSR_ADDR = offsetadd[dev_param->ext_uart_no] | REG_MSR0_ADDR;

				/* The interrupted state of a read serial port */	
				InterruptStatus = ReadCH438Data(REG_IIR_ADDR) & 0x0f;
							
				switch( InterruptStatus )
				{
					case INT_NOINT:	
						break;
					case INT_THR_EMPTY:					
						break;
					case INT_RCV_OVERTIME:	/* Receiving timeout interruption, triggered when no further data is available four data times after receiving a frame*/
					case INT_RCV_SUCCESS:	/* Interrupts are available to receive data */
						rcv_num = CH438UARTRcv(dev_param->ext_uart_no, (uint8 *)read_param->buffer, read_param->size);
						read_param->read_length = rcv_num;
						break;
					case INT_RCV_LINES:		/* Receiving line status interrupted */
						ReadCH438Data(REG_LSR_ADDR);
						break;
					case INT_MODEM_CHANGE:	/* MODEM input changes interrupt */
						ReadCH438Data(REG_MSR_ADDR);
						break;
					default:
						break;
				}
			}
		}
	}
	return rcv_num;
}

static const struct SerialDevDone dev_done = 
{
    NONE,
    NONE,
    Ch438WriteData,
    Ch438ReadData,
};

static void Ch438InitDefault(struct SerialDriver *serial_drv)
{
	struct PinParam PinCfg;
	BusType ch438_pin;

    struct BusConfigureInfo configure_info;

    int ret = 0;
	configure_info.configure_cmd = OPE_CFG;
	configure_info.private_data = (void *)&PinCfg;

	Ch438Sem = KSemaphoreCreate(0);
	if (Ch438Sem < 0) {
		KPrintf("Ch438InitDefault create sem failed .\n");
		return ;
	}

	ch438_pin = PinBusInitGet();

	PinCfg.cmd = GPIO_CONFIG_MODE;
    PinCfg.pin = BSP_CH438_INT_PIN;
    PinCfg.mode = GPIO_CFG_INPUT_PULLUP;
    ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config BSP_CH438_INT_PIN pin %d failed!\n", BSP_CH438_INT_PIN);
        return;
    }

	PinCfg.cmd = GPIO_IRQ_REGISTER;
    PinCfg.pin = BSP_CH438_INT_PIN;
    PinCfg.irq_set.irq_mode = GPIO_IRQ_EDGE_FALLING;
    PinCfg.irq_set.hdr = (void *)Ch438Irq;
    PinCfg.irq_set.args = NONE;
	ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config BSP_CH438_INT_PIN  %d failed!\n", BSP_CH438_INT_PIN);
        return;
    }

	PinCfg.cmd = GPIO_IRQ_ENABLE;
    PinCfg.pin = BSP_CH438_INT_PIN;
    ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);
    if (ret != EOK) {
        KPrintf("config BSP_CH438_INT_PIN  %d failed!\n", BSP_CH438_INT_PIN);
        return;
    }

    struct SerialCfgParam serial_cfg;
    memset(&serial_cfg, 0, sizeof(struct SerialCfgParam));
    configure_info.configure_cmd = OPE_INT;
    configure_info.private_data = (void *)&serial_cfg;

    serial_cfg.data_cfg.port_configure = PORT_CFG_INIT;

    serial_cfg.data_cfg.ext_uart_no = 0;
    serial_cfg.data_cfg.serial_baud_rate = BAUD_RATE_115200;
    BusDrvConfigure(&serial_drv->driver, &configure_info);

    serial_cfg.data_cfg.ext_uart_no = 1;
    serial_cfg.data_cfg.serial_baud_rate = BAUD_RATE_9600;
    BusDrvConfigure(&serial_drv->driver, &configure_info);

    serial_cfg.data_cfg.ext_uart_no = 2;
    serial_cfg.data_cfg.serial_baud_rate = BAUD_RATE_9600;
    BusDrvConfigure(&serial_drv->driver, &configure_info);

    serial_cfg.data_cfg.ext_uart_no = 3;
    serial_cfg.data_cfg.serial_baud_rate = BAUD_RATE_9600;
    BusDrvConfigure(&serial_drv->driver, &configure_info);

    serial_cfg.data_cfg.ext_uart_no = 4;
    serial_cfg.data_cfg.serial_baud_rate = BAUD_RATE_9600;
    BusDrvConfigure(&serial_drv->driver, &configure_info);

    serial_cfg.data_cfg.ext_uart_no = 5;
    serial_cfg.data_cfg.serial_baud_rate = BAUD_RATE_115200;
    BusDrvConfigure(&serial_drv->driver, &configure_info);

    serial_cfg.data_cfg.ext_uart_no = 6;
    serial_cfg.data_cfg.serial_baud_rate = BAUD_RATE_57600;
    BusDrvConfigure(&serial_drv->driver, &configure_info);

    serial_cfg.data_cfg.ext_uart_no = 7;
    serial_cfg.data_cfg.serial_baud_rate = BAUD_RATE_9600;
    BusDrvConfigure(&serial_drv->driver, &configure_info);

	Set485Input(1);
}

static uint32 Ch438DevRegister(struct SerialHardwareDevice *serial_dev, char *dev_name)
{
	x_err_t ret = EOK;
	serial_dev->dev_done = &dev_done;
	serial_dev->ext_serial_mode = RET_TRUE;
    ret = SerialDeviceRegister(serial_dev, NONE, dev_name);
    if (ret != EOK) {
        KPrintf("hw_ch438_init Serial device %s register error %d\n", dev_name, ret);
        return ERROR;
    }

    ret = SerialDeviceAttachToBus(dev_name, CH438_BUS_NAME);
    if (ret != EOK) {
        KPrintf("hw_ch438_init Serial device %s register error %d\n", dev_name, ret);
        return ERROR;
    }

	return ret;
}

int hw_ch438_init(void)
{
    static struct SerialBus serial_bus;
    static struct SerialDriver serial_drv;

    x_err_t ret = EOK;
    
    ret = SerialBusInit(&serial_bus, CH438_BUS_NAME);
    if (ret != EOK) {
        KPrintf("hw_ch438_init Serial bus init error %d\n", ret);
        return ERROR;
    }

    serial_drv.configure = &Ch438DrvConfigure;
    ret = SerialDriverInit(&serial_drv, CH438_DRIVER_NAME);
    if (ret != EOK) {
        KPrintf("hw_ch438_init Serial driver init error %d\n", ret);
        return ERROR;
    }

    ret = SerialDriverAttachToBus(CH438_DRIVER_NAME, CH438_BUS_NAME);
    if (ret != EOK) {
        KPrintf("hw_ch438_init Serial driver attach error %d\n", ret);
        return ERROR;
    }

	static struct SerialHardwareDevice serial_dev_0;
	static struct SerialDevParam dev_param_0;
	dev_param_0.ext_uart_no = 0;
	serial_dev_0.haldev.private_data = (void *)&dev_param_0;
	ret = Ch438DevRegister(&serial_dev_0, CH438_DEVICE_NAME_0);
    if (ret != EOK) {
        KPrintf("hw_ch438_init Ch438DevRegister error %d\n", ret);
        return ERROR;
    }

	static struct SerialHardwareDevice serial_dev_1;
	static struct SerialDevParam dev_param_1;
	dev_param_1.ext_uart_no = 1;
	serial_dev_1.haldev.private_data = (void *)&dev_param_1;
	ret = Ch438DevRegister(&serial_dev_1, CH438_DEVICE_NAME_1);
    if (ret != EOK) {
        KPrintf("hw_ch438_init Ch438DevRegister error %d\n", ret);
        return ERROR;
    }

	static struct SerialHardwareDevice serial_dev_2;
	static struct SerialDevParam dev_param_2;
	dev_param_2.ext_uart_no = 2;
	serial_dev_2.haldev.private_data = (void *)&dev_param_2;
	ret = Ch438DevRegister(&serial_dev_2, CH438_DEVICE_NAME_2);
    if (ret != EOK) {
        KPrintf("hw_ch438_init Ch438DevRegister error %d\n", ret);
        return ERROR;
    }

	static struct SerialHardwareDevice serial_dev_3;
	static struct SerialDevParam dev_param_3;
	dev_param_3.ext_uart_no = 3;
	serial_dev_3.haldev.private_data = (void *)&dev_param_3;
	ret = Ch438DevRegister(&serial_dev_3, CH438_DEVICE_NAME_3);
    if (ret != EOK) {
        KPrintf("hw_ch438_init Ch438DevRegister error %d\n", ret);
        return ERROR;
    }

	static struct SerialHardwareDevice serial_dev_4;
	static struct SerialDevParam dev_param_4;
	dev_param_4.ext_uart_no = 4;
	serial_dev_4.haldev.private_data = (void *)&dev_param_4;
	ret = Ch438DevRegister(&serial_dev_4, CH438_DEVICE_NAME_4);
    if (ret != EOK) {
        KPrintf("hw_ch438_init Ch438DevRegister error %d\n", ret);
        return ERROR;
    }

	static struct SerialHardwareDevice serial_dev_5;
	static struct SerialDevParam dev_param_5;
	dev_param_5.ext_uart_no = 5;
	serial_dev_5.haldev.private_data = (void *)&dev_param_5;
	ret = Ch438DevRegister(&serial_dev_5, CH438_DEVICE_NAME_5);
    if (ret != EOK) {
        KPrintf("hw_ch438_init Ch438DevRegister error %d\n", ret);
        return ERROR;
    }

	static struct SerialHardwareDevice serial_dev_6;
	static struct SerialDevParam dev_param_6;
	dev_param_6.ext_uart_no = 6;
	serial_dev_6.private_data = (void *)&dev_param_6;
	ret = Ch438DevRegister(&serial_dev_6, CH438_DEVICE_NAME_6);
    if (ret != EOK) {
        KPrintf("hw_ch438_init Ch438DevRegister error %d\n", ret);
        return ERROR;
    }

	static struct SerialHardwareDevice serial_dev_7;
	static struct SerialDevParam dev_param_7;
	dev_param_7.ext_uart_no = 7;
	serial_dev_7.private_data = (void *)&dev_param_7;
	ret = Ch438DevRegister(&serial_dev_7, CH438_DEVICE_NAME_7);
    if (ret != EOK) {
        KPrintf("hw_ch438_init Ch438DevRegister error %d\n", ret);
        return ERROR;
    }

	Ch438InitDefault(&serial_drv);

    return ret;
}
