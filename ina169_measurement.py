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
  return AnalogIn(ads, ADS.P0)

def main_loop(adc, tests, data_rate, out_file='currents.log'):
  log = open(out_file, 'a')
  log.write('time,cpu_percent,virtual_memory,adc_voltage\n')
  log.flush()

  for test in tests:
    if len(test['command']) > 0:
      test_load = subprocess.Popen(test['command'].split(' '))
    else:
      test_load = None

    for i in range(test['run_time'] * data_rate):
      log.write('{},{},{},{}\n'.format(str(datetime.datetime.now()),
                                       psutil.cpu_percent(),
                                       psutil.virtual_memory().percent,
                                       adc.voltage))
      log.flush()
      time.sleep(1 / data_rate)

    if test_load is not None and test_load.poll() is None:
      test_load.terminate()

if __name__ == '__main__':
  args = parser.parse_args()

  with open(args.input_file[0]) as f:
    tests = json.load(f)['runs']

  adc = init_adc()
  main_loop(adc, tests, args.data_rate)
