import sys, os, json, math
import pandas as pd
import numpy as np
import time
import matplotlib.pyplot as plt

def get_peak_aoi_by_distance(events, positions):
    events = events[events['timestamp'] >= 5.0]
    event_times = events.loc[:,['timestamp']].drop_duplicates()
    positions = positions.groupby('nodeId', group_keys=False).apply(lambda group: pd.concat([group, event_times]).sort_values('timestamp').reset_index(drop=True).interpolate())
    events = pd.merge(events, positions, how='inner', on=['nodeId','timestamp'])
    send_events = events.query('eventType == "PktSent"')
    nodeIds = np.sort(events['nodeId'].unique())

    peak_aoi = pd.DataFrame()

    for sender in nodeIds:
        for receiver in nodeIds:
            if sender != receiver:
                print(sender, receiver)
                receive_events = events.query(f'eventType == "PktRcvd" & src == {sender} & nodeId == {receiver}')
                receive_times = np.array(receive_events['timestamp'])
                delays = np.array(receive_events['delay'])[0:-1]
                # iat = receive_times[1:0] - receive_times[0:-1]
                iat = np.diff(receive_times)
                aoi_peaks = iat + delays / 1000
                sender_pos = positions[positions['timestamp'].isin(receive_times) & (positions['nodeId'] == sender)]
                receiver_pos = positions[positions['timestamp'].isin(receive_times) & (positions['nodeId'] == receiver)]
                dist = np.array(np.sqrt(np.power(sender_pos['pos_x'] - receiver_pos['pos_x'],2) + np.power(sender_pos['pos_y'] - receiver_pos['pos_y'],2)))
                #print(dist)
                peak_aoi = pd.concat([peak_aoi, pd.DataFrame({'dist': dist[1:], 'aoi': aoi_peaks})])   

    return (peak_aoi, events)


def main(v, event_log, position_log):

    events = pd.read_csv(f'./res/v{v}/{event_log}.csv')
    positions = pd.read_csv(f'./res/v{v}/{position_log}.csv')

    (peak_aoi, enhanced_events) = get_peak_aoi_by_distance(events, positions)
    peak_aoi.to_csv(f'./res/v{v}_parsed/peak_aoi_{event_log}.csv')
    enhanced_events.to_csv(f'./res/v{v}/{event_log}_enhanced.csv')


if __name__ == "__main__":
    start_time = time.time()
    v = sys.argv[1]
    event_log = sys.argv[2]
    course_log = sys.argv[3]

    study_name = f'./res/v{v}_parsed'
    os.makedirs(f'{study_name}', exist_ok=True)

    main(v, event_log, course_log)

    duration = time.time() - start_time
    print(f'Details. Duration: {duration}')
