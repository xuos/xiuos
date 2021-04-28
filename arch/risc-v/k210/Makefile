SRC_FILES :=  interrupt.c boot.S tick.c

ifeq ($(CONFIG_ARCH_SMP),y)
	SRC_FILES += smp_support.c  
endif

include $(KERNEL_ROOT)/compiler.mk
