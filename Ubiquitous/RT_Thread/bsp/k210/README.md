# K210最小系统板（Max bit）说明

## OV2640 menuconfig 配置：

​	More Driver-------->ov2640 driver （勾选） 保存即可



##  RW007 menuconfig 配置：

​	More Driver-------->rw007:SPI WIFI rw007 driver

​																 example driver port (not use example driver, porting by myself)  

​																(20000000)  SPI  MAX Hz

​    Board Drivers Config 

​															 Enable SPI1

​																				(27)  spi1 clk pin number                  

​                      														  (28)  spi1 d0 pin number                                                                         

​																			    (26)  spi1 d1 pin number 

​																				SPI1 Enable SS1（spi11 dev）-------->(8)  spi1 ss1 pin number											

​															 (spi11) the SPIDEV rw007 driver on                                                         

​                     										(7) rw007 int pin for rw007                                                                                  

​															 (6) rw007 rst pin for rw007

**SPI1 Enable SS1（spi11 dev）表示SPI1总线片选编号1 ，此时挂载在总线上设备名是spi11，所以 the SPIDEV rw007 driver on参数也要填写（spi11）**

## SD卡配置：

​	 Board Drivers Config-------->Enable SDCARD (spi1(ss0))  （勾选）保存即可		SPI1 Enable SS0（spi10 dev）-------->(29)  spi1 ss1 pin number 会默认配置

**SD卡和RW007共用一条spi硬件总线 ，其中片选设备sd卡为SPI1 Enable SS0（spi10 dev），RW007片选设备为SPI1 Enable SS1（spi11 dev）**



​												

**上述引脚根据电路实际而定,另外涉及到相关的Lwip wifi framwork等已经默认配置并匹配**

## 以下为引脚硬件的连接表

## RW007(SPI1 ) Kendryte Sipeed MAX bit io

| 引脚               | 作用      | 引脚序号 | RW007板子 |
| ------------------ | --------- | -------- | --------- |
| io 27(印丝标注SCK) | SPI1_SCK  |          | SCK       |
| io 26(印丝标注SO)  | SPI1_MISO |          | MISO      |
| io 28(印丝标注SI)  | SPI1_MOSI |          | MOSI      |
| io 8               | CS/BOOT1  |          | CS        |
| io 7               | INT/BUSY  |          | D9        |
| io 6               | RESET     |          | D8        |





## SD卡Kendryte Sipeed MAX bit io

| 引脚               | 作用      | 引脚序号 | RW007板子 |
| ------------------ | --------- | -------- | --------- |
| io 27(印丝标注SCK) | SPI1_SCK  |          | SCK       |
| io 26(印丝标注SO)  | SPI1_MISO |          | MISO      |
| io 28(印丝标注SI)  | SPI1_MOSI |          | MOSI      |
| io 29              | CS/BOOT1  |          | CS        |



**注意：BSP_SPI1_D0_PIN 10  d0也就是MOSI  ，sd卡可直接利用Max bit板载，无需重新接线。SD卡和Rw007设备共用一条SPI1总线**





## 编译说明

编译K210，需要有RT-Thread的代码，因为K210的sdk是以软件包方式，所以需要在bsp/k210下做软件包更新。Windows下推进使用[env工具][1]，然后在console下进入bsp/k210目录中，运行：

    cd bsp/k210
    pkgs --update

如果在Linux平台下，可以先执行

    scons --menuconfig

它会自动下载env相关脚本到~/.env目录，然后执行

    source ~/.env/env.sh
    
    cd bsp/k210
    pkgs --update

下载risc-v的工具链，[下载地址](https://github.com/xpack-dev-tools/riscv-none-embed-gcc-xpack/releases)  
    
更新完软件包后，在`rtconfig.py`中将risc-v工具链的本地路径加入文档。
注：  

1. 工具链建议使用上方提供的，`kendryte的官方工具链`会报浮点类型不兼容的错误，`risc-v工具链8.2.0之前的版本`会出现头文件不兼容的问题。
2. 网上传需要开启C++ 17,认为k210的神经网络编译器nncase多数语法由C++ 17,故需要开启C++ 17。个人认为没有必要，nncase是在PC端独立使用的，
   作用是将神经网络模型转为kmodel格式，此格式文件为已经编译的二进制文件.

然后执行scons编译：  

    set RTT_EXEC_PATH=your_toolchains
    scons

来编译这个板级支持包。如果编译正确无误，会产生rtthread.elf、rtthread.bin文件。其中rtthread.bin需要烧写到设备中进行运行。  
注：如果初次使用编译报错，可能是使用的SDK过老，使用`menuconfig`命令，在→ RT-Thread online packages → peripheral libraries 
and drivers → the kendryte-sdk package for rt-thread中将SDK改为latest版本即可。