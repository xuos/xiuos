SRC_FILES := tick.c  kservicetask.c zombierecycle.c init.c lock.c  idle.c linklist.c isr.c console.c  ktask.c  id.c  CriticalArea.c bitmap.c delay.c double_link.c single_link.c assignstat.c ktask_stat.c

ifeq ($(CONFIG_KERNEL_SEMAPHORE),y)
	SRC_FILES += semaphore.c
endif

ifeq ($(CONFIG_KERNEL_MUTEX),y)
	SRC_FILES += mutex.c
endif

ifeq ($(CONFIG_KERNEL_EVENT),y)
	SRC_FILES += event.c
endif

ifeq ($(CONFIG_KERNEL_MESSAGEQUEUE),y)
	SRC_FILES += msgqueue.c
endif

ifeq ($(CONFIG_KERNEL_SOFTTIMER),y)
	SRC_FILES += softtimer.c
endif

ifeq ($(CONFIG_KERNEL_BANNER),y)
	SRC_FILES += banner.c
endif

ifeq ($(CONFIG_KERNEL_HOOK),y)
	SRC_FILES += hook.c
endif

ifeq ($(CONFIG_KERNEL_QUEUEMANAGE),y)
	SRC_FILES += queue_manager.c

	ifeq ($(CONFIG_KERNEL_WORKQUEUE),y)
		SRC_FILES += workqueue.c
	endif

	ifeq ($(CONFIG_KERNEL_WAITQUEUE),y)
		SRC_FILES += waitqueue.c
	endif

	ifeq ($(CONFIG_KERNEL_DATAQUEUE),y)
		SRC_FILES += data_queue.c
	endif
endif

ifeq ($(CONFIG_KERNEL_CIRCULAR_AREA),y)
	SRC_FILES += circular_area.c
endif

ifeq ($(CONFIG_KERNEL_AVL_TREE),y)
	SRC_FILES += avl_tree.c
endif


ifeq ($(CONFIG_ARCH_SMP),y)
	SRC_FILES += smp_assign.c  
else
	SRC_FILES += assign.c
endif

ifeq ($(CONFIG_SCHED_POLICY_RR),y)
	SRC_FILES += assign_roundrobin.c
endif

ifeq ($(CONFIG_SCHED_POLICY_FIFO),y)
	SRC_FILES += assign_fifo.c
endif

ifeq ($(CONFIG_SCHED_POLICY_RR_REMAINSLICE),y)
	SRC_FILES += assign_roundrobinremain.c
endif

ifeq ($(CONFIG_USER_APPLICATION),y)
	SRC_FILES += appstartup.c
endif

include $(KERNEL_ROOT)/compiler.mk
