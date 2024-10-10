import sys, os
import json
import pandas as pd
import numpy as np
from scipy.interpolate import interp1d  
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

def get_aoi_samples(aoi_ts, sample_width = 100):
    if len(aoi_ts) <= 1:
        return []

    (x,y) = list(zip(*aoi_ts))

    F = interp1d(x,y,fill_value='extrapolate') 
    x_new = np.arange(x[0], x[-1], sample_width)
    samples = F(x_new)

    return samples

def main(v, file):
    data = pd.read_csv(f'res/v{v}/{file}.csv')
    rcvd_events = data[(data['eventType'] == 'PktRcvd') & (data['numHops'] == 0)]

    nodes = sorted(rcvd_events.nodeId.unique())

    aoi_samples = np.array([])

    for destId in nodes:
        print(destId)
        for srcId in nodes:
            if not (srcId == destId):
                # src_dest_events = rcvd_events[(rcvd_events['nodeId'] == destId) & (rcvd_events['src'] == srcId)]
                # print(srcId, destId, len(src_dest_events))
                ts = get_aoi_ts(rcvd_events, destId, srcId)
                samples = get_aoi_samples(ts, 25)
                aoi_samples = np.concatenate((aoi_samples, samples))

    
    heights, bins = np.histogram(aoi_samples, bins=np.arange(0, 3000, 25))
    bin_centers = (bins[:-1] + bins[1:]) / 2

    heights = heights/sum(heights)

    sum_500 = np.sum(heights[bin_centers < 500])

    fig = plt.figure()
    plt.bar(bin_centers,heights,width=(max(bins) - min(bins)) /len(bins), label=f'P(AoI > 500)={1-sum_500:.2f}')
    plt.legend()
    plt.axis([0, 3000, 0, 0.1])
    plt.xlabel('AoI [ms]')
    plt.ylabel('Probability')
    fig.savefig(f'./figures/aoi_dist_1hop_{file}.png', dpi=500, bbox_inches='tight', pad_inches=0.01)
    fig.savefig(f'./figures/aoi_dist_1hop_{file}.pdf', dpi=500, bbox_inches='tight', pad_inches=0.01)


if __name__ == "__main__":
    # # Usage python3 parse_results.py <version> <num_nodes> <run>
    # v = sys.argv[1] 
    # num_nodes = int(sys.argv[2])
    # send_interval = int(sys.argv[3])
    # run = int(sys.argv[4])

    # study_name = f'./res/v{v}_parsed'
    # os.makedirs(f'{study_name}', exist_ok=True)

    main(7, f'sf_n200_i300_p100_r0')
    main(7, f'sf_n200_i300_p80_r0')
    main(7, f'rdf_n200_i300_q100_r0')
    main(7, f'rdf_n200_i300_q150_r0')
