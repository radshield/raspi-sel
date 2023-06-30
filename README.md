# SEL Detection for Commodity Single-Board Computers

This repo stores scratch files for benchmarking and measurement of current draw for the Raspberry Pi Zero 2 W.

## Requirements

* Python 3.6 or higher with pip
* virtualenv to keep dependencies separate (optional)

## Setup

```bash
virtualenv venv && ./venv/bin/activate
pip3 install -r requirements.txt
```

## Usage

Call either `ina169_measurement.py` or `ina3221_measurement.py` depending on which chip is being used to measure current draw. It takes a few arguments:

* `--data-rate`: Number of measurements to record per second. This defaults to `100`, and should not exceed the polling rate of the measuring IC
* The path to one input file, which describes the test(s) to be run. The format is described in the following section.

## Configuration

The layout of a configuration JSON file should look like the following:

```json
{
  "runs": [
    {
      "commands": [],
      "run_time": 5
    }
  ]
}
```

The `runs` object contains an array of every test to be run. Each element is an object with two attributes:

* `commands` is a set of commands to be executed in parallel. You can leave it black for it to not execute anything
* `run_time` is a timeout value for the maximum time the above commands get run for, after which the commands are killed and the next set of tests, if any, are started.

