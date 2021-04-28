# aiit-riscv64-board README

## 1. 简介

| 硬件 | 描述 |
| -- | -- |
|芯片型号| K210 |
|CPU| 双核RV64GC |
|主频| 400MHz |
|片内SRAM| 8MB |
| 外设 | 内嵌AES与SHA256算法加速器 |
| | DVP、JTAG、OTP、FPIOA、GPIO、UART、SPI、RTC、I²S、I²C、WDT、Timer与PWM |

XiUOS板级当前支持使用GPIO、I2C、LCD、RTC、SPI、Timer、UART和watchdog等。

## 2. 编译说明

编译环境：Ubuntu18.04

编译工具链：[riscv-none-embed-gcc](https://github.com/ilg-archived/riscv-none-gcc/releases)

编译步骤：

>1.将编译工具链的路径添加到board/aiit-riscv64-board/config.mk文件当中，例如将gnu-mcu-eclipse-riscv-none-gcc-8.2.0-2.1-20190425-1021-centos64.tgz解压到/opt/下时添加：
```
export CROSS_COMPILE ?=/opt/gnu-mcu-eclipse/riscv-none-gcc/8.2.0-2.1-20190425-1021/bin/riscv-none-embed-
```
>2.在代码根目录下执行以下命令，生成配置文件
```
make BOARD=aiit-riscv64-board menuconfig
```
>3.在menuconfig界面配置需要关闭和开启的功能，按回车键进入下级菜单，按Y键选中需要开启的功能，按N键选中需要关闭的功能，配置结束后选择Exit保存并退出
>4.继续执行以下命令，进行编译
```
make BOARD=aiit-riscv64-board
```
>5.如果编译正确无误，会产生XiUOS_aiit-riscv64-board.elf、XiUOS_aiit-riscv64-board.bin文件。其中XiUOS_aiit-riscv64-board.bin需要烧写到设备中进行运行。
>注：最后可以执行以下命令，清除配置文件和编译生成的文件
```
make BOARD=aiit-riscv64-board distclean
```

## 3. 烧写及执行

将开发板通过串口转USB模块连接到USB接口，开发板VCC悬空，TX连接模块RX，RX连接模块TX，GND连接模块GND，再将开发板BOOT引脚和GND引脚短接，最后将开发板电源上电，然后使用K-Flash工具进行烧写bin文件。

![aiit_riscv64_board](img/aiit_riscv64_board.png)

执行以下命令安装K-Flash工具
```
sudo apt install python3 python3-pip
sudo pip3 install pyserial
sudo pip3 install pyelftools
sudo pip3 install kflash
```
如果最后一步安装kflash出现错误，可以尝试以下命令
```
sudo python -m pip install kflash
sudo python3 -m pip install kflash
sudo pip install kflash
sudo pip2 install kflash
```
代码根目录下执行K-Flash工具烧录，-p为USB端口号，视实际情况而定
```
kflash -t build/XiUOS_aiit-riscv64-board.bin -p /dev/ttyUSB0
```
### 3.1 运行结果

如果编译 & 烧写无误，将开发板BOOT引脚与GND引脚断开后重新上电或将RESET引脚短接一下，将会在串口终端上看到信息打印输出。

![terminal](img/terminal.png)