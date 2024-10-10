import json
import scipy.stats
import numpy as np
import matplotlib.pyplot as plt

def get_result(v, num_nodes, run, interval = 1000, decayFactor=1):
    with open(f'res/v{v}_parsed/rdf_n{num_nodes}_i{interval}_q{decayFactor}_r{run}_summary.json') as f:
        data = json.load(f)
        return data

def calculate_confidence_interval(data, confidence=.95):
    n = len(data)
    m = np.mean(data)
    std_dev = scipy.stats.sem(data)
    h = std_dev * scipy.stats.t.ppf((1 + confidence) / 2, n - 1)
    return [m, m - h, m + h]

def main(interval=300, decayFactor=100):
    v = 14

    avg_aoi = []
    aoi_ci = []
    avg_diss_rate = []
    nn = range(50, 1200, 50)

    for num_nodes in nn:
        aoi = []
        diss_rate = []
        for r in range(0,10):
            try:
                data = get_result(v, num_nodes, r, interval, decayFactor)
                aoi.append(data['excess_probability_1_hop'] * 100) #avg_dissemination_rate
                diss_rate.append(data['avg_dissemination_rate'])
            except:
                print('ERR')
        
        [m,l,h] = calculate_confidence_interval(aoi)
        avg_aoi.append(m)
        aoi_ci.append(h-m)
        avg_diss_rate.append(np.mean(diss_rate))
    
    print(avg_aoi)

    fig, ax1 = plt.subplots()

    color = 'tab:red'
    ax1.set_xlabel('Num Nodes')
    ax1.set_ylabel('Excess Probability [%]', color=color)
    ax1.plot(nn, avg_aoi, '.-', color=color)
    ax1.errorbar(nn, avg_aoi, aoi_ci, alpha=1, capsize=4, color=color)
    ax1.tick_params(axis='y', labelcolor=color)
    ax1.set_ylim(0, 25)
    ax1.set_yticks([1, 2, 5, 10, 15, 20, 25])
    ax1.grid()
    ax1.legend(loc="lower right")

    ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis

    color = 'tab:blue'
    ax2.set_ylabel('Avg Dissemination Rate', color=color)  # we already handled the x-label with ax1
    ax2.plot(nn, avg_diss_rate, '.-', color=color)
    ax2.tick_params(axis='y', labelcolor=color)
    ax2.set_ylim(0, 1)

    plt.title(f'Avg AoI & Dissemination Rate (RDF) i={interval}ms, q={decayFactor/100:.2f}')

    fig.tight_layout()  # otherwise the right y-label is slightly clipped
    fig.savefig(f'./figures/rdf_1_hop_aoi_i{interval}_q{decayFactor}_v{v}.png', dpi=500, bbox_inches='tight', pad_inches=0.01)
    fig.savefig(f'./figures/rdf_1_hop_aoi_i{interval}_q{decayFactor}_v{v}.pdf', dpi=500, bbox_inches='tight', pad_inches=0.01)
    plt.show()

if __name__ == '__main__':
    for i in [50, 100, 250, 500]:
        for q in [100, 120, 150, 170, 200, 250]:
            main(i,q)