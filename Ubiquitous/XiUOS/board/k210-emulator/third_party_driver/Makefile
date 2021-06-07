SRC_FILES := sleep.c

ifeq ($(CONFIG_BSP_USING_DMA),y)
  SRC_DIR += dma
endif

ifeq ($(CONFIG_BSP_USING_GPIO),y)
  SRC_DIR += gpio
endif

ifeq ($(CONFIG_BSP_USING_PLIC),y)
  SRC_DIR += plic
endif



ifeq ($(CONFIG_BSP_USING_SECURITY),y)
  SRC_DIR += security
endif


ifeq ($(CONFIG_BSP_USING_SYSCLOCK),y)
  SRC_DIR += sys_clock
endif

ifeq ($(CONFIG_BSP_USING_HWTIMER),y)
  SRC_DIR += timer
endif

ifeq ($(CONFIG_BSP_USING_UART),y)
  SRC_DIR += uart
endif


include $(KERNEL_ROOT)/compiler.mk
