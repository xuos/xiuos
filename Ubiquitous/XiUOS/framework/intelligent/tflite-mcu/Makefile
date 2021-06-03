TFLITE_SRCS = \
	source/tensorflow/lite/micro/all_ops_resolver.cc \
	source/tensorflow/lite/micro/debug_log.cc \
	source/tensorflow/lite/micro/memory_helpers.cc \
	source/tensorflow/lite/micro/micro_allocator.cc \
	source/tensorflow/lite/micro/micro_error_reporter.cc \
	source/tensorflow/lite/micro/micro_interpreter.cc \
	source/tensorflow/lite/micro/micro_profiler.cc \
	source/tensorflow/lite/micro/micro_string.cc \
	source/tensorflow/lite/micro/micro_time.cc \
	source/tensorflow/lite/micro/micro_utils.cc \
	source/tensorflow/lite/micro/recording_micro_allocator.cc \
	source/tensorflow/lite/micro/recording_simple_memory_allocator.cc \
	source/tensorflow/lite/micro/simple_memory_allocator.cc \
	source/tensorflow/lite/micro/test_helpers.cc \
	source/tensorflow/lite/micro/benchmarks/keyword_scrambled_model_data.cc \
	source/tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc \
	source/tensorflow/lite/micro/memory_planner/linear_memory_planner.cc \
	source/tensorflow/lite/micro/testing/test_conv_model.cc \
	source/tensorflow/lite/c/common.c \
	source/tensorflow/lite/core/api/error_reporter.cc \
	source/tensorflow/lite/core/api/flatbuffer_conversions.cc \
	source/tensorflow/lite/core/api/op_resolver.cc \
	source/tensorflow/lite/core/api/tensor_utils.cc \
	source/tensorflow/lite/kernels/internal/quantization_util.cc \
	source/tensorflow/lite/kernels/kernel_util.cc \
	source/tensorflow/lite/schema/schema_utils.cc \
	source/tensorflow/lite/micro/kernels/activations.cc \
	source/tensorflow/lite/micro/kernels/arg_min_max.cc \
	source/tensorflow/lite/micro/kernels/ceil.cc \
	source/tensorflow/lite/micro/kernels/circular_buffer.cc \
	source/tensorflow/lite/micro/kernels/comparisons.cc \
	source/tensorflow/lite/micro/kernels/concatenation.cc \
	source/tensorflow/lite/micro/kernels/conv_test_common.cc \
	source/tensorflow/lite/micro/kernels/dequantize.cc \
	source/tensorflow/lite/micro/kernels/detection_postprocess.cc \
	source/tensorflow/lite/micro/kernels/elementwise.cc \
	source/tensorflow/lite/micro/kernels/ethosu.cc \
	source/tensorflow/lite/micro/kernels/flexbuffers_generated_data.cc \
	source/tensorflow/lite/micro/kernels/floor.cc \
	source/tensorflow/lite/micro/kernels/hard_swish.cc \
	source/tensorflow/lite/micro/kernels/kernel_runner.cc \
	source/tensorflow/lite/micro/kernels/kernel_util.cc \
	source/tensorflow/lite/micro/kernels/l2norm.cc \
	source/tensorflow/lite/micro/kernels/logical.cc \
	source/tensorflow/lite/micro/kernels/logistic.cc \
	source/tensorflow/lite/micro/kernels/maximum_minimum.cc \
	source/tensorflow/lite/micro/kernels/neg.cc \
	source/tensorflow/lite/micro/kernels/pack.cc \
	source/tensorflow/lite/micro/kernels/pad.cc \
	source/tensorflow/lite/micro/kernels/prelu.cc \
	source/tensorflow/lite/micro/kernels/quantize.cc \
	source/tensorflow/lite/micro/kernels/quantize_common.cc \
	source/tensorflow/lite/micro/kernels/reduce.cc \
	source/tensorflow/lite/micro/kernels/reshape.cc \
	source/tensorflow/lite/micro/kernels/resize_nearest_neighbor.cc \
	source/tensorflow/lite/micro/kernels/round.cc \
	source/tensorflow/lite/micro/kernels/shape.cc \
	source/tensorflow/lite/micro/kernels/split.cc \
	source/tensorflow/lite/micro/kernels/split_v.cc \
	source/tensorflow/lite/micro/kernels/strided_slice.cc \
	source/tensorflow/lite/micro/kernels/sub.cc \
	source/tensorflow/lite/micro/kernels/svdf_common.cc \
	source/tensorflow/lite/micro/kernels/tanh.cc \
	source/tensorflow/lite/micro/kernels/transpose_conv.cc \
	source/tensorflow/lite/micro/kernels/unpack.cc \
	source/tensorflow/lite/micro/kernels/add.cc \
	source/tensorflow/lite/micro/kernels/conv.cc \
	source/tensorflow/lite/micro/kernels/depthwise_conv.cc \
	source/tensorflow/lite/micro/kernels/fully_connected.cc \
	source/tensorflow/lite/micro/kernels/mul.cc \
	source/tensorflow/lite/micro/kernels/pooling.cc \
	source/tensorflow/lite/micro/kernels/softmax.cc \
	source/tensorflow/lite/micro/kernels/svdf.cc

ifeq ($(CONFIG_INTELLIGENT_TFLITE),y)
	SRC_FILES := $(TFLITE_SRCS) tf_fini_fix.c
	#CPPPATHS += \
		-Isource \
		-Isource/third_party/gemmlowp \
		-Isource/third_party/flatbuffers/include \
		-Isource/third_party/ruy
	DEFINES += -DTF_LITE_USE_GLOBAL_CMATH_FUNCTIONS \
		-DTF_LITE_USE_GLOBAL_MAX \
		-DTF_LITE_USE_GLOBAL_MIN \
		-DTF_LITE_STATIC_MEMORY
endif

include $(KERNEL_ROOT)/compiler.mk
