SRC_FILES := system_init.c boot.S interrupt_vector.S coreclock.c interrupt.c svc_entry.S

ifeq ($(CONFIG_TASK_ISOLATION),y)
SRC_FILES += svc_handle.c mpu.c
endif

include $(KERNEL_ROOT)/compiler.mk
