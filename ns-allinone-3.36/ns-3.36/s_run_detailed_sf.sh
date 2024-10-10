#!/bin/bash -l
#SBATCH --job-name="DETAILED_STOCHASTIC_FLOODING"             	 
#SBATCH --ntasks=1           
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=32G
#SBATCH --array=0-750
#SBATCH --mail-type=ALL
#SBATCH --mail-user="k.fuger@tuhh.de"
#SBATCH --time=5-12:00:00

# Execute simulation
pyenv shell 3.9.14
python3 run_detailed_sf_experiment.py $SLURM_ARRAY_TASK_ID 0

# Exit job
exit
