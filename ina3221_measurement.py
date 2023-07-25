#!/usr/bin/env python3

import argparse
import board
import busio
import datetime
import json
import psutil
import time

from barbudor_ina3221.lite import INA3221

parser = argparse.ArgumentParser(description='Measure current with load and peripherals on INA3221.')
parser.add_argument('-d', '--data-rate', metavar='N', dest='data_rate', default=100,
                    help='number of measurements to record per second')
parser.add_argument('-r', '--run-time', metavar='N', dest='run_time', default=100,
                    help='number of seconds to record data')


def init_monitor(channels=[0, 1, 2]):
    i2c = busio.I2C(board.SCL, board.SDA)
    ina3221 = INA3221(i2c)

    for channel in channels:
        ina3221.enable_channel(channel)

    return ina3221


def main_loop(ina3221, run_time=60, channels=[0, 1, 2], data_rate=100, out_file='currents.log'):
    log = open(out_file, 'w')
    log.write('time,cpu_percent_0,cpu_percent_1,cpu_percent_2,cpu_percent_3,' +
              'virtual_memory,adc_voltage_all,adc_voltage_payload,adc_voltage_peripherals\n')
    log.flush()

    for i in range(data_rate * run_time):
        log.write('{},{},{},{},{},{},{},{},{}\n'.format(str(datetime.datetime.now()),
                                                        100.0 - psutil.cpu_times_percent(percpu=True)[0].idle,
                                                        100.0 - psutil.cpu_times_percent(percpu=True)[1].idle,
                                                        100.0 - psutil.cpu_times_percent(percpu=True)[2].idle,
                                                        100.0 - psutil.cpu_times_percent(percpu=True)[3].idle,
                                                        psutil.virtual_memory().percent,
                                                        ina3221.current(channels[0]),
                                                        ina3221.current(channels[1]),
                                                        ina3221.current(channels[2])))
        log.flush()
        time.sleep(1 / data_rate)


if __name__ == '__main__':
    args = parser.parse_args()

    with open(args.input_file[0]) as f:
        tests = json.load(f)['runs']

    monitor = init_monitor()
    main_loop(monitor, tests, run_time=int(args.run_time), data_rate=int(args.data_rate))
