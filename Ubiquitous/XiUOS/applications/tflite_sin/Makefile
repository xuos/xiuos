ifeq ($(CONFIG_USING_TFLITE_SIN),y)
	SRC_FILES := \
		sinmain.c \
		main_functions.cc \
		model.cc \
		output_handler.cc \
		constants.cc
	CPPPATHS += -I.
endif

include $(KERNEL_ROOT)/compiler.mk
