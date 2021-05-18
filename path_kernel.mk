
export KERNELPATHS:=


ifeq ($(BSP_ROOT),$(KERNEL_ROOT)/board/kd233)
KERNELPATHS :=-I$(BSP_ROOT) \
	-I$(BSP_ROOT)/include \
    -I$(BSP_ROOT)/third_party_driver/include \
	-I$(BSP_ROOT)/third_party_driver \
	-I$(BSP_ROOT)/third_party_driver/camera \
	-I$(BSP_ROOT)/third_party_driver/drivers \
	-I$(BSP_ROOT)/third_party_driver/lcd \
	-I$(KERNEL_ROOT)/include #
endif

ifeq ($(BSP_ROOT),$(KERNEL_ROOT)/board/maix-go)
KERNELPATHS :=-I$(BSP_ROOT) \
	-I$(BSP_ROOT)/include \
    -I$(BSP_ROOT)/third_party_driver/include \
	-I$(BSP_ROOT)/third_party_driver \
	-I$(BSP_ROOT)/third_party_driver/camera \
	-I$(BSP_ROOT)/third_party_driver/drivers \
	-I$(BSP_ROOT)/third_party_driver/lcd \
	-I$(KERNEL_ROOT)/include #
endif

ifeq ($(BSP_ROOT),$(KERNEL_ROOT)/board/stm32f407-st-discovery)
KERNELPATHS :=-I$(BSP_ROOT) \
	-I$(KERNEL_ROOT)/arch/arm/cortex-m4 \
	-I$(BSP_ROOT)/third_party_driver \
	-I$(BSP_ROOT)/third_party_driver/sdio \
	-I$(BSP_ROOT)/include \
	-I$(BSP_ROOT)/third_party_driver/include \
	-I$(BSP_ROOT)/third_party_driver/usb/STM32_USB_OTG_Driver/inc \
	-I$(KERNEL_ROOT)/include \
	-I$(BSP_ROOT)/include #
endif

ifeq ($(BSP_ROOT),$(KERNEL_ROOT)/board/aiit-arm32-board)
KERNELPATHS :=-I$(BSP_ROOT) \
	-I$(KERNEL_ROOT)/arch/arm/cortex-m4 \
	-I$(BSP_ROOT)/third_party_driver \
	-I$(BSP_ROOT)/third_party_driver \
	-I$(BSP_ROOT)/include \
	-I$(BSP_ROOT)/third_party_driver/include \
	-I$(BSP_ROOT)/third_party_driver/spi/third_party_spi_lora/inc \
	-I$(BSP_ROOT)/third_party_driver/spi/third_party_spi_lora/src/radio \
	-I$(BSP_ROOT)/third_party_driver/usb/STM32_USB_OTG_Driver/inc \
	-I$(KERNEL_ROOT)/include \
	-I$(BSP_ROOT)/include #
endif



ifeq ($(BSP_ROOT),$(KERNEL_ROOT)/board/stm32f407zgt6)
KERNELPATHS :=-I$(BSP_ROOT) \
	-I$(KERNEL_ROOT)/arch/arm/cortex-m4 \
	-I$(BSP_ROOT)/third_party_driver \
	-I$(BSP_ROOT)/third_party_driver/sdio\
	-I$(BSP_ROOT)/include \
	-I$(BSP_ROOT)/third_party_driver/include \
	-I$(BSP_ROOT)/third_party_driver/usb/STM32_USB_OTG_Driver/inc \
	-I$(KERNEL_ROOT)/include \
	-I$(BSP_ROOT)/include #
endif

ifeq ($(BSP_ROOT),$(KERNEL_ROOT)/board/hifive1-rev-B)
KERNELPATHS :=-I$(BSP_ROOT) \
	-I$(BSP_ROOT)/third_party_driver \
	-I$(BSP_ROOT)/third_party_driver/include \
	-I$(BSP_ROOT)/third_party_driver/include/sifive \
	-I$(BSP_ROOT)/third_party_driver/include/sifive/devices \
	-I$(BSP_ROOT)/third_party_driver/fe300prci \
	-I$(BSP_ROOT)/third_party_driver/plic\
	-I$(KERNEL_ROOT)/include #
endif

ifeq ($(BSP_ROOT),$(KERNEL_ROOT)/board/hifive1-emulator)
KERNELPATHS :=-I$(BSP_ROOT) \
	-I$(BSP_ROOT)/third_party_driver \
	-I$(BSP_ROOT)/third_party_driver/include \
	-I$(BSP_ROOT)/third_party_driver/include/sifive \
	-I$(BSP_ROOT)/third_party_driver/include/sifive/devices \
	-I$(BSP_ROOT)/third_party_driver/fe300prci \
	-I$(BSP_ROOT)/third_party_driver/plic\
	-I$(KERNEL_ROOT)/include #
endif

ifeq ($(BSP_ROOT),$(KERNEL_ROOT)/board/k210-emulator)
KERNELPATHS :=-I$(BSP_ROOT) \
	-I$(BSP_ROOT)/third_party_driver \
	-I$(BSP_ROOT)/include \
	-I$(BSP_ROOT)/third_party_driver/include \
	-I$(BSP_ROOT)/third_party_driver/spi/third_party_spi_lora/inc \
	-I$(BSP_ROOT)/third_party_driver/spi/third_party_spi_lora/src/radio \
	-I$(BSP_ROOT)/third_party_driver/camera \
	-I$(BSP_ROOT)/third_party_driver/drivers \
	-I$(BSP_ROOT)/third_party_driver/lcd \
	-I$(KERNEL_ROOT)/include #
endif

ifeq ($(BSP_ROOT),$(KERNEL_ROOT)/board/aiit-riscv64-board)
KERNELPATHS :=-I$(BSP_ROOT) \
	-I$(BSP_ROOT)/third_party_driver \
	-I$(BSP_ROOT)/include \
	-I$(BSP_ROOT)/third_party_driver/include \
	-I$(BSP_ROOT)/third_party_driver/spi/third_party_spi_lora/inc \
	-I$(BSP_ROOT)/third_party_driver/spi/third_party_spi_lora/src/radio \
	-I$(BSP_ROOT)/third_party_driver/camera \
	-I$(BSP_ROOT)/third_party_driver/drivers \
	-I$(BSP_ROOT)/third_party_driver/lcd \
	-I$(KERNEL_ROOT)/include #
endif

ifeq ($(BSP_ROOT),$(KERNEL_ROOT)/board/cortex-m3-emulator)
KERNELPATHS :=-I$(BSP_ROOT) \
	-I$(KERNEL_ROOT)/arch/arm/cortex-m3 \
	-I$(BSP_ROOT)/third_party_driver \
	-I$(BSP_ROOT)/third_party_driver/Libraries/driverlib \
	-I$(BSP_ROOT)/third_party_driver/Libraries/ \
	-I$(BSP_ROOT)/third_party_driver/Libraries/inc \
	-I$(KERNEL_ROOT)/include \
	-I$(BSP_ROOT)/include #
endif

KERNELPATHS += -I$(KERNEL_ROOT)/arch \
            -I$(KERNEL_ROOT)/arch/risc-v/shared/kernel_service #

ifeq ($(CONFIG_FS_VFS), y)
KERNELPATHS +=-I$(KERNEL_ROOT)/fs/devfs \
	-I$(KERNEL_ROOT)/fs/fatfs \
	-I$(KERNEL_ROOT)/fs/shared/include #
endif

ifeq ($(CONFIG_FS_CH376), y)
KERNELPATHS +=-I$(KERNEL_ROOT)/fs/compatibility_ch376 #
endif

ifeq ($(CONFIG_PERCEPTION_SENSORDEVICE), y)
KERNELPATHS += -I$(KERNEL_ROOT)/framework/perception #
endif

ifeq ($(CONFIG_CONNECTION_ADAPTER), y)
KERNELPATHS += -I$(KERNEL_ROOT)/framework/connection/Adapter/include #
endif

KERNELPATHS += -I$(KERNEL_ROOT)/resources/include #

ifeq ($(CONFIG_RESOURCES_SPI), y)
KERNELPATHS +=-I$(KERNEL_ROOT)/resources/spi # 

ifeq ($(CONFIG_RESOURCES_SPI_SFUD),y)
KERNELPATHS += -I$(KERNEL_ROOT)/resources/spi/third_party_spi/SFUD/sfud/inc # 
endif

endif

ifeq ($(CONFIG_TOOL_SHELL), y)
KERNELPATHS +=-I$(KERNEL_ROOT)/tool/shell/letter-shell \
	-I$(KERNEL_ROOT)/tool/shell/letter-shell/file_ext #
endif


ifeq ($(CONFIG_LIB_NEWLIB),y)
KERNELPATHS += -I$(KERNEL_ROOT)/lib/newlib/include #
endif


ifeq ($(ARCH), risc-v)
KERNELPATHS +=-I$(KERNEL_ROOT)/arch/risc-v/shared
ifeq ($(MCU), k210)
	KERNELPATHS +=-I$(KERNEL_ROOT)/arch/risc-v/k210
endif
ifeq ($(MCU), FE310)
	KERNELPATHS +=-I$(KERNEL_ROOT)/arch/risc-v/fe310
endif
endif


ifeq ($(ARCH), arm)
KERNELPATHS +=-I$(KERNEL_ROOT)/arch/arm/shared \
	-I$(KERNEL_ROOT)/lib/comlibc/common #
endif


KERNELPATHS += -I$(KERNEL_ROOT)/kernel/include #


COMPILE_KERNEL:
	@$(eval CPPPATHS=$(KERNELPATHS))
	@for dir in $(SRC_KERNEL_DIR);do    \
               $(MAKE) -C $$dir;          \
       done
	@cp link.mk build/Makefile
	@$(MAKE) -C build COMPILE_TYPE="_kernel" TARGET=XiaoShanOS_$(BOARD)_kernel.elf LINK_FLAGS=LFLAGS
	@rm build/Makefile build/make.obj

