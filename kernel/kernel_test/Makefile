SRC_FILES := test_main.c  
SRC_FILES += extsram_test.c

ifeq ($(CONFIG_KERNEL_TEST_SEM),y)
	SRC_FILES += test_sem.c
endif

ifeq ($(CONFIG_KERNEL_TEST_MUTEX),y)
	SRC_FILES += test_mutex.c
endif

ifeq ($(CONFIG_KERNEL_TEST_EVENT),y)
	SRC_FILES += test_event.c
endif

ifeq ($(CONFIG_KERNEL_TEST_MSG),y)
	SRC_FILES += test_mq.c
endif

ifeq ($(CONFIG_KERNEL_TEST_AVLTREE),y)
	SRC_FILES += test_avltree.c
endif

ifeq ($(CONFIG_KERNEL_TEST_CRICULAR_AREA),y)
	SRC_FILES += test_circulararea.c
endif

ifeq ($(CONFIG_KERNEL_TEST_TOUCH),y)
	SRC_FILES += test_touch.c 
endif

ifeq ($(CONFIG_KERNEL_TEST_MEM),y)
	SRC_FILES += test_mem.c
	SRC_FILES += test_gatherblock.c
endif

ifeq ($(CONFIG_KERNEL_TEST_TIMER),y)
	SRC_FILES += test_timer.c 
endif

ifeq ($(CONFIG_KERNEL_TEST_IWG),y)
	SRC_FILES += test_iwg.c
endif

ifeq ($(CONFIG_KERNEL_TEST_CAN),y)
	SRC_FILES += test_can.c
endif

ifeq ($(CONFIG_KERNEL_TEST_REALTIME),y)
	SRC_FILES += test_realtime.c
endif

ifeq ($(CONFIG_KERNEL_TEST_DBG),y)
	SRC_FILES += test_dbg.c
endif

ifeq ($(CONFIG_KERNEL_TEST_SCHED),y)
	SRC_FILES += test_threadsched.c 
endif

ifeq ($(CONFIG_KERNEL_TEST_RTC),y)
	SRC_FILES += test_rtc.c 
endif

ifeq ($(CONFIG_KERNEL_TEST_SERIAL),y)
	SRC_FILES += test_serial.c 
endif

ifeq ($(CONFIG_KERNEL_TEST_HWTIMER),y)
	SRC_FILES += test_hwtimer.c 
endif

ifeq ($(CONFIG_KERNEL_TEST_LCD),y)
	SRC_FILES += test_lcd.c 
endif

ifeq ($(CONFIG_KERNEL_TEST_CH438),y)
	SRC_FILES += test_ch438.c 
endif

ifeq ($(CONFIG_KERNEL_TEST_I2C),y)
	SRC_FILES += test_i2c.c 
endif

ifeq ($(CONFIG_KERNEL_TEST_RISC_CAN),y)
	SRC_FILES += riscv_test_can.c 
endif

include $(KERNEL_ROOT)/compiler.mk
