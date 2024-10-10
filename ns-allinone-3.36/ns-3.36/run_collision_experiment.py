import sys, os, math
import subprocess
import numpy as np

def get_params(run_idx = 0):
    params = []

    A = (0.509 * 2.5)**2 * math.pi

    for run in range(400, 500):
        for txd in range(100, 10100, 100):
            for num_nodes in [61]:
                send_interval = num_nodes / (A * txd)
                params.append({'run': run, 'num_nodes': num_nodes, 'send_interval': send_interval, 'txd': txd})

    print(f'running {run_idx} of {len(params)}')
    return params[run_idx]

if __name__ == '__main__':
    run_command = 'collision-rate'
    run_idx = int(sys.argv[1])
    params = get_params(run_idx)
    v = 400

    study_name = f'./res/v{v}'
    os.makedirs(f'{study_name}', exist_ok=True)

    num_nodes = params.get('num_nodes')
    send_interval = params.get('send_interval')
    run = params.get('run')
    txd = params.get('txd')

    command = ['./ns3', 'run', run_command, '--']
    command.append(f'--interval={send_interval}')
    command.append(f'--numNodes={num_nodes}')
    command.append(f'--seed={run}')
    command.append(f'--txd={txd}')
    command.append(f'--v={v}')

    print(command)
    subprocess.run(command)
    subprocess.run(['python3', 'analysis_scripts/parse_collision_results_v2.py', f'{v}', f'collision_n{num_nodes}_txd{txd}_r{run}'])