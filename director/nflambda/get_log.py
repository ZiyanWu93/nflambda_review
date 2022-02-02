import pysftp
import os
import pickle
import pandas as pd


def get_nflambda_log():
    myHostname = "aum"
    myUsername = "anon"
    remoteFilePath = '/tmp/nf_platform_log.csv'
    localFilePath = './nflambda_log/log.csv'
    with pysftp.Connection(host=myHostname, username=myUsername) as sftp:
        sftp.get(remoteFilePath, localFilePath)

    df = pd.read_csv("./nflambda_log/log.csv", header=None)
    df[0] = pd.to_datetime(df[0])
    key = df[1].unique()
    DataFrameDict = {elem: pd.DataFrame for elem in key}

    for key in DataFrameDict.keys():
        DataFrameDict[key] = df[:][df[1] == key]
        DataFrameDict[key].set_index(0, inplace=True)
        DataFrameDict[key].drop(columns=[1], axis=1, inplace=True)
        DataFrameDict[key] = DataFrameDict[key].pivot_table(values=3, index=DataFrameDict[key].index, columns=2,
                                                            aggfunc='first')
        DataFrameDict[key]['Time'] = DataFrameDict[key].index.asi8
        DataFrameDict[key]['Time_diff'] = DataFrameDict[key]['Time'].diff()
        if key == "worker":
            DataFrameDict["worker"]["throughput"] = 0
            for i in range(1, 49, 2):
                if i in DataFrameDict[key].columns:
                    DataFrameDict[key][str(i) + "_diff"] = DataFrameDict[key][i].diff()
                    DataFrameDict[key][str(i) + "_throughput"] = DataFrameDict[key][str(i) + "_diff"] / (
                            DataFrameDict[key].Time_diff / 1e9) * 84 * 8 / 1e9
                    DataFrameDict[key]["throughput"] = DataFrameDict[key]["throughput"] + DataFrameDict[key][str(i) + "_throughput"]
        else:
            for i in range(0, 1000):
                if i in DataFrameDict[key].columns:
                    DataFrameDict[key][str(i) + "_diff"] = DataFrameDict[key][1].diff()
                    DataFrameDict[key][str(i) + "_throughput"] = DataFrameDict[key][str(i) + "_diff"] / (
                            DataFrameDict[key].Time_diff / 1e9)

    return DataFrameDict

def get_nflambda_log_2():
    myHostname = "aum"
    myUsername = "anon"
    remoteFilePath = '/tmp/nf_platform_log.csv'
    localFilePath = './nflambda_log/log.csv'
    with pysftp.Connection(host=myHostname, username=myUsername) as sftp:
        sftp.get(remoteFilePath, localFilePath)

    df = pd.read_csv("./nflambda_log/log.csv", header=None)
    return df