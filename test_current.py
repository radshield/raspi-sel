#!/usr/bin/env python3

import argparse
import board
import busio
import datetime
import json
import psutil
import subprocess
import time

import adafruit_ads1x15.ads1115 as ADS
from adafruit_ads1x15.analog_in import AnalogIn

parser = argparse.ArgumentParser(description='Measure current with load on INA169.')
parser.add_argument('-d', '--data-rate', metavar='N', dest='data_rate', default=60,
                    help='number of measurements to record per second')
parser.add_argument('input_file', type=str, nargs=1,
                    help='input file for commands to run, as JSON')

def init_adc():
  i2c = busio.I2C(board.SCL, board.SDA)
  ads = ADS.ADS1115(i2c, data_rate=860, mode=ADS.Mode.SINGLE)
  return AnalogIn(ads, ADS.P1)

if __name__ == '__main__':
  args = parser.parse_args()

  with open(args.input_file[0]) as f:
    tests = json.load(f)['runs']

  adc = init_adc()
  while True:
    print(adc.voltage)
    print()
