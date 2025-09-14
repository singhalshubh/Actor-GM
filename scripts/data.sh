#!/bin/bash
#SBATCH --job-name=actor-1
#SBATCH -A gts-vsarkar9-forza
#SBATCH -N16
#SBATCH --ntasks-per-node=16
#SBATCH -t30:00:00
#SBATCH -qinferno        
#SBATCH -odata.out

echo "Started on `/bin/hostname`"   # prints name of compute node job was started on
cd $SLURM_SUBMIT_DIR 

srun -N 16 -n 256 ./rgg_to_mtx -f ~/scratch/mel-dataset/rgg_big.mtx -o ~/scratch/mel-dataset/converted/rgg_big.mtx