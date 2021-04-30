#!/usr/bin/env python3
# ==========================================================================================
# Copyright (c) 2020 AIIT XUOS Lab
# XiOS is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#        http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

#  @file:    mnist-inference.py
#  @brief:   load data amd start model omferemce
#  @version: 1.0
#  @author:  AIIT XUOS Lab
#  @date:    2021/4/30
# ==========================================================================================


import tensorflow as tf

print("TensorFlow version %s" % (tf.__version__))

MODEL_NAME_H5 = 'mnist.h5'
MODEL_NAME_TFLITE = 'mnist.tflite'
DEFAULT_QUAN_MODEL_NAME_TFLITE = 'mnist-default-quan.tflite'
FULL_QUAN_MODEL_NAME_TFLITE = 'mnist-full-quan.tflite'


def show(image):
    for i in range(28):
        for j in range(28):
            if image[i][j][0] > 0.3:
                print('#', end = '')
            else:
                print(' ', end = '')
        print()


if __name__ == '__main__':
    mnist = tf.keras.datasets.mnist
    (_, _), (test_images, test_labels) = mnist.load_data()
    test_images = test_images.reshape(10000, 28, 28, 1)
    index = 0
    input_image = test_images[index].astype('float32')/255
    target_label = test_labels[index]

    interpreter = tf.lite.Interpreter(model_path = DEFAULT_QUAN_MODEL_NAME_TFLITE)
    interpreter.allocate_tensors()
    input_details = interpreter.get_input_details()[0]
    output_details = interpreter.get_output_details()[0]
    interpreter.set_tensor(input_details['index'], [input_image])
    interpreter.invoke()
    output = interpreter.get_tensor(output_details['index'])[0]

    show(input_image)
    print('target label: %d, predict label: %d' % (target_label, output.argmax()))
