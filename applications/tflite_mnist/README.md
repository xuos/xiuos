# MNIST 说明

## 使用

tools/mnist-train.py 训练生成 mnist 模型。

tools/mnist-inference.py 使用 mnist 模型进行推理。

tools/mnist-c-model.py 将 mnist 模型转换成 C 的数组保存在 model.h 中。

tools/mnist-c-digit.py 将 mnist 数据集中的某个数字转成数组保存在 digit.h 中。

## 参考资料

https://tensorflow.google.cn/lite/performance/post_training_quantization

https://tensorflow.google.cn/lite/performance/post_training_integer_quant

https://colab.research.google.com/github/tensorflow/tensorflow/blob/master/tensorflow/lite/micro/examples/hello_world/train/train_hello_world_model.ipynb