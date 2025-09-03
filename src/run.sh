#!/bin/bash
#SBATCH -A gts-vsarkar9-forza
#SBATCH --ntasks-per-node=16
#SBATCH -N 32
#SBATCH -t3:00:00
#SBATCH -qinferno
#SBATCH --mem-per-cpu=12gb

echo "Started on `/bin/hostname`"   # prints name of compute node job was started on
cd $SLURM_SUBMIT_DIR 

source oshmem-slurm.sh
cd /storage/home/hcoda1/8/ssinghal74/r-vsarkar9-1/Actor-GM/src
srun -N 32 -n 512 ./main -f ~/scratch/mel-dataset/converted/kmer_V1r.mtx