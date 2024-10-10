import sys, os
import json
import pandas as pd
import numpy as np
import time
import matplotlib.pyplot as plt

COMM_RANGE = 509 # m
T_MAX = 180000 # ms
T_RES = 20

def interpolate_aoi(new_ts, aoi_ts):
    new_entries = []

    for ts in new_ts:
        smaller_entries = [x for x in aoi_ts if x[0] <= ts]
        larger_entries = [x for x in aoi_ts if x[0] > ts]

        if (len(smaller_entries) > 0) and (len(larger_entries) > 0):
            prev_entry = smaller_entries[-1]
            next_entry = larger_entries[0]

            new_aoi = prev_entry[1] + (next_entry[1] - prev_entry[1]) * (ts - prev_entry[0]) / (next_entry[0] - prev_entry[0])

            new_entries.append((ts, new_aoi))

    new_entries = np.array(new_entries)

    if len(new_entries) == 0:
        return aoi_ts

    aoi_ts = np.concatenate((new_entries, aoi_ts))

    result = sorted(aoi_ts, key=lambda tup: tup[0])
    return result


def get_dissemation_rate_by_node(rcvd_events, nodeId):
    srcIds = rcvd_events[rcvd_events['nodeId'] == nodeId].src.unique()
    return len(srcIds)

def get_avg_dissemination_rate(rcvd_events):
    nodes = rcvd_events.nodeId.unique()
    dissemination_rates = np.array([])

    for nodeId in nodes:
        diss_rate = get_dissemation_rate_by_node(rcvd_events, nodeId) / (nodes.size -1)
        dissemination_rates = np.append(dissemination_rates, [diss_rate])
    
    return np.mean(dissemination_rates)

def get_aoi_ts_sections_by_max_dist(rcvd_events, srcId, destId, positions, R):
    dist_ts = get_dist_ts(positions, srcId, destId)
    aoi_ts = get_aoi_ts(rcvd_events, destId, srcId)

    if len(aoi_ts) == 0:
        return []
    
    dist_ts = [e for e in dist_ts if e[0] >= aoi_ts[0][0]]
    ts = [x[0] for x in aoi_ts]
    distances = [x[1] for x in dist_ts]
    sections = []

    if len(distances) == 0 or min(distances) > R:
        return []

    if len(aoi_ts) == 0:
        return []
    
    section_starts = []
    section_ends = []

    if dist_ts[0][1] <= R:
        section_starts.append(dist_ts[0][0])


    for i in range(1, len(dist_ts)):
        if (dist_ts[i-1][1] > R) and (dist_ts[i][1] <= R):
            section_starts.append(dist_ts[i][0])

    for i in range(0, len(dist_ts)-1):
        if (dist_ts[i][1] <= R) and (dist_ts[i+1][1] > R):
            section_ends.append(dist_ts[i][0]) # Was i+1 previously

    section_ends.append(dist_ts[-1][0]) # was T_MAX prev

    new_ts = section_starts + section_ends
    aoi_ts = interpolate_aoi(new_ts, aoi_ts)

    # plt.plot([x[0] for x in aoi_ts],[x[1] for x in aoi_ts])
    # plt.show()

    for i in range(len(section_starts)):
        start = section_starts[i]
        end = section_ends[i]
        ts = [x for x in aoi_ts if x[0] >= start and x[0] <= end]
        sections.append(np.array(ts))

    # print(section_starts, section_ends, sections)
    return sections

def get_aoi_ts_sections_by_max_hop(rcvd_events, srcId, destId, maxHops):
    hops_ts = get_num_hops_ts(rcvd_events, destId, srcId)
    aoi_ts = get_aoi_ts(rcvd_events, destId, srcId)
    sections = []

    if len(hops_ts) == 0:
        return []
    
    section_starts = []
    section_ends = []

    if hops_ts[0][1] <= maxHops:
        section_starts.append(hops_ts[0][0])


    for i in range(1, len(hops_ts)):
        if (hops_ts[i-1][1] > maxHops) and (hops_ts[i][1] <= maxHops):
            section_starts.append(hops_ts[i][0])

    for i in range(0, len(hops_ts)-1):
        if (hops_ts[i][1] <= maxHops) and (hops_ts[i+1][1] > maxHops):
            section_ends.append(hops_ts[i][0]) # Was i+1 previously

    section_ends.append(hops_ts[-1][0]) # was T_MAX prev

    for i in range(len(section_starts)):
        start = section_starts[i]
        end = section_ends[i]
        ts = [x for x in aoi_ts if x[0] >= start and x[0] <= end]
        sections.append(np.array(ts))

    return sections


def get_num_hops_ts(data, nodeId, srcId):
    data = data[(data['src'] == srcId) & (data['nodeId'] == nodeId)]
    ts = data['timestamp'].to_numpy() * 1000
    num_hops = data['numHops'].to_numpy()

    return list(zip(ts, num_hops))

def get_dist_ts(positions, srcId, destId):
    src_path = positions[positions['nodeId'] == srcId]
    dest_path = positions[positions['nodeId'] == destId]

    src_x = src_path['pos_x'].to_numpy()
    src_y = src_path['pos_y'].to_numpy()    
    dest_x = dest_path['pos_x'].to_numpy()
    dest_y = dest_path['pos_y'].to_numpy()

    ts = src_path['timestamp'].to_numpy() * 1000
    dist = np.sqrt(np.power(src_x - dest_x, 2) + np.power(src_y - dest_y, 2))
    return list(zip(ts, dist))

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

    if len(ts) > 0:
        ts = np.append(ts, [T_MAX])
        delay = np.append(delay, [0])

    rcvd_aoi = list(zip(ts, delay))
    result = []
    for entry in rcvd_aoi:
        if len(result) >= 1:
            prev_val = result[-1]
            age = entry[0] - prev_val[0]
            result.append((entry[0], prev_val[1] + age))
            if entry[1] < (prev_val[1] + age):
                result.append(entry)
        else:
            result.append(entry)

        
            

    return np.array(result)

def get_avg_aoi(aoi_ts):
    if len(aoi_ts) <= 1:
        return -1

    (x,y) = list(zip(*aoi_ts))
    total_time = aoi_ts[-1][0] - aoi_ts[0][0]
    return np.trapz(y, x) / total_time


def extract_stats(data, v, file_name, stats={}):
    forwarded_events = data[(data['eventType'] == 'PktFwd')]
    rcvd_events = data[(data['eventType'] == 'PktRcvd')]
    sent_events = data[(data['eventType'] == 'PktSent')]

    new_stats = {
        'num_sent': int(sent_events.size),
        'num_rcvd': int(rcvd_events.size),
        'num_fwd': int(forwarded_events.size),
        'avg_dissemination_rate': get_avg_dissemination_rate(rcvd_events)
    }
    stats.update(new_stats)

    with open(f'./res/v{v}_parsed/summary_{file_name}.json', 'w') as f:
        json.dump(stats, f, indent=4, sort_keys=True)

def extract_num_forwarded_per_user_dist(data, v, file_name):
    forwarded_events = data[(data['eventType'] == 'PktFwd')]
    forwarded_events_by_user = forwarded_events.groupby('nodeId').size().values
    with open(f'./res/v{v}_parsed/{file_name}_fwd_by_user_dist.json', 'w') as f:
        json.dump(forwarded_events_by_user.tolist(), f)

def extract_num_received_per_user_dist(data, v, file_name):
    rcvd_events = data[(data['eventType'] == 'PktRcvd')]
    rcvd_events_by_user = rcvd_events.groupby('nodeId').size().values
    with open(f'./res/v{v}_parsed/{file_name}_rcvd_by_user_dist.json', 'w') as f:
        json.dump(rcvd_events_by_user.tolist(), f)

def extract_num_forwarded_per_seqno_dist(data, v, file_name):
    forwarded_events = data[(data['eventType'] == 'PktFwd')]
    forwarded_events_by_user = forwarded_events.groupby('seqNo').size().values
    with open(f'./res/v{v}_parsed/{file_name}_fwd_by_user_seqno_dist.json', 'w') as f:
        json.dump(forwarded_events_by_user.tolist(), f)

def extract_num_received_per_seqno_dist(data, v, file_name):
    rcvd_events = data[(data['eventType'] == 'PktRcvd')]
    rcvd_events_by_user = rcvd_events.groupby('seqNo').size().values
    with open(f'./res/v{v}_parsed/{file_name}_rcvd_by_seqno_dist.json', 'w') as f:
        json.dump(rcvd_events_by_user.tolist(), f)

def extract_aoi_distribution(data, v, file_name):
    rcvd_events = data[(data['eventType'] == 'PktRcvd')]
    bin_ends = np.array(np.arange(T_RES, T_MAX, T_RES))
    bins = np.zeros(len(bin_ends))
    nodeIds = sorted(data.nodeId.unique())
    for srcId in nodeIds:
        for destId in nodeIds:
            if srcId != destId:
                aoi_ts = get_aoi_ts(rcvd_events, srcId, destId)
                if len(aoi_ts) > 0:
                    # extract rects
                    aoi_values = aoi_ts[:,1]
                    rect_starts = aoi_values[0:-2]
                    rect_ends = aoi_values[1:-1]
                    rects = np.array(list(zip(rect_starts,rect_ends)))
                    # sum rects on hist
                    for rect in rects:
                        bins += np.logical_and((bin_ends >= rect[0]),(bin_ends < rect[1]))
    
    t = bin_ends - (T_RES / 2)
    p = bins / np.sum(bins)
    data = {
        't': t.tolist(),
        'p': p.tolist()
    }
    idx = np.argmax(np.cumsum(p) >= 0.95)
    aoi_95_percentile = t[idx]
    avg_aoi = np.sum(t * p)

    with open(f'./res/v{v}_parsed/{file_name}_aoi_dist.json', 'w') as f:
        json.dump(data, f)

    return (avg_aoi, aoi_95_percentile)

def extract_aoi_distribution_by_max_dist(data, v, file_name, positions, R):
    rcvd_events = data[(data['eventType'] == 'PktRcvd')]
    bin_ends = np.array(np.arange(T_RES, T_MAX, T_RES))
    bins = np.zeros(len(bin_ends))
    nodeIds = sorted(data.nodeId.unique())
    for srcId in nodeIds:
        for destId in nodeIds:
            if srcId != destId:
                aoi_ts_sections = get_aoi_ts_sections_by_max_dist(rcvd_events, srcId, destId, positions, R)
                for aoi_ts in aoi_ts_sections:
                    if len(aoi_ts) > 0:
                        # extract rects
                        aoi_values = aoi_ts[:,1]
                        rect_starts = aoi_values[0:-2]
                        rect_ends = aoi_values[1:-1]
                        rects = np.array(list(zip(rect_starts,rect_ends)))
                        # sum rects on hist
                        for rect in rects:
                            bins += np.logical_and((bin_ends >= rect[0]),(bin_ends < rect[1]))
    
    t = bin_ends - (T_RES / 2)
    p = bins / np.sum(bins)
    data = {
        't': t.tolist(),
        'p': p.tolist()
    }

    excess_probability_1_hop = 1 - np.sum(p[t <= 500])

    with open(f'./res/v{v}_parsed/{file_name}_aoi_dist_r{int(R/COMM_RANGE)}.json', 'w') as f:
        json.dump(data, f)
            
    return excess_probability_1_hop

def extract_aoi_distribution_by_max_hop(data, v, file_name, maxHops):
    rcvd_events = data[(data['eventType'] == 'PktRcvd')]
    bin_ends = np.array(np.arange(T_RES, T_MAX, T_RES))
    bins = np.zeros(len(bin_ends))
    nodeIds = sorted(data.nodeId.unique())
    for srcId in nodeIds:
        for destId in nodeIds:
            if srcId != destId:
                aoi_ts_sections = get_aoi_ts_sections_by_max_hop(rcvd_events, srcId, destId, maxHops)
                for aoi_ts in aoi_ts_sections:
                    if len(aoi_ts) > 0:
                        # extract rects
                        aoi_values = aoi_ts[:,1]
                        rect_starts = aoi_values[0:-2]
                        rect_ends = aoi_values[1:-1]
                        rects = np.array(list(zip(rect_starts,rect_ends)))
                        # sum rects on hist
                        for rect in rects:
                            bins += np.logical_and((bin_ends >= rect[0]),(bin_ends < rect[1]))
    
    t = bin_ends - (T_RES / 2)
    p = bins / np.sum(bins)
    data = {
        't': t.tolist(),
        'p': p.tolist()
    }

    excess_probability_1_hop = 1 - np.sum(p[t <= 500])

    with open(f'./res/v{v}_parsed/{file_name}_aoi_dist_h{maxHops}.json', 'w') as f:
        json.dump(data, f)
            
    return excess_probability_1_hop

def delete_file(v, file_name):
    os.remove(f'./res/v{v}/{file_name}.csv')
    os.remove(f'./res/v{v}/course_{file_name}.csv')



def main(v, file_name):
    data = pd.read_csv(f'./res/v{v}/{file_name}.csv')
    positions = pd.read_csv(f'./res/v{v}/course_{file_name}.csv')
    extract_num_forwarded_per_user_dist(data, v, file_name)
    extract_num_received_per_user_dist(data, v, file_name)
    extract_num_forwarded_per_seqno_dist(data, v, file_name)
    extract_num_received_per_seqno_dist(data, v, file_name)
    (avg_aoi, aoi_95_percentile) = extract_aoi_distribution(data, v, file_name)
    # excess_probability_1_hop = extract_aoi_distribution_by_max_hop(data, v, file_name, 0)
    excess_probability_1_R = extract_aoi_distribution_by_max_dist(data, v, file_name, positions, COMM_RANGE)
    # excess_probability_2_R = extract_aoi_distribution_by_max_dist(data, v, file_name, positions, 2 * COMM_RANGE)
    # excess_probability_2_hop = extract_aoi_distribution_by_max_hop(data, file_name, 1)

    stats = {
        'avg_aoi': avg_aoi,
        'aoi_95_percentile': aoi_95_percentile,
        'excess_probability_1_R': excess_probability_1_R
    }
    extract_stats(data, v, file_name, stats)
    delete_file(v, file_name)


if __name__ == "__main__":
    start_time = time.time()
    # Usage python3 parse_results.py <version> <num_nodes> <run>
    v = sys.argv[1]
    file_name = sys.argv[2]

    study_name = f'./res/v{v}_parsed'
    os.makedirs(f'{study_name}', exist_ok=True)

    main(v, file_name)

    duration = time.time() - start_time
    print(f'Done. Duration: {duration}')
