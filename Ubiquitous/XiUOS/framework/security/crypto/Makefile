SRC_FILES :=

ifeq ($(CONFIG_CRYPTO_SM3), y)
	SRC_FILES += sm3/sm3.c sm3/sm3_hmac.c test/sm3_test.c
endif

ifeq ($(CONFIG_CRYPTO_SM4), y)
	SRC_FILES += sm4/sm4_common.c  sm4/sms4_setkey.c  sm4/sms4_enc.c sm4/sm4_enc_mode.c test/sm4_test.c
endif

ifeq ($(CONFIG_CRYPTO_SM9), y)
	SRC_FILES += sm9/bignum.c sm9/ecc.c sm9/qn.c sm9/join.c sm9/sm9_util.c sm9/sm9_para.c sm9/sm9.c test/sm9_test.c
endif

include $(KERNEL_ROOT)/compiler.mk