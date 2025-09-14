## Generate RGG graphs
```
mpicxx generate_rgg.cpp -o generate_rgg

srun -N 128 -n 2048 ./generate_rgg -n 536870912 --lcg -o /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/rgg_small

srun -N 128 -n 2048 ./generate_rgg -n 1073741824 --lcg -o /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/rgg_medium

srun -N 128 -n 2048 ./generate_rgg -n 2147483648 --lcg -o /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/rgg_big

make rgg_to_mtx
srun -N 32 -n 512 ./rgg_to_mtx -f ~/scratch/mel-dataset/rgg_small.mtx -o ~/scratch/mel-dataset/converted/rgg_small.mtx

srun -N 32 -n 512 ./rgg_to_mtx -f ~/scratch/mel-dataset/rgg_medium.mtx -o ~/scratch/mel-dataset/converted/rgg_medium.mtx

srun -N 32 -n 512 ./rgg_to_mtx -f ~/scratch/mel-dataset/rgg_large.mtx -o ~/scratch/mel-dataset/converted/rgg_large.mtx

mpicxx mtx_to_bin.cpp -o mtx_to_bin

srun -N 128 -n 2048 ./mtx_to_bin ~/scratch/mel-dataset/converted/rgg_small.mtx ~/scratch/mel-dataset/converted/rgg_small.bin

srun -N 128 -n 2048 ./mtx_to_bin ~/scratch/mel-dataset/converted/rgg_medium.mtx ~/scratch/mel-dataset/converted/rgg_medium.bin

srun -N 128 -n 2048 ./mtx_to_bin ~/scratch/mel-dataset/converted/rgg_big.mtx ~/scratch/mel-dataset/converted/rgg_big.bin

```