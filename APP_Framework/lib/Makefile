SRC_DIR := 

ifeq ($(CONFIG_APP_SELECT_NEWLIB),y)
		ifeq ($(CONFIG_SEPARATE_COMPILE),y)
			SRC_DIR += app_newlib
		endif	
endif


include $(KERNEL_ROOT)/compiler.mk
