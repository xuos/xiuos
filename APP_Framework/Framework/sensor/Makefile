SRC_FILES := sensor.c

ifeq ($(CONFIG_SENSOR_CO2),y)
	SRC_DIR += co2
endif

ifeq ($(CONFIG_SENSOR_PM),y)
	SRC_DIR += pm
endif

ifeq ($(CONFIG_SENSOR_VOICE),y)
	SRC_DIR += voice
endif

ifeq ($(CONFIG_SENSOR_TEMPERATURE),y)
	SRC_DIR += temperature
endif

ifeq ($(CONFIG_SENSOR_HUMIDITY),y)
	SRC_DIR += humidity
endif

include $(KERNEL_ROOT)/compiler.mk
