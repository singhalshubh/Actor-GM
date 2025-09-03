## Graph Matching (1/2 approx)
This repository contains parallel implementation of graph edge matching, half-approximate. The SOTA version before this repository is [ECP-ExaGraph/mel](https://github.com/ECP-ExaGraph/mel/tree/master).

## Contributions
- mel algorithm contains an error which is privy to immediate one-shot rejection of proposals which is inaccurate. The serial algorithm states that heaviest neighbors (edge weights) will be matched. But unless, one stores the proposals, it shall render a case where u--v could have matched, but doesn't. (mate[u] = w initially)
- performance improved using hclib::Actor
- verification pipeline for algorithm
- mel corrected version (to be done)

## Build
Install hclib library (hclib::actor) from [hclib-actor.com](https://hclib-actor.com)

```
cd Actor-GM/src/
make
```

## Experimentation
These experiments are conducted on PACE at Georgia Tech, with 32 nodes, 16 ppn and 12GB memory per cpu (in exclusive mode).
 
```
[ssinghal74@atl1-1-01-003-4-1 src]$ srun -N 32 -n 512 ./main -f ~/scratch/mel-dataset/converted/graph500-scale21-ef16_adj.mtx 
Graph Matching 1/2-Approx Actor Version
Reading file takes 1.150 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/graph500-scale21-ef16_adj.mtx
Number of Vertices: 1243072
Number of Edges: 63463300
Sorting el for all G(V) takes 0.019 seconds
Time taken for matching: 0.559
Matching Size: 267507
Reading file takes 0.177 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/graph500-scale21-ef16_adj.mtx
Number of Vertices: 1243072
Number of Edges: 63463300
Sorting el for all G(V) takes 0.019 seconds
########## Verification ##########
########## Step 1, unique matchings asserted ##########
########## Step 2, all matched edges are in G(E) ##########
########## Step 3, graph after deletion has either endpoints in matching set ##########
Time taken for verification: 0.822

[ssinghal74@atl1-1-01-003-4-1 src]$ srun -N 32 -n 512 ./main -f ~/scratch/mel-dataset/converted/graph500-scale22-ef16_adj.mtx 
Graph Matching 1/2-Approx Actor Version
Reading file takes 1.999 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/graph500-scale22-ef16_adj.mtx
Number of Vertices: 2393285
Number of Edges: 128194008
Sorting el for all G(V) takes 0.031 seconds
Time taken for matching: 0.399
Matching Size: 503919
Reading file takes 0.353 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/graph500-scale22-ef16_adj.mtx
Number of Vertices: 2393285
Number of Edges: 128194008
Sorting el for all G(V) takes 0.031 seconds
########## Verification ##########
########## Step 1, unique matchings asserted ##########
########## Step 2, all matched edges are in G(E) ##########
########## Step 3, graph after deletion has either endpoints in matching set ##########
Time taken for verification: 2.569

[ssinghal74@atl1-1-01-003-4-1 src]$ srun -N 32 -n 512 ./main -f ~/scratch/mel-dataset/converted/graph500-scale23-ef16_adj.mtx 
Graph Matching 1/2-Approx Actor Version
Reading file takes 2.689 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/graph500-scale23-ef16_adj.mtx
Number of Vertices: 4606314
Number of Edges: 258501410
Sorting el for all G(V) takes 0.052 seconds
Time taken for matching: 0.516
Matching Size: 950663
Reading file takes 0.698 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/graph500-scale23-ef16_adj.mtx
Number of Vertices: 4606314
Number of Edges: 258501410
Sorting el for all G(V) takes 0.052 seconds
########## Verification ##########
########## Step 1, unique matchings asserted ##########
########## Step 2, all matched edges are in G(E) ##########
########## Step 3, graph after deletion has either endpoints in matching set ##########
Time taken for verification: 9.332

[ssinghal74@atl1-1-01-003-4-1 src]$ srun -N 32 -n 512 ./main -f ~/scratch/mel-dataset/converted/graph500-scale24-ef16_adj.mtx 
Graph Matching 1/2-Approx Actor Version
Reading file takes 5.479 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/graph500-scale24-ef16_adj.mtx
Number of Vertices: 8860450
Number of Edges: 520523686
Sorting el for all G(V) takes 0.102 seconds
Time taken for matching: 0.613
Matching Size: 1742309
Reading file takes 1.382 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/graph500-scale24-ef16_adj.mtx
Number of Vertices: 8860450
Number of Edges: 520523686
Sorting el for all G(V) takes 0.102 seconds
########## Verification ##########
########## Step 1, unique matchings asserted ##########
########## Step 2, all matched edges are in G(E) ##########
########## Step 3, graph after deletion has either endpoints in matching set ##########
Time taken for verification: 31.854

[ssinghal74@atl1-1-01-003-4-1 src]$ srun -N 32 -n 512 ./main -f ~/scratch/mel-dataset/converted/cage15.mtx 
Graph Matching 1/2-Approx Actor Version
Reading file takes 1.177 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/cage15.mtx
Number of Vertices: 52177205
Number of Edges: 94044692
Sorting el for all G(V) takes 0.006 seconds
Time taken for matching: 0.254
Matching Size: 2465472
Reading file takes 0.212 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/cage15.mtx
Number of Vertices: 52177205
Number of Edges: 94044692
Sorting el for all G(V) takes 0.005 seconds
########## Verification ##########
########## Step 1, unique matchings asserted ##########
########## Step 2, all matched edges are in G(E) ##########
########## Step 3, graph after deletion has either endpoints in matching set ##########
Time taken for verification: 17.517

[ssinghal74@atl1-1-01-003-4-1 src]$ srun -N 32 -n 512 ./main -f ~/scratch/mel-dataset/converted/HV15R.mtx 
Graph Matching 1/2-Approx Actor Version
Reading file takes 2.976 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/HV15R.mtx
Number of Vertices: 164374738
Number of Edges: 324715138
Sorting el for all G(V) takes 0.029 seconds
Time taken for matching: 1.337
Matching Size: 994641
Reading file takes 0.682 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/HV15R.mtx
Number of Vertices: 164374738
Number of Edges: 324715138
Sorting el for all G(V) takes 0.029 seconds
########## Verification ##########
########## Step 1, unique matchings asserted ##########
########## Step 2, all matched edges are in G(E) ##########
########## Step 3, graph after deletion has either endpoints in matching set ##########
Time taken for verification: 3.931

[ssinghal74@atl1-1-01-003-4-1 src]$ srun -N 32 -n 512 ./main -f ~/scratch/mel-dataset/converted/com-Orkut.mtx 
Graph Matching 1/2-Approx Actor Version
Reading file takes 3.261 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/com-Orkut.mtx
Number of Vertices: 3072441
Number of Edges: 234370166
Sorting el for all G(V) takes 0.021 seconds
Time taken for matching: 0.911
Matching Size: 1324040
Reading file takes 0.628 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/com-Orkut.mtx
Number of Vertices: 3072441
Number of Edges: 234370166
Sorting el for all G(V) takes 0.021 seconds
########## Verification ##########
########## Step 1, unique matchings asserted ##########
########## Step 2, all matched edges are in G(E) ##########
########## Step 3, graph after deletion has either endpoints in matching set ##########
Time taken for verification: 9.512

[ssinghal74@atl1-1-01-003-4-1 src]$ srun -N 32 -n 512 ./main -f ~/scratch/mel-dataset/converted/com-Friendster.mtx 
Graph Matching 1/2-Approx Actor Version
Reading file takes 25.872 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/com-Friendster.mtx
Number of Vertices: 65608366
Number of Edges: 3612134270
Sorting el for all G(V) takes 0.325 seconds
Time taken for matching: 2.659
Matching Size: 17141324
Reading file takes 7.435 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/com-Friendster.mtx
Number of Vertices: 65608366
Number of Edges: 3612134270
Sorting el for all G(V) takes 0.325 seconds
########## Verification ##########
########## Step 1, unique matchings asserted ##########
########## Step 2, all matched edges are in G(E) ##########
########## Step 3, graph after deletion has either endpoints in matching set ##########
Time taken for verification: 2511.772

[ssinghal74@atl1-1-01-003-4-1 src]$ srun -N 32 -n 512 ./main -f ~/scratch/mel-dataset/converted/static_highOverlap_lowBlockSizeVar_1000000_nodes.mtx 
Graph Matching 1/2-Approx Actor Version
Reading file takes 0.879 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/static_highOverlap_lowBlockSizeVar_1000000_nodes.mtx
Number of Vertices: 1000001
Number of Edges: 47562866
Sorting el for all G(V) takes 0.001 seconds
Time taken for matching: 0.350
Matching Size: 492213
Reading file takes 0.082 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/static_highOverlap_lowBlockSizeVar_1000000_nodes.mtx
Number of Vertices: 1000001
Number of Edges: 47562866
Sorting el for all G(V) takes 0.001 seconds
########## Verification ##########
########## Step 1, unique matchings asserted ##########
########## Step 2, all matched edges are in G(E) ##########
########## Step 3, graph after deletion has either endpoints in matching set ##########
Time taken for verification: 1.221

[ssinghal74@atl1-1-01-003-4-1 src]$ srun -N 32 -n 512 ./main -f ~/scratch/mel-dataset/converted/static_highOverlap_lowBlockSizeVar_5000000_nodes.mtx 
Graph Matching 1/2-Approx Actor Version
Reading file takes 2.146 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/static_highOverlap_lowBlockSizeVar_5000000_nodes.mtx
Number of Vertices: 5000001
Number of Edges: 237529656
Sorting el for all G(V) takes 0.006 seconds
Time taken for matching: 0.478
Matching Size: 2461950
Reading file takes 0.377 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/static_highOverlap_lowBlockSizeVar_5000000_nodes.mtx
Number of Vertices: 5000001
Number of Edges: 237529656
Sorting el for all G(V) takes 0.006 seconds
########## Verification ##########
########## Step 1, unique matchings asserted ##########
########## Step 2, all matched edges are in G(E) ##########
########## Step 3, graph after deletion has either endpoints in matching set ##########
Time taken for verification: 14.783

[ssinghal74@atl1-1-01-003-4-1 src]$ srun -N 32 -n 512 ./main -f ~/scratch/mel-dataset/converted/static_highOverlap_lowBlockSizeVar_20000000_nodes.mtx 
Graph Matching 1/2-Approx Actor Version
Reading file takes 5.325 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/static_highOverlap_lowBlockSizeVar_20000000_nodes.mtx
Number of Vertices: 20000001
Number of Edges: 950010974
Sorting el for all G(V) takes 0.028 seconds
Time taken for matching: 0.916
Matching Size: 9845744
Reading file takes 1.515 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/static_highOverlap_lowBlockSizeVar_20000000_nodes.mtx
Number of Vertices: 20000001
Number of Edges: 950010974
Sorting el for all G(V) takes 0.028 seconds
########## Verification ##########
########## Step 1, unique matchings asserted ##########
########## Step 2, all matched edges are in G(E) ##########
########## Step 3, graph after deletion has either endpoints in matching set ##########
Time taken for verification: 208.762

[ssinghal74@atl1-1-01-003-4-1 src]$ srun -N 32 -n 512 ./main -f ~/scratch/mel-dataset/converted/kmer_V2a.mtx 
Graph Matching 1/2-Approx Actor Version
Reading file takes 2.156 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/kmer_V2a.mtx
Number of Vertices: 55042369
Number of Edges: 117217600
Sorting el for all G(V) takes 0.005 seconds
Time taken for matching: 0.221
Matching Size: 16529185
Reading file takes 0.245 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/kmer_V2a.mtx
Number of Vertices: 55042369
Number of Edges: 117217600
Sorting el for all G(V) takes 0.005 seconds
########## Verification ##########
########## Step 1, unique matchings asserted ##########
########## Step 2, all matched edges are in G(E) ##########
########## Step 3, graph after deletion has either endpoints in matching set ##########
Time taken for verification: 546.510

[ssinghal74@atl1-1-01-003-4-1 src]$ srun -N 32 -n 512 ./main -f ~/scratch/mel-dataset/converted/kmer_U1a.mtx 
Graph Matching 1/2-Approx Actor Version
Reading file takes 2.217 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/kmer_U1a.mtx
Number of Vertices: 67716231
Number of Edges: 138778562
Sorting el for all G(V) takes 0.006 seconds
Time taken for matching: 0.206
Matching Size: 19053174
Reading file takes 0.292 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/kmer_U1a.mtx
Number of Vertices: 67716231
Number of Edges: 138778562
Sorting el for all G(V) takes 0.006 seconds
########## Verification ##########
########## Step 1, unique matchings asserted ##########
########## Step 2, all matched edges are in G(E) ##########
########## Step 3, graph after deletion has either endpoints in matching set ##########
Time taken for verification: 795.473

[ssinghal74@atl1-1-01-003-4-1 src]$ srun -N 32 -n 512 ./main -f ~/scratch/mel-dataset/converted/kmer_P1a.mtx 
Graph Matching 1/2-Approx Actor Version
Reading file takes 3.757 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/kmer_P1a.mtx
Number of Vertices: 139353211
Number of Edges: 297829984
Sorting el for all G(V) takes 0.014 seconds
Time taken for matching: 0.440
Matching Size: 36488877
Reading file takes 0.621 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/kmer_P1a.mtx
Number of Vertices: 139353211
Number of Edges: 297829984
Sorting el for all G(V) takes 0.014 seconds
########## Verification ##########
########## Step 1, unique matchings asserted ##########
########## Step 2, all matched edges are in G(E) ##########
########## Step 3, graph after deletion has either endpoints in matching set ##########
Time taken for verification: 3373.410

[ssinghal74@atl1-1-01-003-4-1 src]$ srun -N 32 -n 512 ./main -f ~/scratch/mel-dataset/converted/kmer_V1r.mtx 
Graph Matching 1/2-Approx Actor Version
Reading file takes 4.391 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/kmer_V1r.mtx
Number of Vertices: 214005017
Number of Edges: 465410904
Sorting el for all G(V) takes 0.022 seconds
Time taken for matching: 0.358
Matching Size: 54625233
Reading file takes 0.981 seconds
File /storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted/kmer_V1r.mtx
Number of Vertices: 214005017
Number of Edges: 465410904
Sorting el for all G(V) takes 0.022 seconds
########## Verification ##########
########## Step 1, unique matchings asserted ##########
########## Step 2, all matched edges are in G(E) ##########
########## Step 3, graph after deletion has either endpoints in matching set ##########
Time taken for verification: 9181.919
```

## Authors
Shubhendra Pal Singhal (ssinghal74@gatech.edu)
Habanero Labs, Georgia Institute of Technology, USA