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
This will generate two executables - `main` and `main_optimized`. `main` uses O(degree) linear search to compute the mate of a vertex, whereas `main_optimized` uses O(1) since the adjacency list of the vertex is sorted while reading.

## Experimentation
Refer to `src/results.log` and `src/results_optimized.log` for our results of the implementation tested on PACE at Georgia Tech, with upto 128 nodes, 16 ppn and 4GB memory per cpu (in exclusive mode).
```
cd Actor-GM/src/
source run_optimized.sh &> results_optimized.log
source run.sh &> results.log
```

### Corrected Mel Build and Implementation
```
cd Actor-GM/mel-patched/
make clean && make
source run.sh &> results.log
```

- `mel-patched` directory contains the corrected version of the `mel` algorithm.
- Refer to `mel-patched/results.log` for experimentation, done upto 128 nodes, 16 ppn and 12GB memory per cpu (in exclusive mode). Note that `mel` needs much higher memory requirment since it runs in OpenIB OOM errors or simply hangs due to insufficient memory.

## Authors
Shubhendra Pal Singhal (ssinghal74@gatech.edu)
Habanero Labs, Georgia Institute of Technology, USA