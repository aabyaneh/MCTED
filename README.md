# MCTED

Multi-Core Tree Edit Distance computation based on Zhang-Shasha algorithm [1].

### How to compile:
```
clang++ -O3 -std=c++11 TreeSim.cpp -lpthread -o mcted
```

### How to execute:
```
./mcted -h
```

## Methodology:
There are three steps:
1. Dependency detection of jobs
2. Managing overhead of synchronisation
3. Designing and using a concurrent thread-pool to manage independent jobs that can be executed in parallel

## References:
[1] Zhang, Kaizhong, and Dennis Shasha. "Simple fast algorithms for the editing distance between trees and related problems." SIAM journal on computing, 1989.

[2] tree-edit-distance.dbresearch.uni-salzburg.at