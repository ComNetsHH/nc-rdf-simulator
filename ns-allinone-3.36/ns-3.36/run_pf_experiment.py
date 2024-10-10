import sys, os
import subprocess

def get_params(run_idx = 0):
    params = []

    for run in range(0, 5):
        for num_nodes in range(20, 500, 20):
            for send_interval in [0.06, 0.12, 0.24]:
                params.append({'run': run, 'num_nodes': num_nodes, 'send_interval': send_interval})

    print(f'running {run_idx} of {len(params)}')
    return params[run_idx]

if __name__ == '__main__':
    run_command = sys.argv[1] 
    run_idx = int(sys.argv[2])
    params = get_params(run_idx)
    v = 19

    study_name = f'./res/v{v}'
    os.makedirs(f'{study_name}', exist_ok=True)

    num_nodes = params.get('num_nodes')
    send_interval = params.get('send_interval')
    run = params.get('run')

    command = ['./ns3', 'run', run_command, '--']
    command.append(f'--interval={send_interval}')
    command.append(f'--numNodes={num_nodes}')
    command.append(f'--seed={run}')
    command.append(f'--v={v}')
    command.append('--size=5000')

    print(command)
    subprocess.run(command)
    subprocess.run(['python3', 'analysis_scripts/parse_results_v2.py', f'{v}', f'pf_n{num_nodes}_i{int(send_interval*1000)}_r{run}'])