import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

def get_pdrs(num_nodes, run=0):
    pdrs = []
    for i in range(1,9):
        try:
            data = pd.read_csv(f'./res/v3/pf_n{int(num_nodes)}_r{i}.csv')
            rcvd_packets = data[data['eventType'] == 'PktRcvd'].to_numpy()
            sent_packets = data[data['eventType'] == 'PktSent'].to_numpy()

            pdrs.append(len(rcvd_packets) / len(sent_packets))
            #run_delays.append(len(delays))
        except:
            print('ERR', num_nodes, i)

    return pdrs

def main():
    num_nodes = np.arange(1000, 1600, 50)
    avg_pdrs = []

    print(num_nodes)

    for n in num_nodes:
        print(n)
        pdrs = get_pdrs(n)
        avg_pdrs.append(np.mean(pdrs))

    plt.plot(num_nodes, avg_pdrs)
    plt.show()

if __name__ == '__main__':
    main()

