SRC_DIR :=
ifeq ($(CONFIG_SUPPORT_KNOW_FRAMEWORK),y)
	ifeq ($(CONFIG_USING_TFLITE_MNIST),y)
		SRC_DIR += tflite_mnist
	endif

	ifeq ($(CONFIG_USING_TFLITE_SIN),y)
		SRC_DIR += tflite_sin
	endif
endif


include $(KERNEL_ROOT)/compiler.mk
