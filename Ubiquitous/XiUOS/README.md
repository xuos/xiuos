# XiUOS README

[矽璓XiUOS](http://xuos.io/)是一款面向智慧车间的工业物联网操作系统，主要由一个极简的微型实时操作系统内核和其上的工业物联框架构成，通过高效管理工业物联网设备、支撑工业物联应用，在生产车间内实现智能化的“感知环境、联网传输、知悉识别、控制调整”，促进以工业设备和工业控制系统为核心的人、机、物深度互联，帮助提升生产线的数字化和智能化水平。

## 目录结构

| 名称 | 说明 |
| -- | -- |
| application | 应用代码 |
| board | 板级支持包 |
| framework | 应用框架 |
| fs | 文件系统 |
| kernel | 内核源码 |
| resources | 驱动文件 |
| tool | 系统工具 |
| | |

## 硬件支持

目前XiUOS支持ARM和RISC-V两种架构的微处理器:

### ARM

ARM架构系列的开发板有

	aiit-arm32-board	stm32f407-st-discovery		stm32f407zgt6

### RISC-V

RISC-V架构系列的开发板有

	aiit-riscv64-board		hifive1-rev-B		kd233		maix-go

## 开发环境

### 推荐使用：

### 操作系统： [Ubuntu18.04](https://ubuntu.com/download/desktop)

### 开发工具： [VSCode](http://101.36.126.201:8011/vscode_1.55.2-1618307277_amd64.deb)

### 依赖包安装：

```
$ sudo apt-get install build-essential pkg-config
$ sudo apt-get install gcc make libncurses5-dev openssl libssl-dev bison flex libelf-dev autoconf libtool gperf libc6-dev
```

### 编译工具链：

ARM： arm-none-eabi，默认安装到Ubuntu的/usr/bin/arm-none-eabi-，使用如下命令行下载
```shell
$ sudo apt-get install gcc-arm-none-eabi
```

RISC-V: riscv-none-embed-，默认安装到Ubuntu的/opt/，下载源码并解压。[下载网址](https://github.com/ilg-archived/riscv-none-gcc/releases)

```shell
$ tar -zxvf gnu-mcu-eclipse-riscv-none-gcc-8.2.0-2.1-20190425-1021-centos64.tgz -C /opt/
```

### 烧写工具

ARM：ST-LINK

RISC-V：K-FLash

**每种开发板分别对应board目录下的一个文件夹，具体编译及烧录步骤请参见board目录下对应文件夹下的README文件。**
