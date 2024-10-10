import numpy as np
import pandas as pd
import marshal, json
import matplotlib.pyplot as plt
import matplotlib.transforms as transforms
import matplotlib.patches as patches
from matplotlib import cm
import matplotlib
import json, math
from operator import sub
import scipy.stats
from IPython.display import set_matplotlib_formats
from IPython.display import Image
import matplotlib_inline.backend_inline

matplotlib_inline.backend_inline.set_matplotlib_formats('svg')
plt.rcParams.update({
    'font.family': 'serif',
    "font.serif": 'Times',
    'font.size': 12,
    'text.usetex': True,
    'pgf.rcfonts': False,
    'figure.dpi': 300,
    'savefig.dpi': 300,
    'text.latex.preamble': r'\usepackage{amsmath}'
})

v = 44
n = 100
i_sf = 120
p = 100

plot_traces = False

def get_num_possible_receivers(row, positions): 
    x0 = row['pos_x']
    y0 = row['pos_y']
    ts = row['timestamp']
    positions = positions.copy()
    positions_now = positions.query(f'timestamp == {ts}')
    positions_now['dist'] = np.sqrt(np.power(positions_now['pos_x'] -x0, 2) + np.power(positions_now['pos_y'] -y0, 2))
    valid_positions = positions_now[positions_now['dist'] <= 509.8]
    # print(row['num_receivers'] / (len(valid_positions) -1) ,len(positions), len(positions_now), row['num_receivers'], len(valid_positions) -1, row['seqNo'])
    return len(valid_positions)

def get_augmented_events(nodeId, events, positions):
    events = events[events['timestamp'] >= 5.0]
    events = events.query(f'nodeId == {nodeId} | src == {nodeId}')

    nodeIds = events['nodeId'].unique()
    positions = positions[positions['nodeId'].isin(nodeIds)]

    #event_positions = events[['nodeId','timestamp']]
    # positions = pd.concat([positions, event_positions]).sort_values('timestamp').reset_index(drop=True).drop_duplicates()
    # positions = positions.groupby('nodeId', group_keys=False).apply(lambda group: group.interpolate())
    
    event_times = events.loc[:,['timestamp']].drop_duplicates()
    positions = positions.groupby('nodeId', group_keys=False).apply(lambda group: pd.concat([group, event_times]).sort_values('timestamp').reset_index(drop=True).interpolate())
    
    if len(events) * len(positions) == 0:
        return pd.DataFrame([])
        
    events = pd.merge(events, positions, how='inner', on=['nodeId','timestamp'])

    sent_events = events.query('eventType == "PktSent"')
    received_events = events.query('eventType == "PktRcvd" & numHops == 0')
    received_count = received_events['seqNo'].value_counts().rename_axis('seqNo').to_frame('num_receivers')
    sent_events = sent_events.join(received_count, how='inner', on='seqNo')


    sent_events['num_possible_receivers'] = sent_events.apply(lambda row: get_num_possible_receivers(row, positions), axis=1)
    return sent_events

parsed_sent_events = pd.DataFrame([])
for r in range(10):
    rdf_events = pd.read_csv(f'../res/v{v}/sf_n{n}_i{i_sf}_p{p}_r{r}.csv')
    rdf_positions = pd.read_csv(f'../res/v{v}/course_sf_n{n}_i{i_sf}_p{p}_r{r}.csv')

    for i in range(1, n):
        parsed_sent_events = pd.concat([parsed_sent_events, get_augmented_events(i, rdf_events.copy(), rdf_positions.copy())])
        print(r, i, len(parsed_sent_events))

    parsed_sent_events.to_csv(f'../res/v{v}_parsed/sf_n{n}_i{i_sf}_p{p}.csv')
    reception_rates = np.array(parsed_sent_events['num_receivers']) / np.array(parsed_sent_events['num_possible_receivers'] - 1)
    size = math.sqrt(n / 12) * 1000
    fig, ax = plt.subplots()
    im = ax.hexbin(
        parsed_sent_events['pos_x'], 
        parsed_sent_events['pos_y'], 
        C=1-reception_rates, 
        vmin=0,
        vmax=1,
        gridsize=25,
        clip_on=True,
        cmap=cm.jet
    )

    ax.set_ylim([0,size])
    ax.set_xlim([0,size])
    ax.spines.top.set_visible(False)
    ax.spines.left.set_visible(False)
    ax.spines.right.set_visible(False)
    ax.spines.bottom.set_visible(False)
    ax.set_xlabel('x [m]')
    ax.set_ylabel('y [m]')
    plt.title(f'Avg. Loss Rate over Area RDF ($p_L = {1-np.mean(reception_rates):.2f}$)\n')
    plt.colorbar(im)

    fig.savefig(f"../figures/loss_rate_area_sf_n{n}_p{p}_v{v}.pdf", dpi=500, bbox_inches='tight', pad_inches=0.01)
    fig.savefig(f"../figures/loss_rate_area_sf_n{n}_p{p}_v{v}.png", dpi=500, bbox_inches='tight', pad_inches=0.01)




