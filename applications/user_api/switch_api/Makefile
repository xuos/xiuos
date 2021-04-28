ifeq ($(CONFIG_SEPARATE_COMPILE),y)
	SRC_FILES :=user_print_info.c user_task.c user_mem.c

	ifeq ($(CONFIG_KERNEL_SEMAPHORE),y)
		SRC_FILES += user_semaphore.c
	endif

	ifeq ($(CONFIG_KERNEL_MUTEX),y)
		SRC_FILES += user_mutex.c
	endif

	ifeq ($(CONFIG_KERNEL_EVENT),y)
		SRC_FILES += user_event.c
	endif

	ifeq ($(CONFIG_KERNEL_MESSAGEQUEUE),y)
		SRC_FILES += user_msg.c
	endif

	ifeq ($(CONFIG_FS_VFS),y)
		SRC_FILES += user_fs.c
	endif
else
	SRC_FILES :=
endif

include $(KERNEL_ROOT)/compiler.mk