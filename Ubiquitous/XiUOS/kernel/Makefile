SRC_DIR :=  thread memory 

ifeq ($(CONFIG_KERNEL_TEST), y)
	SRC_DIR += kernel_test
endif

SRC_DIR +=kernel_service

include $(KERNEL_ROOT)/compiler.mk
