#!/usr/bin/env python

import board, busio
import psutil
import adafruit_ads1x15.ads1115 as ADS
from adafruit_ads1x15.analog_in import AnalogIn

def init_adc():
  i2c = busio.I2C(board.SCL, board.SDA)
  ads = ADS.ADS1115(i2c, data_rate=860, mode=ADS.Mode.CONTINUOUS)
  return AnalogIn(ads, ADS.P0)


def main():
  adc = init_adc()
  print(psutil.cpu_percent(), psutil.virtual_memory().percent, adc.voltage)

if __name__ == '__main__':
  main()
