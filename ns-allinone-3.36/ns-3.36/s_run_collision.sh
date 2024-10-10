#!/bin/bash -l
#SBATCH -p ib 
#SBATCH --job-name="COLLISION"             	 
#SBATCH --ntasks=1           
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=16G
#SBATCH --array=0-9999
#SBATCH --mail-type=ALL
#SBATCH --mail-user="k.fuger@tuhh.de"
#SBATCH --time=1-12:00:00
#SBATCH --constraint=OS8
#SBATCH --output=/dev/null

# Execute simulation
python3 run_collision_experiment.py $SLURM_ARRAY_TASK_ID

# Exit job
exit
