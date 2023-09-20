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


def init_monitor():
    i2c = busio.I2C(board.SCL, board.SDA)
    ina3221 = INA3221(i2c)

    for channel in range(1, 3):
        ina3221.enable_channel(channel)

    return ina3221


def main_loop(ina3221, run_time=60, channels=[0, 1, 2], data_rate=100, out_file='currents.log'):
    log = open(out_file, 'w')
    log.write('time,cpu_percent_0,cpu_percent_1,cpu_percent_2,cpu_percent_3,' +
              'virtual_memory,adc_voltage_all,adc_voltage_payload,adc_voltage_peripherals\n')
    log.flush()

    for test in tests:
        test_loads = []
        for command in test['commands']:
            test_loads.append(subprocess.Popen(command.split(' ')))

        for i in range(test['run_time'] * data_rate):
            log.write('{},{},{},{},{},{},{}\n'.format(str(datetime.datetime.now()),
                                                      psutil.cpu_percent(interval=None, percpu=True)[0],
                                                      psutil.cpu_percent(interval=None, percpu=True)[1],
                                                      psutil.cpu_percent(interval=None, percpu=True)[2],
                                                      psutil.cpu_percent(interval=None, percpu=True)[3],
                                                      psutil.virtual_memory().percent,
                                                      ina3221.current(0)))
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
