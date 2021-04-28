ifeq ($(CONFIG_PERCEPTION_SENSORDEVICE),y)
	SRC_FILES := sensor.c
endif

ifeq ($(CONFIG_PERCEPTION_CO2),y)
	SRC_DIR += co2
endif

ifeq ($(CONFIG_PERCEPTION_PM),y)
	SRC_DIR += pm
endif

ifeq ($(CONFIG_PERCEPTION_VOICE),y)
	SRC_DIR += voice
endif

ifeq ($(CONFIG_PERCEPTION_TEMPERATURE),y)
	SRC_DIR += temperature
endif

ifeq ($(CONFIG_PERCEPTION_HUMIDITY),y)
	SRC_DIR += humidity
endif

include $(KERNEL_ROOT)/compiler.mk
