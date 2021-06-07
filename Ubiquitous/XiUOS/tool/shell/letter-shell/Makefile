SRC_FILES := shell.c   shell_ext.c shell_port.c cmd.c 

ifeq ($(CONFIG_FS_VFS), y)
SRC_DIR += file_ext
endif

include $(KERNEL_ROOT)/compiler.mk
