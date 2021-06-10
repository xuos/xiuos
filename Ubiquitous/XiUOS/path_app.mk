

export APPPATHS :=-I$(BSP_ROOT) \

APPPATHS +=-I$(KERNEL_ROOT)/../../APP_Framework/Applications/general_functions/linklist \
	-I$(KERNEL_ROOT)/../../APP_Framework/lib/app_newlib/include \
	-I$(KERNEL_ROOT)/../../APP_Framework/Framework/sensor \
	-I$(KERNEL_ROOT)/../../APP_Framework/Framework/transform_layer #


COMPILE_APP:
	@$(eval CPPPATHS=$(APPPATHS))
	@echo $(SRC_APP_DIR)
	@for dir in $(SRC_APP_DIR);do    \
               $(MAKE) -C $$dir;          \
       done
	@cp link.mk build/Makefile
	@$(MAKE) -C build COMPILE_TYPE="_app" TARGET=XiUOS_$(BOARD)_app.elf LINK_FLAGS=APPLFLAGS
	@rm build/Makefile build/make.obj

