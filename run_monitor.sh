#!/usr/bin/env bash

# Record 1 minute of system data
./record input.csv 60

# Create linear model using collected system data
./build_model.py input.csv full_telemetry

# Run ILD using previous data
./seelie full_telemetry
