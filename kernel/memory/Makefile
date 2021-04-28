SRC_FILES := byte_manage.c 

ifeq ($(CONFIG_KERNEL_MEMBLOCK),y)
	SRC_FILES += gatherblock.c
endif

ifeq ($(CONFIG_TASK_ISOLATION),y)
	SRC_FILES += isolation.c
endif

include $(KERNEL_ROOT)/compiler.mk
