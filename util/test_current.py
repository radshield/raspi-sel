#!/usr/bin/env python3

import board
import busio
from barbudor_ina3221.lite import INA3221


def init_monitor():
    i2c = busio.I2C(board.SCL, board.SDA)
    ina3221 = INA3221(i2c)

    for channel in range(1, 3):
        ina3221.enable_channel(channel)

    return ina3221


if __name__ == '__main__':
  adc = init_monitor()
  while True:
    print('Voltage: {}\n'.format(adc.current(1)))
