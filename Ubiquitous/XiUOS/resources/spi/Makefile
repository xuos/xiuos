SRC_FILES += dev_spi.c bus_spi.c drv_spi.c

SRC_DIR := third_party_spi

ifeq ($(CONFIG_RESOURCES_SPI_SD), y)
	SRC_DIR += sd_card_spi
endif

ifeq ($(CONFIG_RESOURCES_SPI_SFUD),y)
	SRC_FILES += flash_spi.c
endif

include $(KERNEL_ROOT)/compiler.mk



