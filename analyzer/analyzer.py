#!/usr/bin/env python3

import argparse
import scipy

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

def runtime(file_location, target_pid):
    pass

def train_model(file_location):
    pass

def main():
    args = parser.parse_args()

    if args.train:
        train_model(args.filename)
    else:
        runtime(args.filename, args.pid)

if __name__ == '__main__':
    main()