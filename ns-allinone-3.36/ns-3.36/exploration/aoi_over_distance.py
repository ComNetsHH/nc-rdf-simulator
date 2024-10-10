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
i_rdf = 120
i_sf = 120
i_pf = 120
i_cbf = 120
p = 50
q = 200
r = 0

def get_peak_aoi_by_distance(events, positions):
    events = events[events['timestamp'] >= 5.0]
    event_times = events.loc[:,['timestamp']].drop_duplicates()
    positions = positions.groupby('nodeId', group_keys=False).apply(lambda group: pd.concat([group, event_times]).sort_values('timestamp').reset_index(drop=True).interpolate())
    events = pd.merge(events, positions, how='inner', on=['nodeId','timestamp'])
    send_events = events.query('eventType == "PktSent"')
    nodeIds = np.sort(events['nodeId'].unique())

    peak_aoi = pd.DataFrame()

    for sender in nodeIds:

        for receiver in nodeIds:
            if sender != receiver:
                receive_events = events.query(f'eventType == "PktRcvd" & src == {sender} & nodeId == {receiver}')
                receive_times = np.array(receive_events['timestamp'])
                delays = np.array(receive_events['delay'])[0:-1]
                # iat = receive_times[1:0] - receive_times[0:-1]
                iat = np.diff(receive_times)
                aoi_peaks = iat + delays / 1000
                sender_pos = positions[positions['timestamp'].isin(receive_times) & (positions['nodeId'] == sender)]
                receiver_pos = positions[positions['timestamp'].isin(receive_times) & (positions['nodeId'] == receiver)]
                dist = np.array(np.sqrt(np.power(sender_pos['pos_x'] - receiver_pos['pos_x'],2) + np.power(sender_pos['pos_y'] - receiver_pos['pos_y'],2)))
                #print(dist)
                peak_aoi = pd.concat([peak_aoi, pd.DataFrame({'dist': dist[1:], 'aoi': aoi_peaks})])   

    return peak_aoi

rdf_events = pd.read_csv(f'../res/v{v}/rdf_n{n}_i{i_rdf}_q{q}_r{r}.csv')
rdf_positions = pd.read_csv(f'../res/v{v}/course_rdf_n{n}_i{i_rdf}_q{q}_r{r}.csv')
sf_events = pd.read_csv(f'../res/v{v}/sf_n{n}_i{i_sf}_p{p}_r{r}.csv')
sf_positions = pd.read_csv(f'../res/v{v}/course_sf_n{n}_i{i_sf}_p{p}_r{r}.csv')
cbf_events = pd.read_csv(f'../res/v{v}/rdf_n{n}_i{i_cbf}_q0_r{r}.csv')
cbf_positions = pd.read_csv(f'../res/v{v}/course_rdf_n{n}_i{i_cbf}_q0_r{r}.csv')
pf_events = pd.read_csv(f'../res/v{v}/sf_n{n}_i{i_pf}_p100_r{r}.csv')
pf_positions = pd.read_csv(f'../res/v{v}/course_sf_n{n}_i{i_pf}_p100_r{r}.csv')

peak_aoi_rdf = get_peak_aoi_by_distance(rdf_events, rdf_positions)
peak_aoi_sf = get_peak_aoi_by_distance(sf_events, sf_positions)
peak_aoi_cbf = get_peak_aoi_by_distance(cbf_events, cbf_positions)
peak_aoi_pf = get_peak_aoi_by_distance(pf_events, pf_positions)

peak_aoi_rdf.to_csv(f'../res/v{v}_parsed/peak_aoi_rdf.csv')
peak_aoi_cbf.to_csv(f'../res/v{v}_parsed/peak_aoi_cbf.csv')
peak_aoi_sf.to_csv(f'../res/v{v}_parsed/peak_aoi_sf.csv')
peak_aoi_pf.to_csv(f'../res/v{v}_parsed/peak_aoi_pf.csv')

rdf_color = '#7eb0d5'
sf_color = '#fd7f6f'
pf_color = '#23a67a'
cbf_color = '#ffa92f'

fig, ax = plt.subplots()
bin_width = 100
bin_means_rdf, bin_edges, _ = scipy.stats.binned_statistic(peak_aoi_rdf['dist'].to_numpy(), peak_aoi_rdf['aoi'].to_numpy(), statistic='mean', bins=np.arange(-bin_width/2, 3000+ bin_width/2, bin_width))
bin_means_cbf, bin_edges, _ = scipy.stats.binned_statistic(peak_aoi_cbf['dist'].to_numpy(), peak_aoi_cbf['aoi'].to_numpy(), statistic='mean', bins=np.arange(-bin_width/2, 3000+ bin_width/2, bin_width))
bin_means_sf, bin_edges, _ = scipy.stats.binned_statistic(peak_aoi_sf['dist'].to_numpy(), peak_aoi_sf['aoi'].to_numpy(), statistic='mean', bins=np.arange(-bin_width/2, 3000+ bin_width/2, bin_width))
bin_means_pf, bin_edges, _ = scipy.stats.binned_statistic(peak_aoi_pf['dist'].to_numpy(), peak_aoi_pf['aoi'].to_numpy(), statistic='mean', bins=np.arange(-bin_width/2, 3000+ bin_width/2, bin_width))
bin_centers = (bin_edges[1:] + bin_edges[0:-1]) / 2
ax.plot(bin_centers, bin_means_rdf, color=rdf_color)
ax.plot(bin_centers, bin_means_cbf, color=cbf_color)
ax.plot(bin_centers, bin_means_sf, color=sf_color)
ax.plot(bin_centers, bin_means_pf, color=pf_color)
ax.scatter(peak_aoi_rdf['dist'], peak_aoi_rdf['aoi'], color='#7eb0d5',s=5, alpha=0.1)
ax.scatter(peak_aoi_sf['dist'], peak_aoi_sf['aoi'], color='#fd7f6f',s=5, alpha=0.1)
# ax.scatter(peak_aoi_pf['dist'], peak_aoi_pf['aoi'], color='#7eb0d5',s=5, alpha=0.1)
# ax.scatter(peak_aoi_sf['dist'], peak_aoi_sf['aoi'], color='#fd7f6f',s=5, alpha=0.1)

ax.set_xlabel('Distance [m]')
ax.set_ylabel('Peak AoI [s]')

ax.set_ylim([0, 10])
ax.set_xlim([0, 2900])

plt.show()

fig.savefig(f"../figures/peak_aoi_over_dist_n{n}_v{v}.pdf", dpi=500, bbox_inches='tight', pad_inches=0.01)
fig.savefig(f"../figures/peak_aoi_over_dist_n{n}_v{v}.png", dpi=500, bbox_inches='tight', pad_inches=0.01)

# %%


# %%


# %%



