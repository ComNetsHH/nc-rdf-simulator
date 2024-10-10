import sys, os
import json
import pandas as pd
import numpy as np


def get_dissemation_ratio(data, nodeId):
    srcIds = data[data['nodeId'] == nodeId].src.unique()
    return len(srcIds)

def get_avg_aoi_by_node(data, nodeId):
    avgs = []
    srcIds = data[data['nodeId'] == nodeId].src.unique()
    for srcId in srcIds:
        aoi_ts = get_aoi_ts(data, nodeId, srcId)  
        avg = get_avg_aoi(aoi_ts)
        avgs.append(avg)
    return avgs


def get_aoi_ts(data, nodeId, srcId):
    data = data[(data['src'] == srcId) & (data['nodeId'] == nodeId)]
    ts = data['timestamp'].to_numpy() * 1000
    delay = data['delay'].to_numpy()

    rcvd_aoi = list(zip(ts, delay))
    result = []
    for entry in rcvd_aoi:
        if len(result) >= 1:
            prev_val = result[-1]
            age = entry[0] - prev_val[0]
            result.append((entry[0], prev_val[1] + age))
        result.append(entry)

    return result

def get_avg_aoi(aoi_ts):
    if len(aoi_ts) <= 1:
        return -1

    (x,y) = list(zip(*aoi_ts))
    total_time = aoi_ts[-1][0] - aoi_ts[0][0]
    return np.trapz(y, x) / total_time

def main(v, num_nodes, send_interval=500, run=1):
    data = pd.read_csv(f'./res/v{v}/pf_n{int(num_nodes)}_i{int(send_interval)}_r{run}.csv')
    forwarded_events = data[(data['eventType'] == 'PktFwd')]

    aoi = []
    dissemination_rates = []
    nodes = forwarded_events.nodeId.unique()

    for nodeId in nodes:
        res = get_avg_aoi_by_node(forwarded_events, nodeId)
        dissemination_rates.append(get_dissemation_ratio(forwarded_events, nodeId))
        aoi += res

    aoi = [x for x in aoi if x > 0]
    avg_diss_rate = np.mean(dissemination_rates) / (num_nodes -1)
    avg_aoi = np.mean(aoi)

    data = {
        'min_aoi': np.min(aoi),
        'max_aoi': np.max(aoi),
        'avg_aoi':avg_aoi,
        'avg_dissemination_rate': avg_diss_rate
    }

    with open(f'./res/v{v}_parsed/pf_n{int(num_nodes)}_i{int(send_interval)}_r{run}.json', 'w') as f:
        json.dump(data, f, indent=4, sort_keys=True)
        os.remove(f'./res/v{v}/pf_n{int(num_nodes)}_i{int(send_interval)}_r{run}.csv')


if __name__ == "__main__":
    # Usage python3 parse_results.py <version> <num_nodes> <run>
    v = sys.argv[1] 
    num_nodes = int(sys.argv[2])
    send_interval = int(sys.argv[3])
    run = int(sys.argv[4])

    study_name = f'./res/v{v}_parsed'
    os.makedirs(f'{study_name}', exist_ok=True)

    main(v, num_nodes, send_interval, run)
