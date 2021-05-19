SRC_FILES := 

ifeq ($(CONFIG_USER_TEST_LORA_ADHOC),y)
	SRC_FILES += test_adhoc_lora.c
endif

ifeq ($(CONFIG_USER_TEST_SPI_LORA),y)
	SRC_FILES += test_spi_lora.c
endif

ifeq ($(CONFIG_USER_TEST_SPI_FLASH),y)
	SRC_FILES += test_spi_flash.c
endif

include $(KERNEL_ROOT)/compiler.mk
