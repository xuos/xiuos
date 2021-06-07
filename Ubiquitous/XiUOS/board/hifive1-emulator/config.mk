export CFLAGS := -mcmodel=medany -march=rv32imac -mabi=ilp32 -fno-common -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -O0 -ggdb -fgnu89-inline -Werror
export AFLAGS := -c -mcmodel=medany -march=rv32imac -mabi=ilp32 -x assembler-with-cpp -ggdb
export LFLAGS := -mcmodel=medany -march=rv32imac -mabi=ilp32 -nostartfiles -Wl,--gc-sections,-Map=XiUOS.map,-cref,-u,_start -T $(BSP_ROOT)/link.lds

export APPLFLAGS := -mcmodel=medany -march=rv32imac -mabi=ilp32 -nostartfiles -Wl,--gc-sections,-Map=XiUOS_app.map,-cref,-u, -T $(BSP_ROOT)/link_userspace.lds

export CXXFLAGS := -mcmodel=medany -march=rv32imac -mabi=ilp32 -fno-common -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -O0 -ggdb -Werror

export CROSS_COMPILE ?=/opt/gnu-mcu-eclipse/riscv-none-gcc/8.2.0-2.1-20190425-1021/bin/riscv-none-embed-


export DEFINES := -DHAVE_CCONFIG_H -DHAVE_SIGINFO


export ARCH = risc-v
export MCU =  FE310

# export  HAVE_SIGINFO=1

# export USING_LORA = 1
