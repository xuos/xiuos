SRC_DIR := general_functions

SRC_FILES := main.c framework_init.c

ifeq ($(CONFIG_RESOURCES_SDIO), y)
	SRC_FILES += sd_card_mount.c
endif

ifeq ($(CONFIG_RESOURCES_SPI_SD), y)
	SRC_FILES += spi_sd_card_mount.c
endif

ifeq ($(CONFIG_CONNECTION_COMMUNICATION_BOOTSTART_LORA_NET_SAMPLE), y)
	SRC_DIR += connection_demo/adhoc_lora_demo
endif

ifeq ($(CONFIG_CONNECTION_COMMUNICATION_ZIGBEE), y)
	SRC_DIR += connection_demo/zigbee_demo
endif

ifeq ($(CONFIG_CONNECTION_COMMUNICATION_ETHERNET), y)
	SRC_DIR += connection_demo/ethernet_demo
endif

ifeq ($(CONFIG_CONNECTION_COMMUNICATION_WIFI), y)
	SRC_DIR += connection_demo/wifi_demo
endif

ifeq ($(CONFIG_CONNECTION_COMMUNICATION_NB_IOT), y)
	SRC_DIR += connection_demo/nbiot_demo
endif

# ifeq ($(CONFIG_CONNECTION_COMMUNICATION_4G), y)
# 	SRC_DIR += connection_demo/4G_demo
# endif

ifeq ($(CONFIG_CONNECTION_COMMUNICATION_BLUETOOTH), y)
	SRC_DIR += connection_demo/bluetooth_demo
endif

ifeq ($(CONFIG_APPLICATION_SENSOR),y)
	SRC_DIR += sensor_app
endif

include $(KERNEL_ROOT)/compiler.mk
