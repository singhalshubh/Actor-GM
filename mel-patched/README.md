```
[ssinghal74@atl1-1-02-008-2-1 mel-patched]$ srun -N 16 -n 256 ./match -f ~/scratch/mel-dataset/converted/graph500-scale21-ef16_adj.bin

-------------------------------------------------------
Graph edge distribution characteristics
-------------------------------------------------------
Number of vertices: 1243072
Number of edges: 63463300
Maximum number of edges: 1354090
Average number of edges: 247904
Expected value of X^2: 1.07722e+11
Variance: 4.62661e+10
Standard deviation: 215096
-------------------------------------------------------
Time to read input file and create distributed graph (in s): 0.790128
MPI-RMA-FIX[shubh]
Invoking matching[shubh]
Unique matchings: 269031
Execution time (in s) for maximal edge matching: 4.10691 on 256 processes.
[ssinghal74@atl1-1-02-008-2-1 mel-patched]$ srun -N 16 -n 256 ./match -f ~/scratch/mel-dataset/converted/graph500-scale22-ef16_adj.bin 

-------------------------------------------------------
Graph edge distribution characteristics
-------------------------------------------------------
Number of vertices: 2393285
Number of edges: 128194008
Maximum number of edges: 3866739
Average number of edges: 500758
Expected value of X^2: 4.76583e+11
Variance: 2.25824e+11
Standard deviation: 475210
-------------------------------------------------------
Time to read input file and create distributed graph (in s): 0.881145
MPI-RMA-FIX[shubh]
Invoking matching[shubh]
Unique matchings: 507290
Execution time (in s) for maximal edge matching: 5.89272 on 256 processes.
[ssinghal74@atl1-1-02-008-2-1 mel-patched]$ srun -N 16 -n 256 ./match -f ~/scratch/mel-dataset/converted/graph500-scale23-ef16_adj.bin 

-------------------------------------------------------
Graph edge distribution characteristics
-------------------------------------------------------
Number of vertices: 4606314
Number of edges: 258501410
Maximum number of edges: 6874376
Average number of edges: 1.00977e+06
Expected value of X^2: 1.85626e+12
Variance: 8.36622e+11
Standard deviation: 914671
-------------------------------------------------------
Time to read input file and create distributed graph (in s): 1.4713
MPI-RMA-FIX[shubh]
Invoking matching[shubh]
Unique matchings: 956011
Execution time (in s) for maximal edge matching: 9.34055 on 256 processes.
[ssinghal74@atl1-1-02-008-2-1 mel-patched]$ srun -N 16 -n 256 ./match -f ~/scratch/mel-dataset/converted/graph500-scale24-ef16_adj.bin 

-------------------------------------------------------
Graph edge distribution characteristics
-------------------------------------------------------
Number of vertices: 8860450
Number of edges: 520523686
Maximum number of edges: 13816291
Average number of edges: 2.0333e+06
Expected value of X^2: 7.59288e+12
Variance: 3.45859e+12
Standard deviation: 1.85973e+06
-------------------------------------------------------
Time to read input file and create distributed graph (in s): 2.13247
MPI-RMA-FIX[shubh]
Invoking matching[shubh]
Unique matchings: 1798564
Execution time (in s) for maximal edge matching: 13.8035 on 256 processes.

[ssinghal74@atl1-1-02-008-2-1 mel-patched]$ srun -N 16 -n 256 ./match -f ~/scratch/mel-dataset/converted/HV15R.bin 

-------------------------------------------------------
Graph edge distribution characteristics
-------------------------------------------------------
Number of vertices: 164374738
Number of edges: 326732307
Maximum number of edges: 105484640
Average number of edges: 1.2763e+06
Expected value of X^2: 1.27898e+14
Variance: 1.26269e+14
Standard deviation: 1.12369e+07
-------------------------------------------------------
Time to read input file and create distributed graph (in s): 4.86843
MPI-RMA-FIX[shubh]
Invoking matching[shubh]
Unique matchings: 964712
Execution time (in s) for maximal edge matching: 4.46421 on 256 processes.

[ssinghal74@atl1-1-02-008-2-1 mel-patched]$ srun -N 16 -n 256 ./match -f ~/scratch/mel-dataset/converted/cage15.bin 

-------------------------------------------------------
Graph edge distribution characteristics
-------------------------------------------------------
Number of vertices: 52177205
Number of edges: 99199551
Maximum number of edges: 5212272
Average number of edges: 387498
Expected value of X^2: 1.5406e+12
Variance: 1.39045e+12
Standard deviation: 1.17917e+06
-------------------------------------------------------
Time to read input file and create distributed graph (in s): 1.88794
MPI-RMA-FIX[shubh]
Invoking matching[shubh]
Unique matchings: 2103257
Execution time (in s) for maximal edge matching: 1.07745 on 256 processes.

[ssinghal74@atl1-1-02-008-2-1 mel-patched]$ srun -N 16 -n 256 ./match -f ~/scratch/mel-dataset/converted/com-Orkut.bin 

-------------------------------------------------------
Graph edge distribution characteristics
-------------------------------------------------------
Number of vertices: 3072441
Number of edges: 234370166
Maximum number of edges: 4499690
Average number of edges: 915508
Expected value of X^2: 1.15457e+12
Variance: 3.16416e+11
Standard deviation: 562509
-------------------------------------------------------
Time to read input file and create distributed graph (in s): 1.59523
MPI-RMA-FIX[shubh]
Invoking matching[shubh]
Unique matchings: 1325535
Execution time (in s) for maximal edge matching: 4.81343 on 256 processes.
[ssinghal74@atl1-1-02-008-2-1 mel-patched]$ srun -N 16 -n 256 ./match -f ~/scratch/mel-dataset/converted/com-Friendster.bin 

-------------------------------------------------------
Graph edge distribution characteristics
-------------------------------------------------------
Number of vertices: 65608366
Number of edges: 3612134270
Maximum number of edges: 25071727
Average number of edges: 1.41099e+07
Expected value of X^2: 2.2417e+14
Variance: 2.50806e+13
Standard deviation: 5.00806e+06
-------------------------------------------------------
Time to read input file and create distributed graph (in s): 8.20165
MPI-RMA-FIX[shubh]
Invoking matching[shubh]
Unique matchings: 19283029
Execution time (in s) for maximal edge matching: 12.8646 on 256 processes.

[ssinghal74@atl1-1-02-008-2-1 mel-patched]$ srun -N 16 -n 256 ./match -f ~/scratch/mel-dataset/converted/kmer_V2a.bin 

-------------------------------------------------------
Graph edge distribution characteristics
-------------------------------------------------------
Number of vertices: 55042369
Number of edges: 117217600
Maximum number of edges: 477654
Average number of edges: 457881
Expected value of X^2: 2.09693e+11
Variance: 3.72674e+07
Standard deviation: 6104.7
-------------------------------------------------------
Time to read input file and create distributed graph (in s): 1.29728
MPI-RMA-FIX[shubh]
Invoking matching[shubh]
Unique matchings: 23535879
Execution time (in s) for maximal edge matching: 6.28145 on 256 processes.

[ssinghal74@atl1-1-02-008-2-1 mel-patched]$ srun -N 16 -n 256 ./match -f ~/scratch/mel-dataset/converted/kmer_U1a.bin 

-------------------------------------------------------
Graph edge distribution characteristics
-------------------------------------------------------
Number of vertices: 67716231
Number of edges: 138778562
Maximum number of edges: 555606
Average number of edges: 542104
Expected value of X^2: 2.93936e+11
Variance: 5.95503e+07
Standard deviation: 7716.89
-------------------------------------------------------
Time to read input file and create distributed graph (in s): 1.058
MPI-RMA-FIX[shubh]
Invoking matching[shubh]
Unique matchings: 29080353
Execution time (in s) for maximal edge matching: 7.2233 on 256 processes.

[ssinghal74@atl1-1-02-008-2-1 mel-patched]$ srun -N 16 -n 256 ./match -f ~/scratch/mel-dataset/converted/kmer_P1a.bin 

-------------------------------------------------------
Graph edge distribution characteristics
-------------------------------------------------------
Number of vertices: 139353211
Number of edges: 297829984
Maximum number of edges: 1281552
Average number of edges: 1.1634e+06
Expected value of X^2: 1.3539e+12
Variance: 4.0466e+08
Standard deviation: 20116.2
-------------------------------------------------------
Time to read input file and create distributed graph (in s): 2.13674
MPI-RMA-FIX[shubh]
Invoking matching[shubh]
Unique matchings: 59554221
Execution time (in s) for maximal edge matching: 13.7074 on 256 processes.

[ssinghal74@atl1-1-02-008-2-1 mel-patched]$ srun -N 16 -n 256 ./match -f ~/scratch/mel-dataset/converted/kmer_V1r.bin 

-------------------------------------------------------
Graph edge distribution characteristics
-------------------------------------------------------
Number of vertices: 214005017
Number of edges: 465410904
Maximum number of edges: 1941910
Average number of edges: 1.81801e+06
Expected value of X^2: 3.30628e+12
Variance: 1.11009e+09
Standard deviation: 33318
-------------------------------------------------------
Time to read input file and create distributed graph (in s): 2.59331
MPI-RMA-FIX[shubh]
Invoking matching[shubh]
Unique matchings: 91442543
Execution time (in s) for maximal edge matching: 19.47 on 256 processes.



```