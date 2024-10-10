import sys, os, json, time, random
import polars as pl
import numpy as np
import argparse
from google.cloud import bigquery

def persist_df(df, backoff_window = 10):
    try:
        client = bigquery.Client(project="phd-data-414113")
        table = client.get_table("nc_rdf_v1.sim_results")
        if len(table.schema) == 0:
            job_config = bigquery.LoadJobConfig(
                autodetect=True,
                write_disposition="WRITE_APPEND",
            )
            job = client.load_table_from_dataframe(
                df, 
                "nc_rdf_v1.sim_results",
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
        persist_df(df, backoff_window * 2)

def main(args):
    data = pl.read_csv(f'./res/v{args.v}/{args.result_file_name}.csv')
    avg_loss_rate = -1
    try:
        event_log = args.result_file_name.replace('kpi_','')
        loss_rates = pl.read_csv(f'./res/v{args.v}_parsed/loss_rate_{event_log}.csv')
        avg_loss_rate = 1 - loss_rates['reception_rates'].mean()

    except:
        pass



    duration = time.time() - args.start_time

    stats = {
        'num_sent': int(data['sumSent'][0]),
        'num_coded': int(data['sumSentCoded'][0]),
        'num_rcvd': int(data['sumRcvd'][0]),
        'num_fwd': int(data['sumFwd'][0]),
        'excess_probability_1_R_peak': float(data['pe500'][0]),
        'avg_dissemination_rate': float(data['pd'][0]),
        'avg_loss_rate': avg_loss_rate,
        'duration': duration,
        'v': args.v,
        'i': args.i,
        'q': args.q,
        'run': args.run,
        'num_nodes': args.num_nodes,
        'min_gain': args.min_gain,
        'p_L': args.p_L
    }

    with open(f'./res/v{args.v}_parsed/summary_{args.result_file_name}.json', 'w') as f:
        json.dump(stats, f, indent=4, sort_keys=True)

    df = pl.from_dict(stats)
    persist_df(df.to_pandas())


if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument("--start_time", type=float)
    parser.add_argument("--result_file_name")
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
