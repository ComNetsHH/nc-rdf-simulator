import sys, os
import json
import pandas as pd
import numpy as np
import time
import matplotlib.pyplot as plt
import marshal

def main(v, file_name):
    data = pd.read_csv(f'./res/v{v}/{file_name}.csv')
    avg_loss_rate = -1
    try:
        event_log = file_name.replace('kpi_','')
        loss_rates = pd.read_csv(f'./res/v{v}_parsed/loss_rate_{event_log}.csv')
        avg_loss_rate = 1 - loss_rates['reception_rates'].mean()

    except:
        print('No loss rate found')

    stats = {
        'num_sent': int(data['sumSent'][0]),
        'num_coded': int(data['sumSentCoded'][0]),
        'num_rcvd': int(data['sumRcvd'][0]),
        'num_fwd': int(data['sumFwd'][0]),
        'excess_probability_1_R_peak': float(data['pe500'][0]),
        'avg_dissemination_rate': float(data['pd'][0]),
        'avg_loss_rate': avg_loss_rate
    }

    with open(f'./res/v{v}_parsed/summary_{file_name}.json', 'w') as f:
        json.dump(stats, f, indent=4, sort_keys=True)


if __name__ == "__main__":
    start_time = time.time()
    # Usage python3 parse_results.py <version> <num_nodes> <run>
    v = sys.argv[1]
    file_name = sys.argv[2]

    study_name = f'./res/v{v}_parsed'
    os.makedirs(f'{study_name}', exist_ok=True)

    main(v, file_name)

    duration = time.time() - start_time
    print(f'KPIs. Duration: {duration}')
