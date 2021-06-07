SRC_FILES +=riscv64_switch.c
SRC_FILES +=prepare_rhwstack.c

ifeq ($(CONFIG_TASK_ISOLATION),y)
SRC_FILES += pmp.c
endif

include $(KERNEL_ROOT)/compiler.mk

