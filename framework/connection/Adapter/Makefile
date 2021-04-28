# ifeq ($(CONFIG_CONNECTION_COMMUNICATION_LORA), y)
# SRC_DIR += lora
# endif

ifeq ($(CONFIG_CONNECTION_COMMUNICATION_ETHERNET), y)
SRC_DIR += ethernet
endif

ifeq ($(CONFIG_CONNECTION_COMMUNICATION_WIFI), y)
SRC_DIR += wifi
endif

ifeq ($(CONFIG_CONNECTION_COMMUNICATION_ZIGBEE), y)
	SRC_DIR += zigbee
endif

# ifeq ($(CONFIG_CONNECTION_COMMUNICATION_NB_IOT), y)
# SRC_DIR += nbiot
# endif

# ifeq ($(CONFIG_CONNECTION_COMMUNICATION_4G), y)
# SRC_DIR += 4G
# endif

# ifeq ($(CONFIG_CONNECTION_COMMUNICATION_BLUETOOTH), y)
# SRC_DIR += bluetooth
# endif

# ifeq ($(CONFIG_CONNECTION_COMMUNICATION_RS485), y)
# SRC_DIR += rs485
# endif

SRC_DIR += src

include $(KERNEL_ROOT)/compiler.mk