#!/usr/bin/env python3

import argparse
import board
import busio
import os
import psutil
import tempfile
import time

from barbudor_ina3221.lite import INA3221

parser = argparse.ArgumentParser(description='Measure current with load and peripherals on INA3221.')
parser.add_argument('-d', '--data-rate', metavar='N', dest='data_rate', default=100,
                    help='number of measurements to record per second')
parser.add_argument('-r', '--run-time', metavar='N', dest='run_time', default=120,
                    help='number of seconds to record data')
parser.add_argument('-o', '--output', metavar='N', dest='output_file', default=0,
                    help='name of output file')


def init_monitor():
    i2c = busio.I2C(board.SCL, board.SDA)
    ina3221 = INA3221(i2c)

    for channel in range(1, 4):
        ina3221.enable_channel(channel)

    return ina3221


def main_loop(ina3221, run_time, data_rate, out_file):
    log = open(out_file, 'w')
    log.write('time,cpu_percent_0,cpu_percent_1,cpu_percent_2,cpu_percent_3,' +
              'virtual_memory,adc_voltage_all,adc_voltage_payload,adc_voltage_peripherals\n')
    log.flush()

    for i in range(data_rate * run_time):
        log.write('{},{},{},{},{},{},{},{},{}\n'.format(str(time.time()),
                                                        100.0 - psutil.cpu_times_percent(percpu=True)[0].idle,
                                                        100.0 - psutil.cpu_times_percent(percpu=True)[1].idle,
                                                        100.0 - psutil.cpu_times_percent(percpu=True)[2].idle,
                                                        100.0 - psutil.cpu_times_percent(percpu=True)[3].idle,
                                                        psutil.virtual_memory().percent,
                                                        ina3221.current(1),
                                                        ina3221.current(2),
                                                        ina3221.current(3)))
        log.flush()
        time.sleep(1.0 / data_rate)


if __name__ == '__main__':
    args = parser.parse_args()

    output_file = args.output_file
    if output_file == 0:
        temp_dir = tempfile.mkdtemp()
        output_file = os.path.join(temp_dir, 'currents.log')

    print('Output file: {}'.format(output_file))

    monitor = init_monitor()
    main_loop(monitor,
              run_time=int(args.run_time),
              data_rate=int(args.data_rate),
              out_file=output_file)
