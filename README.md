# nc-rdf-simulator

This repository holds the simulation code for the evaluation of flooding protocols for urban UAV networks. It is intended for interested parties to reproduce research results, understand the inner workings of the protocols, or apply them to their own scenarios.

The simulation is based on [ns-3](https://www.nsnam.org/) and uses its IEEE 802.11p implementation.

All flooding protocols are implemented as application layer protocols.

## Getting started
All simulation code is the `ns-allinone-3.36/ns-3.36` folder. In there, simulations are setup in the `scratch` folder. For convenience, all simulations are wrapped in a python scripts.

To execute a NC-RDF simulation, run

```
python3 run_nc_rdf_experiment <k> 0
```

where $k \in [0, 2079]$.

The `exploration` directory contains all scripts used to post process and analyze the simulation data.
