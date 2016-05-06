# CLARISSE - Distributed middleware for aggregating and filtering performance counters <br />

-> N publisher nodes are generating performance counters either periodically or on-demand <br />
-> M subscriber nodes register for receiving these counters <br />
Subscribers can register to a subset of these counters or set the interval <br />
<br />
Approach: <br />
-> Bootstrap: launch n EvPath receivers with MPI and distribute the addresses to the tree nodes using MPI communication <br />
-> Use a logical tree topology for aggregating statistics <br />
-> Use trees with different number of children (e.g. 2, 4, 8, ...) <br />
-> Use a different time stamp for each aggregation step <br />
-> Implement statistics computations on the tree (MIN, MAX, AVG) <br />