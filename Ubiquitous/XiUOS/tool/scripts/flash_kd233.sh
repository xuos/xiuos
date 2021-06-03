#!/bin/sh
#
BOARD=kd233
TTY_PORT=/dev/ttyUSB${1}
BIN=build/XiUOS_k210.bin

kflash -B ${BOARD} \
-p ${TTY_PORT} \
-t  ${BIN}
