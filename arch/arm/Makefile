#公共部分
SRC_DIR := shared

ifeq ($(CONFIG_BOARD_CORTEX_M3_EVB),y)
SRC_DIR +=cortex-m3
endif

ifeq ($(CONFIG_BOARD_STM32F407_EVB),y)
SRC_DIR +=cortex-m4
endif

include $(KERNEL_ROOT)/compiler.mk
