#!/usr/bin/env python3

from ina169_measurement import init_adc;

if __name__ == '__main__':
  adc = init_adc()
  while True:
    print('Voltage: {}\n'.format(adc.voltage))

