SRC_DIR :=
SRC_FILES +=  bus.c

ifeq ($(CONFIG_KERNEL_DEVICE),y)
	SRC_FILES += device.c
endif

ifeq ($(CONFIG_RESOURCES_CAN),y)
	SRC_DIR += can
endif

ifeq ($(CONFIG_RESOURCES_I2C),y)
	SRC_DIR += i2c
endif

ifeq ($(CONFIG_RESOURCES_LCD),y)
	SRC_DIR += lcd
endif

ifeq ($(CONFIG_RESOURCES_PIN),y)
	SRC_DIR += pin
endif

ifeq ($(CONFIG_RESOURCES_RTC),y)
	SRC_DIR += rtc
endif

ifeq ($(CONFIG_RESOURCES_SDIO),y)
	SRC_DIR += sdio
endif

ifeq ($(CONFIG_RESOURCES_SERIAL),y)
	SRC_DIR += serial
endif

ifeq ($(CONFIG_RESOURCES_SPI),y)
	SRC_DIR += spi
endif

ifeq ($(CONFIG_RESOURCES_HWTIMER),y)
	SRC_DIR += timer
endif

ifeq ($(CONFIG_RESOURCES_TOUCH),y)
	SRC_DIR += touch
endif

ifeq ($(CONFIG_RESOURCES_USB),y)
	SRC_DIR += usb
endif

ifeq ($(CONFIG_RESOURCES_LWIP),y)
	SRC_DIR += ethernet
endif

ifeq ($(CONFIG_RESOURCES_WDT),y)
	SRC_DIR += watchdog
endif

include $(KERNEL_ROOT)/compiler.mk
