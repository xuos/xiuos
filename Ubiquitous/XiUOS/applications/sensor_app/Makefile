SRC_FILES := 

ifeq ($(CONFIG_APPLICATION_SENSOR_CO2_ZG09), y)
	SRC_FILES += co2_zg09.c
endif

ifeq ($(CONFIG_APPLICATION_SENSOR_PM1_0_PS5308), y)
	SRC_FILES += pm1_0_ps5308.c
endif

ifeq ($(CONFIG_APPLICATION_SENSOR_VOICE_D124), y)
	SRC_FILES += voice_d124.c
endif

ifeq ($(CONFIG_APPLICATION_SENSOR_HUMIDITY_HS300X), y)
	SRC_FILES += humidity_hs300x.c
endif

ifeq ($(CONFIG_APPLICATION_SENSOR_TEMPERATURE_HS300X), y)
	SRC_FILES += temperature_hs300x.c
endif

include $(KERNEL_ROOT)/compiler.mk
