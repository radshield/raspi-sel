# SEL Detection for Commodity Single-Board Computers

This repo stores a prototype program for detecting radiation-induced latchups on
the Raspberry Pi Zero 2 W.

## Requirements

* Python 3.9 or higher with pip and CPython support
* virtualenv to keep dependencies separate (optional)
* CMake
* Linux with perf and cpufreq support
* INA3221 connected to the Raspberry Pi through GPIO

## Setup

```bash
# Install dependencies
sudo apt-get install -y libcpupower-dev linux-perf libboost-dev libi2c-dev
sudo apt-get install -y ninja-build cmake clang

# Build recording tool
mkdir build && cd build
CC=clang CXX=clang++ cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
ninja
```

## Usage

A model will first need to be built. This can be done with the included Python
scripts.

Once complete, the prototye can be run from the root directory of the project as
so:

```bash
./build/seelie model_to_use
```
