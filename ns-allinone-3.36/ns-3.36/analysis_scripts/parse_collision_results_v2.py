import sys, os
import json
import pandas as pd
import numpy as np
import time
import matplotlib.pyplot as plt

COMM_RANGE = 509 # m

def extract_stats(data, v, file_name, stats={}):
    forwarded_events = data[(data['eventType'] == 'PktFwd')]
    rcvd_events = data[(data['eventType'] == 'PktRcvd')]
    sent_events = data[(data['eventType'] == 'PktSent')]

    in_range_nodes = stats['in_range_nodes']

    num_sent = int(sent_events.size)
    num_rcvd = int(rcvd_events.size)

    new_stats = {
        'num_sent': num_sent,
        'num_rcvd': num_rcvd,
        'num_fwd': int(forwarded_events.size),
        'collision_rate': -1 if in_range_nodes <=1 else 1- num_rcvd / (num_sent * (in_range_nodes -1)),
    }
    stats.update(new_stats)

    with open(f'./res/v{v}_parsed/summary_{file_name}.json', 'w') as f:
        json.dump(stats, f, indent=4, sort_keys=True)

def delete_file(v, file_name):
    os.remove(f'./res/v{v}/{file_name}.csv')
    os.remove(f'./res/v{v}/course_{file_name}.csv')

def get_in_range_nodes(positions):
    xx = positions['pos_x'].to_numpy()
    yy = positions['pos_y'].to_numpy()   
    dist = np.sqrt(np.power(xx, 2) + np.power(yy, 2))
    positions['dist'] = dist

    in_range_nodes = positions[positions['dist'] <= COMM_RANGE / 2]
    return in_range_nodes['nodeId'].to_list()

def main(v, file_name):
    data = pd.read_csv(f'./res/v{v}/{file_name}.csv')
    positions = pd.read_csv(f'./res/v{v}/course_{file_name}.csv')


    in_range_nodes = get_in_range_nodes(positions)

    print(in_range_nodes)

    data = data[data['nodeId'].isin(in_range_nodes) & data['src'].isin(in_range_nodes)]

    stats = {
        # 'avg_aoi': avg_aoi,
        # 'aoi_95_percentile': aoi_95_percentile,
        'in_range_nodes': len(in_range_nodes)
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
