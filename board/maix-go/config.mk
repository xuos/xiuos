export CFLAGS := -mcmodel=medany -march=rv64imafdc -mabi=lp64d -fno-common -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -O0 -ggdb -fgnu89-inline
export AFLAGS := -c -mcmodel=medany -march=rv64imafdc -mabi=lp64d -x assembler-with-cpp -ggdb
export LFLAGS := -mcmodel=medany -march=rv64imafdc -mabi=lp64d -nostartfiles -Wl,--gc-sections,-Map=XiUOS.map,-cref,-u,_start -T $(BSP_ROOT)/link.lds

export APPLFLAGS := -mcmodel=medany -march=rv64imafdc -mabi=lp64d -nostartfiles -Wl,--gc-sections,-Map=XiUOS_app.map,-cref,-u, -T $(BSP_ROOT)/link_userspace.lds

export CXXFLAGS := -mcmodel=medany -march=rv64imafdc -mabi=lp64d -fno-common -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -O0 -ggdb 

export CROSS_COMPILE ?=/opt/gnu-mcu-eclipse/riscv-none-gcc/8.2.0-2.1-20190425-1021/bin/riscv-none-embed-

export DEFINES := -DHAVE_CCONFIG_H -DHAVE_SIGINFO


export ARCH = risc-v
export MCU =  k210

# export  HAVE_SIGINFO=1

# export USING_LORA = 1
