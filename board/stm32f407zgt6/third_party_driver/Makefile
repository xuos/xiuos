
SRC_DIR := common 

ifeq ($(CONFIG_BSP_USING_GPIO),y)
  SRC_DIR += gpio
endif

ifeq ($(CONFIG_BSP_USING_UART),y)
  SRC_DIR += uart
endif


include $(KERNEL_ROOT)/compiler.mk
