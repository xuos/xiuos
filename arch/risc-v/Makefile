SRC_DIR :=  shared

ifeq ($(CONFIG_BOARD_FE310_EVB),y)
SRC_DIR +=fe310
endif

ifeq ($(CONFIG_BOARD_K210_EVB),y)
SRC_DIR +=k210
endif

ifeq ($(CONFIG_BOARD_MAIX_GO_EVB),y)
SRC_DIR +=k210
endif

ifeq ($(CONFIG_BOARD_AIIT_RISCV_EVB),y)
SRC_DIR +=k210
endif

include $(KERNEL_ROOT)/compiler.mk
