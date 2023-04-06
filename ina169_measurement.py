#!/usr/bin/env python3

import board
import busio
import datetime
import psutil
import time

import adafruit_ads1x15.ads1115 as ADS
from adafruit_ads1x15.analog_in import AnalogIn

data_rate = 860

def init_adc():
  i2c = busio.I2C(board.SCL, board.SDA)
  ads = ADS.ADS1115(i2c, data_rate=data_rate, mode=ADS.Mode.SINGLE)
  return AnalogIn(ads, ADS.P0)

def main_loop(adc, filename='currents.log'):
  log = open(filename, 'a')
  log.write('time,cpu_percent,virtual_memory,adc_voltage\n')
  log.flush()

  while True:
    log.write('{},{},{},{}\n'.format(str(datetime.datetime.now()),
                                     psutil.cpu_percent(),
                                     psutil.virtual_memory().percent,
                                     adc.voltage))
    log.flush()
    time.sleep(1 / data_rate)

if __name__ == '__main__':
  adc = init_adc()
  main_loop(adc)
