# Assignment 2 Report
Group: TODO

# 1a)

#### Can we safely permute the loops?
Yes for small matrices. For big matrices the performance of the implementation is different, because cache
misses are getting dominant for some of the implementations. The fastest implementation is kij, we iterate along
row of A and row of C (least cache misses). The slowest implementation is jki, we iterate along a column of A
(to which we write and read) and a column of C. The implementation ijk is in between, we iterate along a row of
B and column of C, while we write to the same location of A.

#### Explain which of the loops (i, j, k) is parallelizable?
It's easily possible to parallelize the both outer loops (i, j), since they don't have data dependencies and the two
distinct threads will not write to the same memory location. When parallelizing the third loop (k), a conflict can happen
as multiple threads want to update the same memory location.
To do this, atomic operations are necessary.
