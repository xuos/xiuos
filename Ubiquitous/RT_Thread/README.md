# 基于rt-thread的矽璓工业物联操作系统XiUOS

矽璓工业物联操作系统XiUOS主要分为系统层 、框架层、和应用层。其中系统层支持XIUOS、Nuttx、rt-thread三个操作系统，该目录主要内容是基于**rt-thread的系统层。**

## 目录内容

```
xiuos/Ubiquitous/Rt-thread
    ├── README.md    
    ├── bsp         BSP代码
        |──stm32f407-atk-coreboard
        |──k210
    ├── drivers     WiFi、Camera 等相关用户驱动
    └── rt-thread   RT-Thread 代码
```

## 支持平台

Rt-thread/bsp 目前主要支持两个平台：STM32F4 和 Kendryte K210。如果后续用户想自己添加相关bsp在此目录添加即可。

Rt-thread/drivers 目前主要支持 WiFi 和 Camera，其他驱动复用 RT-Thread 的内容。如果rt-thread官方仓库驱动不满足用户使用要求，如果用户增加相关驱动可在此目录。

Rt-thread/rt-thread 使用 RT-Thread 作为系统基础设施，提供底层支持。

## 使用

运行以下指令下载代码、编译运行：

```
# 进入xiuos目录下载更新子模块(包括RT-thread 和 K210 SDK)
git submodule update --init --recursive
# 进入 xiuos/Ubiquitous/Rt-thread/bsp/stm32f407-atk-coreboard 配置 Kconfig
scons --menuconfig
# 编译
scons
# 烧录镜像
st-flash write rtthread.bin 0x8000000
```

同时也可以支持windows开发环境进行上述命令进行编译，需安装env插件，详细介绍可以参照[rt-thread官方资料](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/application-note/setup/standard-project/an0017-standard-project?id=%e4%bd%bf%e7%94%a8-env-%e5%88%9b%e5%bb%ba-rt-thread-%e9%a1%b9%e7%9b%ae%e5%b7%a5%e7%a8%8b
)。


