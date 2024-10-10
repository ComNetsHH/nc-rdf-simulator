import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

def get_delays(num_nodes, run=0):
    run_delays = []
    for i in range(1,9):
        try:
            data = pd.read_csv(f'./res/v3/pf_n{int(num_nodes)}_r{i}.csv')
            delays = data[data['eventType'] == 'PktRcvd']
            delays = delays['delay'].to_numpy()
            print(delays)
            run_delays.append(np.mean(delays))
            #run_delays.append(len(delays))
        except:
            print('ERR', num_nodes, i)

    return run_delays

def main():
    num_nodes = np.arange(1000, 1500, 50)
    avg_delays = []
    pkt_rcvd = []

    print(num_nodes)

    for n in num_nodes:
        delays = get_delays(n)
        avg_delays.append(np.mean(delays))
        pkt_rcvd.append(len(delays))

    print(avg_delays)
    #plt.plot(num_nodes, pkt_rcvd / np.max(pkt_rcvd))
    plt.plot(num_nodes, avg_delays)
    plt.show()

if __name__ == '__main__':
    main()

