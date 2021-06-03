export CROSS_COMPILE ?=/usr/bin/arm-none-eabi-

export CFLAGS := -mcpu=cortex-m7 -mthumb  -ffunction-sections -fdata-sections -Dgcc -O0 -gdwarf-2 -g -fgnu89-inline -Wa,-mimplicit-it=thumb 
export AFLAGS := -c -mcpu=cortex-m7 -mthumb -ffunction-sections -fdata-sections -x assembler-with-cpp -Wa,-mimplicit-it=thumb  -gdwarf-2
export LFLAGS := -mcpu=cortex-m7 -mthumb -ffunction-sections -fdata-sections -Wl,--gc-sections,-Map=XiUOS_ok1052-c.map,-cref,-u,Reset_Handler -T $(BSP_ROOT)/link.lds
export CXXFLAGS := -mcpu=cortex-m7 -mthumb -ffunction-sections -fdata-sections -Dgcc -O0 -gdwarf-2 -g 

export APPLFLAGS := -mcpu=cortex-m7 -mthumb -ffunction-sections -fdata-sections -Wl,--gc-sections,-Map=XiUOS_app.map,-cref,-u, -T $(BSP_ROOT)/link_userspace.lds


export DEFINES := -DHAVE_CCONFIG_H  -DCPU_MIMXRT1052CVL5B -DSKIP_SYSCLK_INIT -DEVK_MCIMXRM -DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1 -DXIP_EXTERNAL_FLASH=1 -D__STARTUP_INITIALIZE_NONCACHEDATA -D__STARTUP_CLEAR_BSS

export ARCH = arm
export MCU = cortex-m7
