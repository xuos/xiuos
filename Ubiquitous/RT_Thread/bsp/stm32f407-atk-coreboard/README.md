# STM32F407最小系统板说明

##  OV2640 menuconfig 配置：

​	More Driver-------->ov2640 driver （勾选） 保存即可



##  RW007 menuconfig 配置：

​	More Driver-------->rw007:SPI WIFI rw007 driver

​																example driver port (rw007 for stm32)---->rw007 for stm32

​																(30000000)  SPI  MAX Hz

​																(spi1) RW007 BUS NAME

​																(7)  CS pin index

​																(19) BOOT0 pin insex (same as spi clk pin)

​																(7)  BOOT1 pin index (same as spi cs pin)

​																(87)  INT/BUSY pin index

​																(88) RESET pin index 

**上述引脚根据电路实际而定,另外涉及到相关的Lwip wifi framwork已经默认配置并匹配**

## 以下为引脚硬件的连接表

## **RW007(SPI1 )**

| 引脚 | 作用      | 引脚序号 | RW007板子 |
| ---- | --------- | -------- | --------- |
| PB3  | SPI1_SCK  | 19       | SCK       |
| PB4  | SPI1_MISO | 20       | MISO      |
| PB5  | SPI1_MOSI | 21       | MOSI      |
| PF6  | CS/BOOT1  | 86       | CS        |
| PF7  | INT/BUSY  | 87       | D9        |
| PF8  | RESET     | 88       | D8        |

PF6 (PA7)

## uart2 串口2PC2   

| PA2  | UART2 TX |
| ---- | -------- |
| PA3  | UART2 RX |



## SPI2 SD卡文件系统

| PC2  | SPI2_MISO |
| ---- | --------- |
| PC3  | SPI2_MOSI |
| PB13 | SPI2_SCK  |
| PB12 | SPI2_CS   |

## OV2460（DCMI接口）

| PD6  | SCCB_SCL   |
| ---- | ---------- |
| PD7  | SCCB_SDA   |
| PB7  | DCMI_VSYNC |
| PA4  | DCMI_HREF  |
| PA6  | DCMI_PCLK  |
| PG15 | DCMI_RESET |
| PG9  | DCMI_PWDN  |
| PA8  | DCMI_XCLK  |
| PE6  | DCMI_D7    |
| PE5  | DCMI_D6    |
| PB6  | DCMI_D5    |
| PC11 | DCMI_D4    |
| PC9  | DCMI_D3    |
| PC8  | DCMI_D2    |
| PC7  | DCMI_D1    |
| PC6  | DCMI_D0    |

## SRAM（FSMC接口）Pin

| PF0  | FSMC_A0   |
| ---- | --------- |
| PF1  | FSMC_A1   |
| PF2  | FSMC_A2   |
| PF3  | FSMC_A3   |
| PF4  | FSMC_A4   |
| PF5  | FSMC_A5   |
| PF12 | FSMC_A6   |
| PF13 | FSMC_A7   |
| PF14 | FSMC_A8   |
| PF15 | FSMC_A9   |
| PG0  | FSMC_A10  |
| PG1  | FSMC_A11  |
| PE7  | FSMC_D4   |
| PE8  | FSMC_D5   |
| PE9  | FSMC_D6   |
| PE10 | FSMC_D7   |
| PE11 | FSMC_D8   |
| PE12 | FSMC_D9   |
| PE13 | FSMC_D10  |
| PE14 | FSMC_D11  |
| PE15 | FSMC_D12  |
| PD8  | FSMC_D13  |
| PD9  | FSMC_D14  |
| PD10 | FSMC_D15  |
| PD11 | FSMC_A16  |
| PD12 | FSMC_A17  |
| PD13 | FSMC_A18  |
| PD14 | FSMC_D0   |
| PD15 | FSMC_D1   |
| PG2  | FSMC_A12  |
| PG3  | FSMC_A13  |
| PG4  | FSMC_A14  |
| PG5  | FSMC_A15  |
| PD0  | FSMC_D2   |
| PD1  | FSMC_D3   |
| PD4  | FSMC_NOE  |
| PD5  | FSMC_NWE  |
| PG10 | FSMC_NE3  |
| PE0  | FSMC_NBL0 |
| PE1  | FSMC_NBL1 |