SRC_FILES := connect_spi.c hardware_spi.c


ifeq ($(CONFIG_CONNECTION_COMMUNICATION_LORA_USING_SX1278),y)
	SRC_DIR += third_party_spi_lora
	SRC_FILES += connect_lora_spi.c
endif

include $(KERNEL_ROOT)/compiler.mk
