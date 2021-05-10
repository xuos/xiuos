MAKEFLAGS += --no-print-directory


.PHONY:all clean distclean show_info  menuconfig
.PHONY:COMPILE_APP COMPILE_KERNEL


support :=kd233 stm32f407-st-discovery maix-go stm32f407zgt6 aiit-riscv64-board aiit-arm32-board hifive1-rev-B hifive1-emulator k210-emulator
SRC_DIR:=

export BOARD ?=kd233

ifeq ($(filter $(BOARD),$(support)),)
$(warning "You should choose board like this:make BOARD=kd233")
$(warning "This is what we support:")
$(warning "$(support)")
$(error "break" )
endif

export TARGET 
export COMPILE_TYPE
export KERNEL_ROOT ?=$(strip $(shell pwd))

MAKEFILES  =$(KERNEL_ROOT)/.config
-include $(KERNEL_ROOT)/.config

export BSP_ROOT ?= $(KERNEL_ROOT)/board/$(BOARD)
include board/$(BOARD)/config.mk
export BSP_BUILD_DIR := board/$(BOARD)
export HOSTTOOLS_DIR ?= $(KERNEL_ROOT)/tool/hosttools
export CONFIG2H_EXE ?= $(HOSTTOOLS_DIR)/xsconfig.sh

export CPPPATHS
export SRC_APP_DIR := applications framework
export SRC_KERNEL_DIR := arch board lib fs kernel resources tool
export SRC_DIR:= $(SRC_APP_DIR) $(SRC_KERNEL_DIR)

PART:=

all:

ifeq ($(CONFIG_COMPILER_APP)_$(CONFIG_COMPILER_KERNEL),y_)
include path_app.mk
PART += COMPILE_APP

else ifeq ($(CONFIG_COMPILER_APP)_$(CONFIG_COMPILER_KERNEL),_y)
include path_kernel.mk
PART += COMPILE_KERNEL

else ifeq ($(CONFIG_COMPILER_APP)_$(CONFIG_COMPILER_KERNEL),y_y)
include path_app.mk
include path_kernel.mk
PART := COMPILE_APP COMPILE_KERNEL

else
include path_kernel.mk
CPPPATHS := $(KERNELPATHS)
PART := COMPILE_ALL
endif



all: $(PART)


COMPILE_ALL:
	@for dir in $(SRC_DIR);do    \
               $(MAKE) -C $$dir;          \
       done
	@cp link.mk build/Makefile
	@$(MAKE) -C build TARGET=XiUOS_$(BOARD).elf LINK_FLAGS=LFLAGS
	@rm build/Makefile build/make.obj



show_info:
	@echo "TARGET is :" $(TARGET)
	@echo "VPATH is :" $(VPATH)
	@echo "BSP_ROOT is :" $(BSP_ROOT)
	@echo "KERNEL_ROOT is :" $(KERNEL_ROOT)
	@echo "CPPPATHS is :" $(CPPPATHS)
	@echo "SRC_DIR is :" $(SRC_DIR)
	@echo "BUILD_DIR is :" $(BUILD_DIR)
	@echo "RTT_ROOT_DIR is :" $(RTT_ROOT_DIR)
	@echo "BSP_BUILD_DIR is :" $(BSP_BUILD_DIR)
	@echo "OBJS is :" $(OBJS)
	@for f in $(CPPPATHS); do \
		echo $$f;                \
	done



menuconfig: 
	@if [  -f "$(BSP_ROOT)/.config" ]; then \
  		cp $(BSP_ROOT)/.config $(KERNEL_ROOT)/.config; \
		else if [  -f "$(BSP_ROOT)/.defconfig" ]; then \
		cp $(BSP_ROOT)/.defconfig $(KERNEL_ROOT)/.config ;\
	fi ;fi
	@kconfig-mconf $(BSP_ROOT)/Kconfig
	@$(CONFIG2H_EXE) .config
	@cp $(KERNEL_ROOT)/.config $(BSP_ROOT)/.config

clean:
	@echo Clean target and build_dir
	@rm -rf build
	@rm -rf temp.txt

distclean:
	@echo Clean all configuration
	@make clean
	@rm -f .config*
	@rm -f $(BSP_ROOT)/.config
