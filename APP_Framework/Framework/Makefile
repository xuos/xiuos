SRC_DIR :=  transform_layer 

ifeq ($(CONFIG_SUPPORT_SENSOR_FRAMEWORK),y)
	SRC_DIR += sensor
endif

ifeq ($(CONFIG_SUPPORT_CONNECTION_FRAMEWORK),y)
	SRC_DIR += connection
endif

include $(KERNEL_ROOT)/compiler.mk

