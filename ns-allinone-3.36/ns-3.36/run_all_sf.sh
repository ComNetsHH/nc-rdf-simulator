parallel -j1 "python3 run_detailed_sf_experiment.py {1} 0" ::: {0..100}
