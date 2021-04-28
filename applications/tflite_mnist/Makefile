ifeq ($(CONFIG_USING_TFLITE_MNIST),y)
	SRC_FILES := \
		mnistapp.cpp \
		mnistmain.c
	CPPPATHS += -I.
endif

include $(KERNEL_ROOT)/compiler.mk
