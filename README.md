## Graph Matching (1/2 approx)
This repository contains parallel implementation of graph edge matching, half-approximate. The SOTA version before this repository is [ECP-ExaGraph/mel](https://github.com/ECP-ExaGraph/mel/tree/master).

## Contributions
- **Identification of error**: mel algorithm contains an error which is privy to immediate one-shot rejection of proposals which is inaccurate. The serial algorithm states that heaviest neighbors (edge weights) will be matched. But unless, one stores the proposals, it shall render a case where u--v could have matched, but doesn't. 
- **Corrected mel version** and a much faster algorithm simply due to the elimination of requirement of communicating _INVALID_ message and its broadcasted response.
- **Performance improved** using _hclib::Actors_, where overlap of asynchronous non-blocking sends via `PUT` and recvs happen during the computation in one step. Note that _mel_ employs blocking `PUT` and _does not allow_ overlap of computation with recvs. Both the algorithms use barrier after each step.
- **Verification pipeline** built for the algorithm.

## Algorithm
Assuming there are p processors in the system, this algorithm is network-bound, since it requires O(|E|/p) communication calls and memory accesses in worse-case. Since network is costly, it dominates the cost. For `mel` algorithm, please refer to their IPDPS 2019 paper.

### Our SOTA algorithm
Let the graph be G(V,E), and data structures used are status[1..|V|] and storage (pair of vertex and weight) [1..|V|].

- Read the data from _undirected and weighted_ graphs.
- Push handler uses ngbr[u], $\forall$ u in V. It scans the status of every vertex. If status[u] = INIT, search for v = ngbr[u] which has the highest weight w and send REQUEST(u,v,w) to PE which owns ‘v’ and mark status[u] = WAITING if proposal[u] != v, else send ACCEPT(u,v,w) to mb1 and mark status[u] = FINISHED. If status[u] = FINISHED or WAITING, skip.
- Pull handler [mb0] receives REQUEST(u,v,w). If status[v] = FINISHED, then send REJECT(u,v,w) to mb1. Else if ngbr[v] = u, send ACCEPT(u,v,w) to mb1 and mark status[v] = FINISHED; else  compare ‘w’ (and lowest glb vertex id) to update proposal[v]. If nothing updates, send REJECT(u,v,w) to mb1, else send the popped proposal vertex id REJECT(u’,v,w) to mb1 to owner of u’.
- Pull handler [mb1] receives ACCEPT and marks status[u] = FINISHED. If REJECT, pop the ngbr and mark status = INIT.

> For termination requirement of `hclib::Actors`, our implementation restricts [mb1] for ACCEPT and [mb2] for REJECT, since ACCEPT handlers can also produce new REJECT messages for storage of that matched vertex.

## Directory Structure
```
.
├── data 
│   ├── karate.bin
│   ├── karate.mtx
│   ├── rmat10.bin
│   ├── rmat10.mtx
│   ├── rmat6.bin
│   └── rmat6.mtx
├── mel-patched (contains Mel rectified)
│   ├── graph.hpp
│   ├── main.cpp
│   ├── Makefile
│   ├── match.hpp
│   ├── README.md
│   ├── results.log
│   ├── results_mel_original.log
│   ├── run.sh
│   └── utils.hpp
├── README.md
├── scripts (contains scripts for pre-processing datasets)
│   ├── convert-to-mtx.py
│   ├── convert-to-symmetric.py
│   ├── data.out
│   ├── data.sh
│   ├── generate_rgg.cpp
│   ├── Makefile
│   ├── mtx_to_bin.cpp
│   ├── rcm.py
│   ├── README.md
│   └── rgg_to_mtx.cpp
├── setup.sh (contains all dataset related conversion commands)
├── src (contains Actor-GM)
│   ├── graph.h
│   ├── main.cpp
│   ├── Makefile
│   ├── match.h
│   ├── results_block.log
│   ├── results_block_optimized.log
│   ├── results.log
│   ├── results_optimized.log
│   ├── run_block_optimized.sh
│   ├── run_block.sh
│   ├── run_optimized.sh
│   ├── run.sh
│   └── verify.h
└── test (contains format checking of input file)
    └── graph
        └── checkFormat.py

6 directories, 41 files
```

## Build
Install hclib library (`hclib::Actors`) from [hclib-actor.com](https://hclib-actor.com)

```
cd Actor-GM/src/
make
```
We have two flags `-DOPTIMIZED` and `-DBLOCK_D`. 
- `-DOPTIMIZED` is for when instead of using O(degree) linear search to compute the mate of a vertex, we pre-sort the the adjacency list of the vertex and incur only O(1) to find the mate. 
- By default, `main` program uses Cyclic 1D data distribution and enabling `-DBLOCK_D` uses Range/Block 1D distribution. 

This will generate four executables - `main`, `main_optimized`, `block` and `block_optimized`.

## Experimentation
Refer to `src/results.log` and `src/results_optimized.log` for our results of the implementation (cyclic 1D)
and refer to `src/results_block.log` and `src/results_block_optimized.log` for our results of the implementation (block 1D) tested on PACE at Georgia Tech, with upto 128 nodes, 16 ppn and 4GB memory per cpu (in exclusive mode).
```
cd Actor-GM/src/
source run_optimized.sh &> results_optimized.log
source run.sh &> results.log
source run_block_optimized.sh &> results_block_optimized.log
source run_block.sh &> results_block.log
```

### Corrected Mel Build and Implementation
```
cd Actor-GM/mel-patched/
make clean && make
source run.sh &> results.log
```

- `mel-patched` directory contains the corrected version of the `mel` algorithm.
- Refer to `mel-patched/results.log` for experimentation, done upto 128 nodes, 16 ppn and 12GB memory per cpu (in exclusive mode).

### Mel Results
You can download the `mel` repository at the aforementioned link. Then add `-DUSE_MPI_RMA` to `OPTFLAGS` and do `make`. The results for the `RMA` version is stored in `mel-patched/results_mel_original.log`, run upto 128 nodes, 16 ppn and 12GB memory per cpu (in exclusive mode).
