import sys, os, json, math, random
import pandas as pd
import polars as pl
import numpy as np
import time
import argparse
from google.cloud import bigquery

def persist_df(df, table_name, backoff_window = 10):
    try:
        client = bigquery.Client(project="phd-data-414113")
        table = client.get_table(f"nc_rdf_v1.{table_name}")
        if len(table.schema) == 0:
            job_config = bigquery.LoadJobConfig(
                autodetect=True,
                write_disposition="WRITE_APPEND",
            )
            job = client.load_table_from_dataframe(
                df, 
                f"nc_rdf_v1.{table_name}",
                job_config=job_config
            )
            print(job.result())
        else:
            client.insert_rows_from_dataframe(
                table,
                df, 
            )
    except Exception as e:
        print(e)
        print('Error, retrying...')
        if backoff_window > 1000:
            return

        time.sleep(random.uniform(1, backoff_window))
        persist_df(df, table_name, backoff_window * 2)

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
        pl.col("pL").last(),
        pl.col("num_receivers").last(),
        pl.col('pos_x').last().alias('pos_x'),
        pl.col('pos_y').last().alias('pos_y'),
        (pl.Expr.pow(pl.col('pos_x') - pl.col('pos_x').last(), 2) + pl.Expr.pow(pl.col('pos_y') - pl.col('pos_y').last(), 2) <= pl.lit(509.8 **2)).sum().alias('num_possible_receivers')
    ])

    print(enhanced_sent_events)
    reception_rates = enhanced_sent_events.select([(pl.col('num_receivers') / (pl.col('num_possible_receivers')-1)).alias('pdr')])
    estimated_loss_rate = np.array(enhanced_sent_events['pL'])
    reception_rates = np.array(reception_rates)[0]
    pos_x = np.array(enhanced_sent_events['pos_x'])
    pos_y = np.array(enhanced_sent_events['pos_y'])

    return (reception_rates, estimated_loss_rate, pos_x, pos_y)

def main(args):
    v = args.v
    event_log = args.events_file_name
    position_log = args.course_file_name
    events = pl.read_csv(f'./res/v{v}/{event_log}.csv')
    positions = pl.read_csv(f'./res/v{v}/{position_log}.csv', dtypes = [pl.Float64, pl.Int64, pl.Float32, pl.Float32, pl.Float32])
    (reception_rates, estimated_loss_rate, pos_x, pos_y) = get_reception_rate(events, positions)
    loss_rate = pd.DataFrame({
        'pos_x': pos_x,
        'pos_y': pos_y,
        'reception_rates': reception_rates,
        'estimated_loss_rate': estimated_loss_rate
    })

    loss_rate.to_csv(f'./res/v{v}_parsed/loss_rate_{event_log}.csv')
    # enriched_events = events.with_columns([
    #     pl.lit(args.v).alias('v'),
    #     pl.lit(args.i).alias('i'),
    #     pl.lit(args.q).alias('q'),
    #     pl.lit(args.run).alias('run'),
    #     pl.lit(args.num_nodes).alias('num_nodes'),
    #     pl.lit(args.min_gain).alias('min_gain'),
    #     pl.lit(args.p_L).alias('p_L'),
    # ])
    # persist_df(enriched_events.to_pandas(), 'events')
    # forwarding_options = pl.read_csv(f'./res/v{v}/fwd_options_{event_log}.csv')
    # enriched_forwarding_options = forwarding_options.with_columns([
    #     pl.lit(args.v).alias('v'),
    #     pl.lit(args.i).alias('i'),
    #     pl.lit(args.q).alias('q'),
    #     pl.lit(args.run).alias('run'),
    #     pl.lit(args.num_nodes).alias('num_nodes'),
    #     pl.lit(args.min_gain).alias('min_gain'),
    #     pl.lit(args.p_L).alias('p_L'),
    # ])
    # persist_df(enriched_forwarding_options.to_pandas(), 'forwarding_options')



if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument("--start_time", type=float)
    parser.add_argument("--events_file_name")
    parser.add_argument("--course_file_name")
    parser.add_argument("--v")
    parser.add_argument("--i", type=int)
    parser.add_argument("--q", type=int)
    parser.add_argument("--run", type=int)
    parser.add_argument("--num_nodes", type=int)
    parser.add_argument("--min_gain", type=int)
    parser.add_argument("--p_L", type=int)

    args = parser.parse_args()

    study_name = f'./res/v{args.v}_parsed'
    os.makedirs(f'{study_name}', exist_ok=True)

    main(args)
