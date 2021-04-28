SRC_DIR := 

ifeq ($(CONFIG_POSIX_API),y)
	SRC_DIR += posix_support
endif

ifeq ($(CONFIG_SEPARATE_COMPILE),y)
	SRC_DIR += switch_api
	SRC_DIR += general_functions
endif

include $(KERNEL_ROOT)/compiler.mk