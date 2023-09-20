#!/usr/bin/env python3

from ina3221_measurement import init_monitor;

if __name__ == '__main__':
  adc = init_monitor()
  while True:
    print('Voltage: {}\n'.format(adc.current(1)))
