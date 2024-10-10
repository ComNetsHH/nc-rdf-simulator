#!/bin/bash -l
#SBATCH -p ib 
#SBATCH --job-name="STOCHASTIC_FLOODING"             	 
#SBATCH --ntasks=1           
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=16G
#SBATCH --array=0-4999
#SBATCH --mail-type=ALL
#SBATCH --mail-user="k.fuger@tuhh.de"
#SBATCH --time=5-12:00:00
#SBATCH --constraint=OS8

# Execute simulation
python3 run_sf_experiment.py $SLURM_ARRAY_TASK_ID 5000

# Exit job
exit