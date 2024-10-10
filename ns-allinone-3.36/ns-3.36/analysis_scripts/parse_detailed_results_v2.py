import sys, os, json, math
import pandas as pd
import polars as pl
import numpy as np
import time

def get_reception_rate(events, positions):
    event_times = events['timestamp'].unique()
    positions = positions.groupby('nodeId').apply(lambda group: pl.concat([group, pl.DataFrame(event_times)], how="diagonal").sort('timestamp').interpolate())
    positions = positions.filter(~pl.col('pos_x').is_null() & pl.col('timestamp').is_in(event_times))
    events = events.join(positions, how='inner', on=['nodeId','timestamp'])
    sent_events = events.filter(pl.col('eventType') == "PktSent")
    received_events = events.filter((pl.col('eventType') == "PktRcvd") & (pl.col('numHops') == 0))
    received_count = received_events.groupby('seqNo').agg(pl.count().alias("num_receivers"))
    sent_events = sent_events.join(received_count, how='inner', on='seqNo')
    sent_events = events.filter(pl.col('eventType') == "PktSent")
    received_events = events.filter((pl.col('eventType') == "PktRcvd") & (pl.col('numHops') == 0))
    received_count = received_events.groupby('seqNo').agg(pl.count().alias("num_receivers"))
    sent_events = sent_events.join(received_count, how='inner', on='seqNo')
    positions_at_sent_time= positions.filter(pl.col('timestamp').is_in(sent_events['timestamp'].unique()))
    enhanced_sent_events = pl.concat([sent_events, positions_at_sent_time.with_columns(pl.lit('position').alias('eventType'))], how='diagonal').sort(['timestamp','eventType'], reverse=True)
    enhanced_sent_events = enhanced_sent_events.groupby('timestamp').agg([
        (pl.col('eventType') == 'position').sum().alias('posEvents'),
        (pl.col('eventType') == 'PktSent').sum().alias('sentEvent'),
        pl.col("num_receivers").last(),
        pl.col('pos_x').last().alias('pos_x'),
        pl.col('pos_y').last().alias('pos_y'),
        (pl.Expr.pow(pl.col('pos_x') - pl.col('pos_x').last(), 2) + pl.Expr.pow(pl.col('pos_y') - pl.col('pos_y').last(), 2) <= pl.lit(509.8 **2)).sum().alias('num_possible_receivers')
    ])
    reception_rates = enhanced_sent_events.select([(pl.col('num_receivers') / (pl.col('num_possible_receivers')-1)).alias('pdr')])
    reception_rates = np.array(reception_rates)[0]
    pos_x = np.array(enhanced_sent_events['pos_x'])
    pos_y = np.array(enhanced_sent_events['pos_y'])

    return (reception_rates, pos_x, pos_y)

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

    events = pl.read_csv(f'./res/v{v}/{event_log}.csv')
    positions = pl.read_csv(f'./res/v{v}/{position_log}.csv', dtypes = [pl.Float64, pl.Int64, pl.Float32, pl.Float32, pl.Float32])
    (reception_rates, pos_x, pos_y) = get_reception_rate(events, positions)
    loss_rate = pd.DataFrame({
        'pos_x': pos_x,
        'pos_y': pos_y,
        'reception_rates': reception_rates
    })

    loss_rate.to_csv(f'./res/v{v}_parsed/loss_rate_{event_log}.csv')


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
