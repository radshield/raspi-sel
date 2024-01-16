# SEL Detection for Commodity Single-Board Computers

This repo stores scratch files for benchmarking and measurement of current draw for the Raspberry Pi Zero 2 W.

## Requirements

* CMake 3.16+
* ninja-build
* `libi2c-dev`
* `libcpupower-dev`

## Building Recording Tool

```bash
cmake -S . -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ninja -C build
```

## Usage

The recording script needs to access `perf` and `/dev` to record system and INA3221 statistics. Thus, you'll probably want to run it with sudo:

```bash
sudo build/bin/record OUTPUT_FILENAME
```
