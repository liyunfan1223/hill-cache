import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), '../..'))

from scripts.config import config

trace_list = config['trace_list']

def run(access):
    mp = {}
    ats = {}
    ts = 0
    interval = 0
    ninterval = 0
    tot_freq = 0
    nHiF = 1
    for i in access:
        ts += 1
        if i in mp.keys():
            mp[i] += 1
            interval += (ts - ats[i])
            ninterval += 1
            ats[i] = ts
        else:
            mp[i] = 1
            ats[i] = ts
    sort_result = sorted(mp.items(), key=lambda x: x[1], reverse=True)
    for idx, item in enumerate(sort_result):
        if (idx == nHiF):
            break
        tot_freq += item[1]
    avg_freq = len(access) / len(mp)
    avg_hiF = tot_freq / nHiF
    avg_interval = interval / ninterval

    print(f'requests: {len(access)}')
    print(f'unique keys: {len(mp.keys())}')
    print("average freq:", avg_freq)
    print(f"average high {nHiF} freq :", avg_hiF)
    print("average interval:", avg_interval)

def GetUniqueKeys(trace_file):
    access = []
    with open(trace_file, "r") as f:
        for line in f.readlines():
            k, r, _, _ = line.strip().split()
            k = int(k)
            r = int(r)
            for i in range(k, k + r):
                access.append(i)
    mp = {}
    ats = {}
    ts = 0
    interval = 0
    ninterval = 0
    tot_freq = 0
    nHiF = 1
    for i in access:
        ts += 1
        if i in mp.keys():
            mp[i] += 1
            interval += (ts - ats[i])
            ninterval += 1
            ats[i] = ts
        else:
            mp[i] = 1
            ats[i] = ts
    sort_result = sorted(mp.items(), key=lambda x: x[1], reverse=True)
    for idx, item in enumerate(sort_result):
        if (idx == nHiF):
            break
        tot_freq += item[1]
    avg_freq = len(access) / len(mp)
    avg_hiF = tot_freq / nHiF
    avg_interval = interval / ninterval
    return len(mp.keys())

def GetRequests(trace_file):
    access = []
    reqs = 0
    with open(trace_file, "r") as f:
        for line in f.readlines():
            k, r, _, _ = line.strip().split()
            k = int(k)
            r = int(r)
            reqs += r
    return reqs

def analyzer(trace_file):
    access = []
    with open(trace_file, "r") as f:
        for line in f.readlines():
            k, r, _, _ = line.strip().split()
            k = int(k)
            r = int(r)
            for i in range(k, k + r):
                access.append(i)
    run(access)
    GetUniqueKeys(trace_file)

if __name__ == "__main__":
    sum = 0
    count = 0
    for trace in trace_list:
        print(f"Analyzing {trace}:")
        analyzer(f"traces/{trace}.lis")