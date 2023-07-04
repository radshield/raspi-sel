#!/usr/bin/env python3

import argparse
import board
import busio
import datetime
import json
import psutil
import subprocess
import time

from barbudor_ina3221.lite import INA3221

parser = argparse.ArgumentParser(description='Measure current with load and peripherals on INA3221.')
parser.add_argument('-d', '--data-rate', metavar='N', dest='data_rate', default=100,
                    help='number of measurements to record per second')
parser.add_argument('input_file', type=str, nargs=1,
                    help='input file for commands to run, as JSON')

def init_monitor(channels=[0, 1]):
  i2c = busio.I2C(board.SCL, board.SDA)
  monitor = INA3221(i2c)

  for channel in channels:
    monitor.enable_channel(channel)

  return monitor

def main_loop(monitor, tests, channels=[0, 1], data_rate=100, out_file='currents.log'):
  log = open(out_file, 'w')
  log.write('time,cpu_percent_0,cpu_percent_1,cpu_percent_2,cpu_percent_3,' +
            'virtual_memory,adc_voltage_all,adc_voltage_payload\n')
  log.flush()
  
  for test in tests:
    test_loads = []
    for command in test['commands']:
      test_loads.append(subprocess.Popen(command.split(' ')))

    for i in range(test['run_time'] * data_rate):
      log.write('{},{},{},{},{},{},{},{}\n'.format(str(datetime.datetime.now()),
        100.0 - psutil.cpu_times_percent(percpu=True)[0].idle,
        100.0 - psutil.cpu_times_percent(percpu=True)[1].idle,
        100.0 - psutil.cpu_times_percent(percpu=True)[2].idle,
        100.0 - psutil.cpu_times_percent(percpu=True)[3].idle,
        psutil.virtual_memory().percent,
        monitor.current(channels[0]),
        monitor.current(channels[1])))
      log.flush()
      time.sleep(1 / data_rate)

    for test_load in test_loads:
      if test_load is not None and test_load.poll() is None:
        test_load.kill()

if __name__ == '__main__':
  args = parser.parse_args()

  with open(args.input_file[0]) as f:
    tests = json.load(f)['runs']

  monitor = init_monitor()
  main_loop(monitor, tests, data_rate=int(args.data_rate))

