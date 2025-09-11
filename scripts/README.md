## Generate RGG graphs
```
g++ -O3 -march=native -fopenmp generate_rgg.cpp -o generate_rgg
export OMP_NUM_THREADS=24
srun -N 1 ./generate_rgg -n 536870000 -d 8.56e-05 -o /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/rgg_1.mtx
srun -N 1 ./generate_rgg -n 1070000000 -d 6.12E-05 -o /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/rgg_2.mtx
srun -N 1 ./generate_rgg -n 2140000000 -d 4.37E-05 -o /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/rgg_3.mtx
```