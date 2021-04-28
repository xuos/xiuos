

export APPPATHS :=-I$(BSP_ROOT) \


APPPATHS +=-I$(KERNEL_ROOT)/applications/user_api/switch_api \
	-I$(KERNEL_ROOT)/applications/user_api/general_functions/linklist \
	-I$(KERNEL_ROOT)/applications/user_api/include #

ifeq ($(CONFIG_CONNECTION_AT), y)
APPPATHS +=-I$(KERNEL_ROOT)/framework/connection/AT/at_device/inc \
	-I$(KERNEL_ROOT)/framework/connection/communication/wifi/esp8266 \
	-I$(KERNEL_ROOT)/framework/connection/communication/4G/air720 \
	-I$(KERNEL_ROOT)/framework/connection/communication/4G/ec20 \
	-I$(KERNEL_ROOT)/framework/connection/communication/4G/ec200x \
	-I$(KERNEL_ROOT)/framework/connection/communication/nbiot/bc26 \
	-I$(KERNEL_ROOT)/framework/connection/communication/nbiot/me3616 #

APPPATHS +=-I$(KERNEL_ROOT)/framework/connection/AT/at_protocol/include \
	-I$(KERNEL_ROOT)/framework/connection/AT/at_protocol/at_socket #

APPPATHS +=-I$(KERNEL_ROOT)/framework/connection/AT/sal_socket/include \
	-I$(KERNEL_ROOT)/framework/connection/AT/sal_socket/include/vfs_net \
	-I$(KERNEL_ROOT)/framework/connection/AT/sal_socket/include/socket \
	-I$(KERNEL_ROOT)/framework/connection/AT/sal_socket/include/socket/sys_socket \
	-I$(KERNEL_ROOT)/framework/connection/AT/sal_socket/impl #

APPPATHS +=-I$(KERNEL_ROOT)/framework/connection/AT/netdev/include #
endif

ifeq ($(CONFIG_CONNECTION_MQTT), y)
APPPATHS +=-I$(KERNEL_ROOT)/framework/connection/MQTT/MQTTClient-C \
	-I$(KERNEL_ROOT)/framework/connection/MQTT/MQTTPacket/src #
endif

ifeq ($(CONFIG_CONNECTION_COMMUNICATION_LORA), y)
APPPATHS +=-I$(KERNEL_ROOT)/framework/connection/Adapter/lora/inc \
	-I$(KERNEL_ROOT)/framework/connection/Adapter/lora/src/radio #
endif

APPPATHS +=-I$(KERNEL_ROOT)/framework/connection/Adapter/include \
           -I$(KERNEL_ROOT)/framework/connection/Adapter/4G \
		   -I$(KERNEL_ROOT)/framework/connection/Adapter/5G \
		   -I$(KERNEL_ROOT)/framework/connection/Adapter/bluetooth \
		   -I$(KERNEL_ROOT)/framework/connection/Adapter/can \
		   -I$(KERNEL_ROOT)/framework/connection/Adapter/ethernet \
		   -I$(KERNEL_ROOT)/framework/connection/Adapter/lora \
		   -I$(KERNEL_ROOT)/framework/connection/Adapter/nbiot \
		   -I$(KERNEL_ROOT)/framework/connection/Adapter/rs485 \
		   -I$(KERNEL_ROOT)/framework/connection/Adapter/wifi \
		   -I$(KERNEL_ROOT)/framework/connection/Adapter/zigbee #

ifeq ($(CONFIG_PERCEPTION_SENSORDEVICE), y)
APPPATHS +=-I$(KERNEL_ROOT)/framework/perception
endif

APPPATHS +=-I$(KERNEL_ROOT)/lib/libcpp #

ifeq ($(CONFIG_INTELLIGENT_TFLITE),y)
	APPPATHS += \
		-I$(KERNEL_ROOT)/framework/intelligent/tflite-mcu/source \
		-I$(KERNEL_ROOT)/framework/intelligent/tflite-mcu/source/third_party/gemmlowp \
		-I$(KERNEL_ROOT)/framework/intelligent/tflite-mcu/source/third_party/flatbuffers/include \
		-I$(KERNEL_ROOT)/framework/intelligent/tflite-mcu/source/third_party/ruy
endif

APPPATHS += -I$(KERNEL_ROOT)/applications/app_newlib/include #


COMPILE_APP:
	@$(eval CPPPATHS=$(APPPATHS))
	@echo $(SRC_APP_DIR)
	@for dir in $(SRC_APP_DIR);do    \
               $(MAKE) -C $$dir;          \
       done
	@cp link.mk build/Makefile
	@$(MAKE) -C build COMPILE_TYPE="_app" TARGET=XiaoShanOS_$(BOARD)_app.elf LINK_FLAGS=APPLFLAGS
	@rm build/Makefile build/make.obj

