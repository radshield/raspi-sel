#!/usr/bin/env python3

import argparse
import pandas as pd
from sklearn.linear_model import LinearRegression

k = 251


def percent_scaling(series):
    return series / 100


def cpu_scaling(series):
    return series * 4 / 100


def single_cpu_scaling(series):
    return ((series - 100) * (-1)) / 100


def min_max_scaling(series):
    return (series - series.min()) / (series.max() - series.min())


def preprocess_df(df):
    df["cache_hit_rate_0"] = (df["cache_hit_0"] - df["cache_miss_0"]) / df[
        "cache_hit_0"
    ]
    df["cache_hit_rate_1"] = (df["cache_hit_0"] - df["cache_miss_1"]) / df[
        "cache_hit_1"
    ]
    df["cache_hit_rate_2"] = (df["cache_hit_0"] - df["cache_miss_2"]) / df[
        "cache_hit_2"
    ]
    df["cache_hit_rate_3"] = (df["cache_hit_0"] - df["cache_miss_3"]) / df[
        "cache_hit_3"
    ]

    df["br_miss_rate_0"] = df["br_miss_0"] / df["br_insns_0"]
    df["br_miss_rate_1"] = df["br_miss_1"] / df["br_insns_1"]
    df["br_miss_rate_2"] = df["br_miss_2"] / df["br_insns_2"]
    df["br_miss_rate_3"] = df["br_miss_3"] / df["br_insns_3"]

    df["insns_0_norm"] = df["insns_0"] / (df["time"] - df["time"].shift())
    df["insns_1_norm"] = df["insns_1"] / (df["time"] - df["time"].shift())
    df["insns_2_norm"] = df["insns_2"] / (df["time"] - df["time"].shift())
    df["insns_3_norm"] = df["insns_3"] / (df["time"] - df["time"].shift())

    df["cycles_0_norm"] = df["cpu_cycles_0"] / (df["time"] - df["time"].shift())
    df["cycles_1_norm"] = df["cpu_cycles_1"] / (df["time"] - df["time"].shift())
    df["cycles_2_norm"] = df["cpu_cycles_2"] / (df["time"] - df["time"].shift())
    df["cycles_3_norm"] = df["cpu_cycles_3"] / (df["time"] - df["time"].shift())

    df["bus_cycles_0_norm"] = df["bus_cycles_0"] / (df["time"] - df["time"].shift())
    df["bus_cycles_1_norm"] = df["bus_cycles_1"] / (df["time"] - df["time"].shift())
    df["bus_cycles_2_norm"] = df["bus_cycles_2"] / (df["time"] - df["time"].shift())
    df["bus_cycles_3_norm"] = df["bus_cycles_3"] / (df["time"] - df["time"].shift())

    df["rd_ios_norm"] = df["rd_ios"] * 1000 / (df["time"] - df["time"].shift())
    df["wr_ios_norm"] = df["wr_ios"] * 1000 / (df["time"] - df["time"].shift())

    df["curr_filt_2"] = (
        df["current_2"].rolling(window=k, center=True, min_periods=1).min()
    )
    df["curr_filt_3"] = (
        df["current_3"].rolling(window=k, center=True, min_periods=1).min()
    )

    df.drop(df.tail(k // 2).index, inplace=True)
    df.drop(df.head(k // 2).index, inplace=True)

    return df


parser = argparse.ArgumentParser(
    prog="build_model", description="Builds a latchup detection model for ILD to use"
)
parser.add_argument('input_csv')
parser.add_argument('output_file')

if __name__ == "__main__":
    args = parser.parse_args()

    df = pd.read_csv(args.input_csv)

    X = df[
        [
            "curr_filt_3",
            "insns_0_norm",
            "insns_1_norm",
            "insns_2_norm",
            "insns_3_norm",
            "bus_cycles_0_norm",
            "bus_cycles_1_norm",
            "bus_cycles_2_norm",
            "bus_cycles_3_norm",
            "br_miss_rate_0",
            "br_miss_rate_1",
            "br_miss_rate_2",
            "br_miss_rate_3",
            "rd_ios_norm",
            "wr_ios_norm",
            "freq_0",
            "freq_1",
            "freq_2",
            "freq_3",
            "cache_hit_rate_0",
            "cache_hit_rate_1",
            "cache_hit_rate_2",
            "cache_hit_rate_3",
        ]
    ]
    Y = df["curr_filt_2"]

    model = LinearRegression(n_jobs=8)
    model.fit(X, Y)

    with open(args.output_file) as f:
        for coef in model.coef_:
            f.write(coef)
            f.write('\n')
        f.write(str(model.intercept_))
