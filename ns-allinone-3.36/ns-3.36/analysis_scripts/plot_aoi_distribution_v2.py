import sys, os
import json
import pandas as pd
import numpy as np
from scipy.interpolate import interp1d  
import matplotlib.pyplot as plt

def get_result(v, filename):
    with open(f'res/v{v}_parsed/{filename}_aoi_dist_r1.json') as f:
        data = json.load(f)
        return data

def main(v, filename):

    data = get_result(v, filename) 
    t = np.array(data['t'])
    p = np.array(data['p'])

    excess_probability_1_hop = (1 - np.sum(p[t <= 500])) * 100
    

    fig = plt.figure()
    plt.plot(t, np.cumsum(p), label=f'E={excess_probability_1_hop:.2f}%')
    plt.bar(t, p,width=(max(t) - min(t)) /len(t))
    plt.xlabel('AoI [ms]')
    plt.ylabel('Probability')
    plt.axis([0, 500, 0, 1])
    plt.legend()
    plt.title(filename)
    plt.show()


if __name__ == "__main__":
    v = 18
    n = 320
    i = 250
    main(v, f'rdf_n{n}_i{i}_q100_r0')
    main(v, f'rdf_n{n}_i{i}_q120_r0')
    main(v, f'rdf_n{n}_i{i}_q150_r0')
    main(v, f'rdf_n{n}_i{i}_q200_r0')
    main(v, f'rdf_n{n}_i{i}_q250_r0')
