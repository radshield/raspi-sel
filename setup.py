#!/usr/bin/env python

from distutils.core import setup

setup(name='raspi-sel',
      version='1.0',
      description='Raspberry Pi Latchup Detection tools',
      url='https://github.com/h313/raspi-sel/',
      install_requires=['adafruit-circuitpython-ads1x15',
                        'adafruit-circuitpython-busdevice',
                        'barbudor-circuitpython-ina3221',
                        'psutil'],
      py_modules=[],
      scripts=['ina3221_measurement.py']
      )
