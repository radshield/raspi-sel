#!/usr/bin/env python3

import argparse
import os
import subprocess
import sys
import tempfile

parser = argparse.ArgumentParser(description='Measure current with load and peripherals on INA3221.')
parser.add_argument('-d', '--data-rate', metavar='N', dest='data_rate', default=100,
                    help='number of measurements to record per second')
parser.add_argument('-o', '--output', metavar='N', dest='output_file', default=0,
                    help='name of output file')
parser.add_argument('operation', metavar='[start/stop]', type=str, nargs=1,
                    help='start or stop logging')


if __name__ == '__main__':
    args = parser.parse_args()

    if args.operation[0] == 'start':
        output_file = args.output_file
        if output_file == 0:
            temp_dir = tempfile.mkdtemp()
            output_file = os.path.join(temp_dir, 'currents.log')

        print('Output file: {}'.format(output_file))

        proc = subprocess.Popen([sys.executable,
                                 os.path.join(os.path.dirname(os.path.realpath(__file__)),
                                              'ina3221_measurement.py'),
                                 '-o', output_file,
                                 '-d', str(args.data_rate)])

        print('Now running monitor with pid {}'.format(proc.pid))

        with open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'proc.pid'), 'w') as f:
            f.write(str(proc.pid))
    elif args.operation[0] == 'stop':
        with open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'proc.pid')) as f:
            pid = int(f.readline())

        subprocess.call(['kill', '-SIGUSR1', str(pid)])
        os.remove(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'proc.pid'))
        print('Killed process with pid {}'.format(pid))
    else:
        print('ERROR: unrecognized operation')
