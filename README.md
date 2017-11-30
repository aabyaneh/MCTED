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

## References:
[1] Zhang, Kaizhong, and Dennis Shasha. "Simple fast algorithms for the editing distance between trees and related problems." SIAM journal on computing, 1989.

[2] tree-edit-distance.dbresearch.uni-salzburg.at