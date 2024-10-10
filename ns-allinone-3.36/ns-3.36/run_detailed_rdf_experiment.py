import sys, os, math
import subprocess

def get_params(run_idx = 0):
    params = []

    for num_nodes in [25, 50, 100]:
        for run in range(0, 20):
            for q in [2, 0]:
                for send_interval in [0.06, 0.09, 0.12, 0.15, 0.18, 0.21]:
                    params.append({'run': run, 'num_nodes': num_nodes, 'send_interval': send_interval, 'q': q})

    print(f'running {run_idx} of {len(params)}')
    if run_idx >= len(params):
        return None
    return params[run_idx]

if __name__ == '__main__':
    run_command = 'rate-decay-flooding' 
    run_idx = int(sys.argv[1])
    offset = int(sys.argv[2])
    params = get_params(run_idx + offset)
    v = 48

    if params != None:

        study_name = f'./res/v{v}'
        os.makedirs(f'{study_name}', exist_ok=True)

        num_nodes = params.get('num_nodes')
        send_interval = params.get('send_interval')
        run = params.get('run')
        q = params.get('q')
        density = 12
        warmup = 5
        A = num_nodes / density
        size = math.sqrt(A) * 1000
        simTime = 55#size/33.3 + warmup
        #simTime = 180 + warmup

        res_file_name = f'kpi_rdf_n{num_nodes}_i{int(send_interval*1000)}_q{int(q*100)}_r{run}'
        event_file_name = f'rdf_n{num_nodes}_i{int(send_interval*1000)}_q{int(q*100)}_r{run}'
        course_file_name = f'course_rdf_n{num_nodes}_i{int(send_interval*1000)}_q{int(q*100)}_r{run}'

        if not os.path.isfile(f'./res/v{v}_parsed/summary_{res_file_name}.json'):
            command = ['./ns3', 'run', run_command, '--']
            command.append(f'--interval={send_interval}')
            command.append(f'--numNodes={num_nodes}')
            command.append(f'--seed={run}')
            command.append(f'--decayFactor={q}')
            command.append(f'--v={v}')
            command.append(f'--size={size}')
            command.append(f'--simTime={simTime}')
            command.append('--speedMin=22.2')
            command.append('--speedMax=33.3')
            command.append('--tracing')

            subprocess.run(command)
            subprocess.run(['python3', 'analysis_scripts/parse_detailed_results_v2.py', f'{v}', event_file_name, course_file_name])
            subprocess.run(['python3', 'analysis_scripts/parse_results_v4.py', f'{v}', res_file_name])
