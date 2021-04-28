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
# ==========================================================================================
#!/usr/bin/env python3

import os
import tensorflow as tf

print("TensorFlow version %s" % (tf.__version__))

MODEL_NAME_H5 = 'mnist.h5'
MODEL_NAME_TFLITE = 'mnist.tflite'
DEFAULT_QUAN_MODEL_NAME_TFLITE = 'mnist-default-quan.tflite'
FULL_QUAN_MODEL_NAME_TFLITE = 'mnist-full-quan.tflite'

def build_model(model_name):
    print('\n>>> load mnist dataset')
    mnist = tf.keras.datasets.mnist
    (train_images, train_labels),(test_images, test_labels) = mnist.load_data()
    print("train images shape: ", train_images.shape)
    print("train labels shape: ", train_labels.shape)
    print("test images shape: ", test_images.shape)
    print("test labels shape: ", test_labels.shape)

    # transform label to categorical, like: 2 -> [0, 0, 1, 0, 0, 0, 0, 0, 0, 0]
    print('\n>>> transform label to categorical')
    train_labels = tf.keras.utils.to_categorical(train_labels)
    test_labels = tf.keras.utils.to_categorical(test_labels)
    print("train labels shape: ", train_labels.shape)
    print("test labels shape: ", test_labels.shape)

    # transform color like: [0, 255] -> 0.xxx
    print('\n>>> transform image color into float32')
    train_images = train_images.astype('float32') / 255
    test_images = test_images.astype('float32') / 255

    # reshape image like: (60000, 28, 28) -> (60000, 28, 28, 1)
    print('\n>>> reshape image with color channel')
    train_images = train_images.reshape((60000, 28, 28, 1))
    test_images = test_images.reshape((10000, 28, 28, 1))
    print("train images shape: ", train_images.shape)
    print("test images shape: ", test_images.shape)

    print('\n>>> build model')
    model = tf.keras.models.Sequential([
        tf.keras.layers.Conv2D(32, (3, 3), activation=tf.nn.relu, input_shape=(28, 28, 1)),
        tf.keras.layers.MaxPooling2D((2, 2)),
        tf.keras.layers.Conv2D(64, (3, 3), activation=tf.nn.relu),
        tf.keras.layers.MaxPooling2D((2, 2)),
        tf.keras.layers.Conv2D(64, (3, 3), activation=tf.nn.relu),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dense(64, activation=tf.nn.relu),
        tf.keras.layers.Dense(10, activation=tf.nn.softmax)
    ])
    model.compile(optimizer='rmsprop',
                  loss='categorical_crossentropy',
                  metrics=['accuracy'])
    model.summary()

    print('\n>>> train the model')
    early_stopping = tf.keras.callbacks.EarlyStopping(
        monitor='loss', min_delta=0.0005, patience=3, verbose=1, mode='auto',
        baseline=None, restore_best_weights=True
    )
    model.fit(train_images, train_labels, epochs=100, batch_size=64, callbacks=[early_stopping])

    print('\n>>> evaluate the model')
    test_loss, test_acc = model.evaluate(test_images, test_labels)
    print("lost: %f, accuracy: %f" % (test_loss, test_acc))

    print('\n>>> save the keras model as %s' % model_name)
    model.save(model_name)


if __name__ == '__main__':

    if not os.path.exists(MODEL_NAME_H5):
        build_model(MODEL_NAME_H5)

    if not os.path.exists(MODEL_NAME_TFLITE):
        print('\n>>> save the tflite model as %s' % MODEL_NAME_TFLITE)
        converter = tf.lite.TFLiteConverter.from_keras_model(tf.keras.models.load_model(MODEL_NAME_H5))
        tflite_model = converter.convert()
        with open(MODEL_NAME_TFLITE, "wb") as f:
            f.write(tflite_model)

    if not os.path.exists(DEFAULT_QUAN_MODEL_NAME_TFLITE):
        print('\n>>> save the default quantized model as %s' % DEFAULT_QUAN_MODEL_NAME_TFLITE)
        converter = tf.lite.TFLiteConverter.from_keras_model(tf.keras.models.load_model(MODEL_NAME_H5))
        converter.optimizations = [tf.lite.Optimize.DEFAULT]
        tflite_model = converter.convert()
        with open(DEFAULT_QUAN_MODEL_NAME_TFLITE, "wb") as f:
            f.write(tflite_model)

    if not os.path.exists(FULL_QUAN_MODEL_NAME_TFLITE):
        mnist = tf.keras.datasets.mnist
        (train_images, _), (_, _) = mnist.load_data()
        train_images = train_images.astype('float32') / 255
        train_images = train_images.reshape((60000, 28, 28, 1))
        def representative_data_gen():
            for input_value in tf.data.Dataset.from_tensor_slices(train_images).batch(1).take(100):
                yield [input_value]
        print('\n>>> save the full quantized model as %s' % DEFAULT_QUAN_MODEL_NAME_TFLITE)
        converter = tf.lite.TFLiteConverter.from_keras_model(tf.keras.models.load_model(MODEL_NAME_H5))
        converter.optimizations = [tf.lite.Optimize.DEFAULT]
        converter.representative_dataset = representative_data_gen
        converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
        converter.inference_input_type = tf.uint8
        converter.inference_output_type = tf.uint8
        tflite_model = converter.convert()
        with open(FULL_QUAN_MODEL_NAME_TFLITE, "wb") as f:
            f.write(tflite_model)
