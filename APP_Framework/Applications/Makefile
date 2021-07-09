SRC_DIR := general_functions

SRC_FILES := main.c framework_init.c

ifeq ($(CONFIG_APPLICATION_SENSOR),y)
	SRC_DIR += sensor_app
endif

ifeq ($(CONFIG_APPLICATION_CONNECTION),y)
	SRC_DIR += connection_app
endif

ifeq ($(CONFIG_APPLICATION_KNOWING),y)
	SRC_DIR += knowing_app
endif

ifeq ($(CONFIG_APPLICATION_CONTROL),y)
	SRC_DIR += control_app
endif

include $(KERNEL_ROOT)/compiler.mk
