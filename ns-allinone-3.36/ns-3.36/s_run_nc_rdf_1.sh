#!/bin/bash -l
#SBATCH -p ib 
#SBATCH --job-name="NC-RDF"             	 
#SBATCH --ntasks=1           
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=16G
#SBATCH --array=0-999
#SBATCH --mail-type=FAIL
#SBATCH --mail-user="k.fuger@tuhh.de"
#SBATCH --time=5-12:00:00
#SBATCH --constraint=OS8
#SBATCH --output=/dev/null

# Execute simulation
pyenv shell 3.9.14
export PROJECT_ID=phd-data-414113
export GOOGLE_APPLICATION_CREDENTIALS=/fibus/fs0/14/ckf1377/cred/phd-data-key.json

python3 run_nc_rdf_experiment.py $SLURM_ARRAY_TASK_ID 0

# Exit job
exit
