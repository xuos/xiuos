ifeq ($(CONFIG_POSIX_API),y)
	SRC_FILES += pthread.c

	ifeq ($(CONFIG_KERNEL_SEMAPHORE),y)
		SRC_FILES += semaphore.c
	endif

	ifeq ($(CONFIG_KERNEL_MUTEX),y)
		SRC_FILES += pthread_mutex.c
	endif

	ifeq ($(CONFIG_KERNEL_MESSAGEQUEUE),y)
		SRC_FILES += mqueue.c
	endif
else
	SRC_FILES :=
endif

include $(KERNEL_ROOT)/compiler.mk