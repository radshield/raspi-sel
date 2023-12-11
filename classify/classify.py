#!/usr/bin/env python3

import argparse
import collections
import pickle
import subprocess

from sklearn.linear_model import SGDRegressor

parser = argparse.ArgumentParser(prog='raspi-sel',
                                 description='Latchup Detector for Raspberry Pi')
parser.add_argument('--train',
                    help='Train linear model on benchmark',
                    action='store_true')
parser.add_argument('-p', '--pid',
                    help='Target process ID',
                    nargs=1)
parser.add_argument('filename',
                    help='Linear model file location',
                    nargs=1)

# Create input buffers for reading from logging process
input_buf = []
# Current draw
input_buf.append({
    'shunt_1': collections.deque(maxlen=200),
    'shunt_2': collections.deque(maxlen=200),
    'shunt_3': collections.deque(maxlen=200)})
# CPU metrics
for i in range(4):
    input_buf.append({
        'cpu_cycles': collections.deque(maxlen=200),
        'insns': collections.deque(maxlen=200),
        'cache_hit_rate': collections.deque(maxlen=200),
        'br_hit_rate': collections.deque(maxlen=200),
        'bus_cycles': collections.deque(maxlen=200),
        'cpu_freq': collections.deque(maxlen=200)})
# Disk metrics
input_buf.append({
    'rd_ios': collections.deque(maxlen=200),
    'wr_ios': collections.deque(maxlen=200)})

def runtime(file_location, target_pid):
    pass

def train_model(file_location):
    model = SGDRegressor()
    for freq in ['600000', '700000', '800000', '900000', '1000000']:
        for 
        test_load = subprocess.Popen(test['command'].split(' '))
    pickle.dump(model, open(file_location, 'wb'))
    pass

def main():
    args = parser.parse_args()

    if args.train:
        train_model(args.filename)
    else:
        runtime(args.filename, args.pid)

if __name__ == '__main__':
    main()
