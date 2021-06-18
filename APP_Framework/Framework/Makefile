SRC_DIR :=  transform_layer 

ifeq ($(CONFIG_SUPPORT_SENSOR_FRAMEWORK),y)
	SRC_DIR += sensor
endif

ifeq ($(CONFIG_SUPPORT_CONNECTION_FRAMEWORK),y)
	SRC_DIR += connection
endif

ifeq ($(CONFIG_SUPPORT_KNOWING_FRAMEWORK),y)
	SRC_DIR += know
endif

ifeq ($(CONFIG_SUPPORT_CONTROL_FRAMEWORK),y)
	SRC_DIR += control
endif


include $(KERNEL_ROOT)/compiler.mk

