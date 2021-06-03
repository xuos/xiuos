SRC_DIR_TEMP :=$(SRC_DIR)
SRC_DIR:=

CUR_DIR :=$(shell pwd)

CFLAGS += $(CPPPATHS)
AFLAGS += $(CPPPATHS)
CXXFLAGS += $(CPPPATHS)

CFLAGS += $(DEFINES)
AFLAGS += $(DEFINES)
CXXFLAGS += $(DEFINES)
BUILD_DIR := $(KERNEL_ROOT)/build

 
.PHONY:COMPILER
COMPILER:
	@if [ "${SRC_DIR_TEMP}" != "" ];	then \
		for dir in $(SRC_DIR_TEMP);do    \
               $(MAKE) -C $$dir;          \
    	done; \
	fi
	@echo -n $(OBJS) " " >> $(KERNEL_ROOT)/build/make.obj


################################################
define add_c_file
$(eval COBJ := $(1:%.c=%.o)) \
$(eval COBJ := $(COBJ:$(KERNEL_ROOT)/%=%)) \
$(eval LOCALC := $(addprefix $(BUILD_DIR)/,$(COBJ))) \
$(eval OBJS += $(LOCALC)) \
$(if $(strip $(LOCALC)),$(eval $(LOCALC): $(1)
	@if [ ! -d $$(@D) ]; then mkdir -p $$(@D); fi
	@echo cc $$<
	@echo -n $(dir $(LOCALC)) >>$(KERNEL_ROOT)/build/make.dep
	@$(CROSS_COMPILE)gcc -MM $$(CFLAGS) -c $$< >>$(KERNEL_ROOT)/build/make.dep
	@$(CROSS_COMPILE)gcc $$(CFLAGS) -c $$< -o $$@))
endef

define add_cpp_file
$(eval COBJ := $(1:%.cpp=%.o)) \
$(eval COBJ := $(COBJ:$(KERNEL_ROOT)/%=%)) \
$(eval LOCALCPP := $(addprefix $(BUILD_DIR)/,$(COBJ))) \
$(eval OBJS += $(LOCALCPP)) \
$(if $(strip $(LOCALCPP)),$(eval $(LOCALCPP): $(1)
	@if [ ! -d $$(@D) ]; then mkdir -p $$(@D); fi
	@echo cc $$<
	@echo -n $(dir $(LOCALCPP)) >>$(KERNEL_ROOT)/build/make.dep
	@$(CROSS_COMPILE)g++ -MM $$(CXXFLAGS) -c $$< >>$(KERNEL_ROOT)/build/make.dep
	@$(CROSS_COMPILE)g++ $$(CXXFLAGS) -c $$< -o $$@))
endef

define add_cc_file
$(eval COBJ := $(1:%.cc=%.o)) \
$(eval COBJ := $(COBJ:$(KERNEL_ROOT)/%=%)) \
$(eval LOCALCPP := $(addprefix $(BUILD_DIR)/,$(COBJ))) \
$(eval OBJS += $(LOCALCPP)) \
$(if $(strip $(LOCALCPP)),$(eval $(LOCALCPP): $(1)
	@if [ ! -d $$(@D) ]; then mkdir -p $$(@D); fi
	@echo cc $$<
	@echo -n $(dir $(LOCALCPP)) >>$(KERNEL_ROOT)/build/make.dep
	@$(CROSS_COMPILE)g++ -MM $$(CXXFLAGS) -c $$< >>$(KERNEL_ROOT)/build/make.dep
	@$(CROSS_COMPILE)g++ $$(CXXFLAGS) -c $$< -o $$@))
endef

define add_S_file
$(eval SOBJ := $(1:%.S=%.o)) \
$(eval SOBJ := $(SOBJ:$(KERNEL_ROOT)/%=%)) \
$(eval LOCALS := $(addprefix $(BUILD_DIR)/,$(SOBJ))) \
$(eval OBJS += $(LOCALS)) \
$(if $(strip $(LOCALS)),$(eval $(LOCALS): $(1)
	@if [ ! -d $$(@D) ]; then mkdir -p $$(@D); fi
	@echo cc $$<
	@echo -n $(dir $(LOCALC)) >>$(KERNEL_ROOT)/build/make.dep
	@$(CROSS_COMPILE)gcc -MM $$(CFLAGS) -c $$< >>$(KERNEL_ROOT)/build/make.dep
	@$(CROSS_COMPILE)gcc $$(AFLAGS) -c $$< -o $$@))
endef




SRCS := $(strip $(filter %.c,$(SRC_FILES)))
$(if $(SRCS),$(foreach f,$(SRCS),$(call add_c_file,$(addprefix $(CUR_DIR)/,$(f)))))

SRCS := $(strip $(filter %.cpp,$(SRC_FILES)))
$(if $(SRCS),$(foreach f,$(SRCS),$(call add_cpp_file,$(addprefix $(CUR_DIR)/,$(f)))))

SRCS := $(strip $(filter %.cc,$(SRC_FILES)))
$(if $(SRCS),$(foreach f,$(SRCS),$(call add_cc_file,$(addprefix $(CUR_DIR)/,$(f)))))

SRCS := $(strip $(filter %.S,$(SRC_FILES)))
$(if $(SRCS),$(foreach f,$(SRCS),$(call add_S_file,$(addprefix $(CUR_DIR)/,$(f)))))



COMPILER:$(OBJS) 



-include $(KERNEL_ROOT)/build/make.dep
