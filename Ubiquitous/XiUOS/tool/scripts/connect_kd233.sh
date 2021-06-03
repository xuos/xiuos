#!/bin/sh
#
#BOARD=kd233
TTY_PORT=/dev/ttyUSB${1}
BAUDRATE=115200
#needed for USB-TypeC port, but not for raw uart(TX/RX)
OTHER_OPT='--dtr=0 --rts=0'

python3 -m serial.tools.miniterm \
--raw  \
${OTHER_OPT} \
--filter colorize  \
${TTY_PORT} \
${BAUDRATE}
