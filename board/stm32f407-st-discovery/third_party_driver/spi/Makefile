SRC_FILES := hardware_spi.c connect_spi.c

ifeq ($(CONFIG_RESOURCES_SPI_SFUD),y)
	SRC_FILES += connect_flash_spi.c
endif

include $(KERNEL_ROOT)/compiler.mk
