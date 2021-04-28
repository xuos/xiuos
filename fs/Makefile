SRC_DIR :=  

ifeq ($(CONFIG_FS_VFS),y)
	SRC_DIR += shared iot-devicefile

	ifeq ($(CONFIG_FS_VFS_FATFS),y)
		SRC_DIR += fatfs
	endif

	ifeq ($(CONFIG_FS_CH376),y)
		SRC_DIR += compatibility_ch376
	endif

	ifeq ($(CONFIG_FS_LWEXT4),y)
		SRC_DIR += lwext4
	endif
endif

include $(KERNEL_ROOT)/compiler.mk
