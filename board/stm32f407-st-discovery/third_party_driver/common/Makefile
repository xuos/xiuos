SRC_FILES := misc.c  hardware_rcc.c hardware_exti.c \
	hardware_it.c hardware_pwr.c hardware_syscfg.c  \

ifeq ($(CONFIG_BSP_USING_DMA),y)
  SRC_FILES +=  hardware_dma.c
endif

include $(KERNEL_ROOT)/compiler.mk
