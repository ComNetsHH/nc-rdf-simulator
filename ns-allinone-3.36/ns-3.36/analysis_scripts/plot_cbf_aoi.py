import json
import scipy.stats
import numpy as np
import matplotlib.pyplot as plt

def calculate_confidence_interval(data, confidence=.95):
    n = len(data)
    m = np.mean(data)
    std_dev = scipy.stats.sem(data)
    h = std_dev * scipy.stats.t.ppf((1 + confidence) / 2, n - 1)
    return [m, m - h, m + h]

def get_result(v, num_nodes, run, interval = 1000):
    with open(f'res/v{v}_parsed/cbf_n{num_nodes}_i{interval}_r{run}.json') as f:
        data = json.load(f)
        return data


def main(interval=1000):
    v = 7

    avg_aoi = []
    aoi_ci = []
    avg_diss_rate = []
    nn = range(20, 500, 20)

    for num_nodes in nn:
        aoi = []
        diss_rate = []
        for r in range(1,11):
            try:
                data = get_result(v, num_nodes, r, interval)
                aoi.append(data['avg_aoi']) #avg_dissemination_rate
                diss_rate.append(data['avg_dissemination_rate'])
            except:
                print('ERR')
        
        [m,l,h] = calculate_confidence_interval(aoi)
        avg_aoi.append(m)
        aoi_ci.append(h-m)
        avg_diss_rate.append(np.mean(diss_rate))

    fig, ax1 = plt.subplots()
    axis_max = 22000

    color = 'tab:red'
    ax1.set_xlabel('Num Nodes')
    ax1.set_ylabel('Avg AoI [ms]', color=color)
    ax1.plot(nn, avg_aoi, '.-', color=color)
    ax1.errorbar(nn, avg_aoi, aoi_ci, alpha=1, capsize=4, color=color)
    index_min = np.argmin(avg_aoi[5:])
    value_min = np.min(avg_aoi[5:])
    ax1.plot([nn[5:][index_min], nn[5:][index_min]],[0, axis_max], color='#000000', label=f'min=({nn[5:][index_min]} | {int(value_min)})')
    ax1.tick_params(axis='y', labelcolor=color)
    ax1.set_ylim(2000, axis_max)
    ax1.legend(loc="lower right")

    ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis

    color = 'tab:blue'
    ax2.set_ylabel('Avg Dissemination Rate', color=color)  # we already handled the x-label with ax1
    ax2.plot(nn, avg_diss_rate, '.-', color=color)
    ax2.tick_params(axis='y', labelcolor=color)

    plt.title(f'Avg AoI & Dissemination Rate (CBF) i={interval}ms')

    fig.tight_layout()  # otherwise the right y-label is slightly clipped
    fig.savefig(f'./figures/cbf_aoi_diss_rate_i{interval}_v{v}.png', dpi=500, bbox_inches='tight', pad_inches=0.01)
    fig.savefig(f'./figures/cbf_aoi_diss_rate_i{interval}_v{v}.pdf', dpi=500, bbox_inches='tight', pad_inches=0.01)
    #plt.show()

if __name__ == '__main__':
    for i in [1000, 500, 300]:
        main(i)