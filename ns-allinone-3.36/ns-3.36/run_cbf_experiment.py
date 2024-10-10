import sys
import subprocess

def get_params(run_idx = 0):
    params = []

    for run in range(0, 10):
        for send_interval in [0.3, 0.5, 1]:
            for num_nodes in range(20, 500, 20):
                params.append({'run': run, 'num_nodes': num_nodes, 'send_interval': send_interval})

    print(f'running {run_idx} of {len(params)}')
    return params[run_idx]

if __name__ == '__main__':
    run_command = sys.argv[1] 
    run_idx = int(sys.argv[2])
    params = get_params(run_idx)

    command = ['./ns3', 'run', run_command, '--']
    command.append(f'--interval={params.get("send_interval")}')
    command.append(f'--numNodes={params.get("num_nodes")}')
    command.append(f'--seed={params.get("run")}')

    print(command)
    subprocess.run(command)
    subprocess.run(['python3', 'analysis_scripts/parse_results_cbf.py', '7', f'{int(params.get("num_nodes"))}', f'{int(params.get("send_interval")* 1000)}', f'{int(params.get("run"))}'])