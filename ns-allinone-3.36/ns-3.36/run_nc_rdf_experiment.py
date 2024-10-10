import sys, os, math, subprocess, time
import numpy as np

def get_params(run_idx = 0):
    params = []
    for run in range(0, 10):
        for num_nodes in reversed(np.arange(100, 1400, 25)):
            for (min_gain, p_L) in [(10, 0.0), (0, 0.39), (0, 2.0)]:
            #for (min_gain, p_L) in [(0, 0.1),(0, 0.2),(0, 0.3)]:
                q_opts = [2.0]
                if min_gain == 10:
                    q_opts.append(0.0)
                for q in q_opts:
                    for send_interval in [0.12]:
                        params.append({'run': run, 'num_nodes': num_nodes, 'send_interval': send_interval, 'q': q, 'min_gain': min_gain, 'p_L': p_L})

    print(f'running {run_idx} of {len(params)}')
    if run_idx >= len(params):
        return None
    return params[run_idx]

if __name__ == '__main__':
    start_time = time.time()
    run_command = 'rate-decay-flooding-nc' 
    run_idx = int(sys.argv[1])
    offset = int(sys.argv[2])
    params = get_params(run_idx + offset)
    v = 710

    if params != None:

        study_name = f'./res/v{v}'
        os.makedirs(f'{study_name}', exist_ok=True)

        num_nodes = params.get('num_nodes')
        send_interval = params.get('send_interval')
        min_gain = params.get('min_gain')
        run = params.get('run')
        q = params.get('q')
        p_L = params.get('p_L')
        density = 12
        warmup = 5
        A = num_nodes / density
        size = math.sqrt(A) * 1000
        simTime = size/33.3 + warmup
        #simTime = 180 + warmup

        res_file_name = f'kpi_rdf_n{num_nodes}_i{int(send_interval*1000)}_q{int(q*100)}_mg{int(min_gain * 100)}_pL{int(p_L * 100)}_r{run}'
        event_file_name = f'rdf_n{num_nodes}_i{int(send_interval*1000)}_q{int(q*100)}_mg{int(min_gain * 100)}_pL{int(p_L * 100)}_r{run}'
        course_file_name = f'course_rdf_n{num_nodes}_i{int(send_interval*1000)}_q{int(q*100)}_mg{int(min_gain * 100)}_pL{int(p_L * 100)}_r{run}'

        if not os.path.isfile(f'./res/v{v}_parsed/summary_{res_file_name}.json'):
            command = ['./ns3', 'run', run_command, '--']
            command.append(f'--interval={send_interval}')
            command.append(f'--numNodes={num_nodes}')
            command.append(f'--seed={run}')
            command.append(f'--decayFactor={q}')
            command.append(f'--minGain={min_gain}')
            command.append(f'--lossRate={p_L}')
            command.append(f'--v={v}')
            command.append(f'--size={size}')
            command.append(f'--simTime={simTime}')
            command.append('--speedMin=22.2')
            command.append('--speedMax=33.3')
            # command.append('--tracing')

            subprocess.run(command)
            #subprocess.run(['python3', 'analysis_scripts/parse_detailed_results_v2.py', f'{v}', event_file_name, course_file_name])
            subprocess.run([
                'python3', 
                'analysis_scripts/parse_results_v6.py', 
                f'--start_time={start_time}', 
                f'--result_file_name={res_file_name}',
                f'--v={v}',
                f'--i={int(send_interval*1000)}',
                f'--q={int(q*100)}',
                f'--run={run}',
                f'--num_nodes={num_nodes}',
                f'--p_L={int(p_L * 100)}',
                f'--min_gain={int(min_gain * 100)}'
            ])
