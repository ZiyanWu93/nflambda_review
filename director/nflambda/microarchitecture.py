import time

import pysftp
import os
import pickle
import pandas as pd

def start_pcm():
    os.system(
        '''ssh ip_address "cd /home/anon/project/pcm && nohup sudo /home/anon/project/pcm/pcm-raw.x 0.5 -el /home/anon/project/pcm/event_list/l_1.txt -csv=/tmp/log/output.csv > foo.out 2> foo.err < /dev/null &"'''
    )
    return

def kill_pcm():
    os.system(
        '''ssh anon@ip_address "python /home/anon/PycharmProjects/nsdi_tools/kill_pcm.py"'''
    )
    return

def get_microarchitecture_snapshot():
    os.system(
        '''ssh anon@ip_address "python3 /home/anon/PycharmProjects/nsdi_tools/perf.py "'''
    )
    myHostname = "aum"
    myUsername = "anon"
    remoteFilePath = '/tmp/test.pkl'
    localFilePath = './profiling/test.pkl'
    with pysftp.Connection(host=myHostname, username=myUsername) as sftp:
        sftp.get(remoteFilePath, localFilePath)
    with open(localFilePath, 'rb') as infile:
        result = pickle.load(infile)

    return result

def get_microarchitecture_snapshot_2():
    myHostname = "aum"
    myUsername = "anon"
    remoteFilePath = '/tmp/log/output.csv'
    localFilePath = './profiling/log.csv'
    with pysftp.Connection(host=myHostname, username=myUsername) as sftp:
        sftp.get(remoteFilePath, localFilePath)

    df = pd.read_csv("profiling/log.csv", header=None)
    df[0] = pd.to_datetime(df[0])
    Counter_list = df[1].unique()
    DataFrameDict = {elem: pd.DataFrame for elem in Counter_list}

    for key in DataFrameDict.keys():
        DataFrameDict[key] = df[:][df[1] == key]
        DataFrameDict[key].set_index(0, inplace=True)
        DataFrameDict[key].drop(columns=[1], axis=1, inplace=True)
        DataFrameDict[key].columns = [i for i in range(0, 48, 1)]

    DataFrameDict["IPS"] = DataFrameDict['INST_RETIRED.ANY'] / DataFrameDict['INST_RETIRED.TOTAL_CYCLES_PS']
    DataFrameDict["L1_MISS"] = DataFrameDict['L1D.REPLACEMENT'] / DataFrameDict['MEM_INST_RETIRED.ALL_LOADS']
    DataFrameDict["L2_MISS"] = DataFrameDict['MEM_LOAD_RETIRED.L2_MISS'] / DataFrameDict['L1D.REPLACEMENT']
    DataFrameDict["L3_MISS"] = DataFrameDict['MEM_LOAD_RETIRED.L3_MISS'] / DataFrameDict['MEM_LOAD_RETIRED.L2_MISS']
    DataFrameDict["IPS"].dropna(inplace = True)
    DataFrameDict["L1_MISS"].dropna(inplace=True)
    DataFrameDict["L2_MISS"].dropna(inplace=True)
    DataFrameDict["L3_MISS"].dropna(inplace=True)
    return DataFrameDict