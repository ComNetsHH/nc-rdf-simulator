import pandas as pd
import numpy as np
import matplotlib.pyplot as plt


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

def main(num_nodes, run=1):
    data = pd.read_csv(f'./res/v4/pf_n{int(num_nodes)}_r{run}.csv')
    forwarded_events = data[(data['eventType'] == 'PktFwd')]

    aoi = []
    dissemination_rates = []
    nodes = forwarded_events.nodeId.unique()

    for nodeId in nodes:
        res = get_avg_aoi_by_node(forwarded_events, nodeId)
        dissemination_rates.append(get_dissemation_ratio(forwarded_events, nodeId))
        aoi += res

    print('Num Nodes', num_nodes)
    print('Avg AoI', np.mean(aoi))
    print('Avg Dissemination Rate', np.mean(dissemination_rates) / (num_nodes -1))
    # plt.hist(aoi, np.arange(0, 100000, 20), cumulative=True, density=True)

    avg_diss_rate = np.mean(dissemination_rates) / (num_nodes -1)
    avg_aoi = np.mean(aoi)

    # plt.show()
    return (avg_aoi, avg_diss_rate)

if __name__ == '__main__':
    res = []

    nn = range(10, 120)
    runs = 8

    for i in nn:
        aois = []
        disses = []
        for r in range(1, runs+1):
            (aoi, diss) = main(i, r)
            aois.append(aoi)
            disses.append(diss)

        res.append((np.mean(aois), np.mean(disses)))

    plt.plot(nn, [x[0] for x in res])
    plt.title('AoI')
    plt.show()

    plt.plot(nn, [x[1] for x in res])
    plt.title('Diss Rate')
    plt.show()


