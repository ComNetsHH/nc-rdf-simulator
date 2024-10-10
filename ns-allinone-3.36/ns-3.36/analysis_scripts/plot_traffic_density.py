import json, math
import scipy.stats
import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import savgol_filter

traffic_density_positions = None
traffic_density_curve = None

traffic_density_computed = False

def get_colision_result(v, num_nodes, run, interval = 1000):
    try:
        with open(f'res/v{v}_parsed/summary_collision_n{num_nodes}_i{interval}_r{run}.json') as f:
            data = json.load(f)
            return data
    except:
        return {
            'collision_rate': -1
        }

def get_collision_rate_distribution(v, num_nodes, interval, num_runs):
    collision_rate = []
    for i in range(num_runs):
        data = get_colision_result(v, num_nodes, i, interval)
        collision_rate.append(data['collision_rate'])
    collsision_rates = np.array(collision_rate)
    collsision_rates = collsision_rates[collsision_rates > -1]
    return np.histogram(collsision_rates, np.arange(0,1.0, 0.1))

def get_expected_excess(traffic_density):
    global traffic_density_computed
    global traffic_density_positions
    global traffic_density_curve

    if not traffic_density_computed:
        excess = []
        nn = np.arange(20, 1000, 5)
        t_max = 500
        for num_nodes in nn:

            (p_collsision_rates, bins) = get_collision_rate_distribution(21, num_nodes, 60, 800)

            p_collsision_rates = p_collsision_rates / np.sum(p_collsision_rates)
            n = np.arange(1, 100)
            p_n_sum = np.zeros(len(n))
            i=0

            for p_c in np.arange(0.1, 1.0, 0.1):
                # np.power(p_c, n - 1)
                p_n = (1-p_c) *np.power(p_c, n - 1)
                p_n_sum += p_n * p_collsision_rates[i]
                i+=1

            excess_factor = [max(0, (1 - t_max / (ni * 60))) for ni in n]
            
            e = np.sum((p_n_sum * excess_factor))
            excess.append(e)


        A = (0.509 * 2.5)**2 * math.pi
        factor = 1000 / (60 * A)
        positions = nn * factor
        traffic_density_positions = positions
        traffic_density_curve = savgol_filter(np.array(excess), 51, 3)
        traffic_density_computed = True
    
    idx = (np.abs(traffic_density_positions - traffic_density)).argmin()

    print(idx, traffic_density)
    return traffic_density_curve[idx] * 100


def get_result(v, num_nodes, run, interval = 1000, decayFactor=1):
    with open(f'res/v{v}_parsed/summary_rdf_n{num_nodes}_i{interval}_q{decayFactor}_r{run}.json') as f:
        data = json.load(f)
        return data

def calculate_confidence_interval(data, confidence=.95):
    n = len(data)
    m = np.mean(data)
    std_dev = scipy.stats.sem(data)
    h = std_dev * scipy.stats.t.ppf((1 + confidence) / 2, n - 1)
    return [m, m - h, m + h]

def main(interval=300, decayFactor=100):
    v = 24

    avg_aoi = []
    aoi_ci = []
    avg_traffic_density = []
    avg_traffic_density_fwd = []

    expected_excess = []
    nn = range(20, 500, 20)

    for num_nodes in nn:
        aoi = []
        traffic_density = []
        traffic_density_fwd =[]

        for r in range(0,10):
            try:
                data = get_result(v, num_nodes, r, interval, decayFactor)
                aoi.append(data['excess_probability_1_R'] * 100) #avg_dissemination_rate
                traffic_density.append(data['num_sent'] + data['num_fwd'])
                traffic_density_fwd.append(data['num_fwd'])
            except:
                print('ERR')
        
        [m,l,h] = calculate_confidence_interval(aoi)
        avg_aoi.append(m)
        aoi_ci.append(h-m)
        avg_traffic_density.append(np.mean(traffic_density))
        avg_traffic_density_fwd.append(np.mean(traffic_density_fwd))

        expected_excess.append(get_expected_excess((avg_traffic_density[-1]) / (25 * 180)))

    fig, ax1 = plt.subplots()

    color = 'tab:red'
    ax1.set_xlabel('Num Nodes')
    ax1.set_ylabel('Excess Probability [%]', color=color)
    ax1.plot(nn, avg_aoi, '.-', color=color)
    ax1.errorbar(nn, avg_aoi, aoi_ci, alpha=1, capsize=4, color=color)
    ax1.plot(nn, expected_excess, '--', color=color)
    ax1.tick_params(axis='y', labelcolor=color)
    ax1.set_ylim(0, 25 * 1.05)
    ax1.set_xlim(0, 500)
    ax1.set_yticks([1, 2, 5, 10, 15, 20, 25])
    ax1.grid()

    ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis

    color = 'tab:blue'
    ax2.set_ylabel('Traffic Density [Erlang / km2]', color=color)  # we already handled the x-label with ax1
    ax2.plot(nn, np.array(avg_traffic_density) / (25 * 180), '.-', color=color)
    ax2.plot(nn, np.array(avg_traffic_density_fwd) / (25 * 180), '--', color=color)
    ax2.tick_params(axis='y', labelcolor=color)
    ax2.set_ylim(0, 15000)
    ax2.set_xlim(0, 500)
    #ax2.set_yticks([0, 0.2, 0.4, 0.6, 0.8, 0.99])
    #ax2.grid()

    plt.title(f'Avg AoI & Dissemination Rate (RDF) i={interval}ms, q={decayFactor/100:.2f}')

    fig.tight_layout()  # otherwise the right y-label is slightly clipped
    fig.savefig(f'./figures/rdf_traffic_density_i{interval}_q{decayFactor}_v{v}.png', dpi=500, bbox_inches='tight', pad_inches=0.01)
    fig.savefig(f'./figures/rdf_traffic_density_i{interval}_q{decayFactor}_v{v}.pdf', dpi=500, bbox_inches='tight', pad_inches=0.01)
    plt.show()

if __name__ == '__main__':
    for i in [60, 120, 180, 240]:
        for q in [100, 150, 200, 250, 300, 350]:
            main(i,q)