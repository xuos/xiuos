import os

# toolchains options
ARCH        ='risc-v'
CPU         ='k210'
CROSS_TOOL  ='gcc'

if  CROSS_TOOL == 'gcc':
    PLATFORM    = 'gcc'
    EXEC_PATH   = r'/opt/xpack-riscv-none-gcc/bin'
else:
    print('Please make sure your toolchains is GNU GCC!')
    exit(0)

if os.getenv('RTT_EXEC_PATH'):
    EXEC_PATH = os.getenv('RTT_EXEC_PATH')

BUILD = 'debug'

if PLATFORM == 'gcc':
    PREFIX  = 'riscv-none-embed-'
    CC      = PREFIX + 'gcc'
    CXX     = PREFIX + 'g++'
    AS      = PREFIX + 'gcc'
    AR      = PREFIX + 'ar'
    LINK    = PREFIX + 'gcc'
    TARGET_EXT = 'elf'
    SIZE    = PREFIX + 'size'
    OBJDUMP = PREFIX + 'objdump'
    OBJCPY  = PREFIX + 'objcopy'

    DEVICE  = ' -mcmodel=medany -march=rv64imafc -mabi=lp64f -fsingle-precision-constant'
    CFLAGS  = DEVICE + ' -fno-common -ffunction-sections -fdata-sections -fstrict-volatile-bitfields'
    AFLAGS  = ' -c' + DEVICE + ' -x assembler-with-cpp'
    LFLAGS  = DEVICE + ' -nostartfiles -Wl,--gc-sections,-Map=rtthread.map,-cref,-u,_start -T link.lds'
    CPATH   = ''
    LPATH   = ''

    if BUILD == 'debug':
        CFLAGS += ' -O0 -ggdb'
        AFLAGS += ' -ggdb'
    else:
        CFLAGS += ' -O2 -Os'

    CXXFLAGS = CFLAGS
    # we use c++ 11, but -std=c++11 don't have 'struct siginfo', need gnu++11
    CXXFLAGS += ' -std=gnu++11'

POST_ACTION = OBJCPY + ' -O binary $TARGET rtthread.bin\n' + SIZE + ' $TARGET \n'
