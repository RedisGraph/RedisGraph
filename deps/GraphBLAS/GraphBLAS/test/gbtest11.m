function gbtest11
%GBTEST11 test GrB, sparse

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;
A = 100 * rand (4) ;
A (1,1) = 0 %#ok<*NOPRT>
S = sparse (A)
assert (gbtest_eq (S, double (full (GrB (S)))))
assert (gbtest_eq (S, double (full (full (GrB (S))))))
assert (gbtest_eq (S, double (full (double (full (GrB (S)))))))

S2 = double (GrB (full (double (full (GrB (S))))))
assert (norm (S-S2,1) == 0)
% S2 = 1*S2 ;
assert (gbtest_eq (S, S2))

S2 = double (GrB (double (GrB (full (double (full (GrB (S))))))))
assert (gbtest_eq (S, S2))

S = logical (S) ;
assert (gbtest_eq (S, full (GrB (S))))

X = int8 (A)
G = GrB (X)
assert (gbtest_eq (X, full (int8 (G))))
assert (gbtest_eq (X, int8 (full (G))))

X = int16 (A)
G = GrB (X)
assert (gbtest_eq (X, full (int16 (G))))
assert (gbtest_eq (X, int16 (full (G))))

X = int32 (A)
G = GrB (X)
assert (gbtest_eq (X, full (int32 (G))))
assert (gbtest_eq (X, int32 (full (G))))

X = int64 (A)
G = GrB (X)
assert (gbtest_eq (X, full (int64 (G))))
assert (gbtest_eq (X, int64 (full (G))))

X = uint8 (A)
G = GrB (X)
assert (gbtest_eq (X, full (uint8 (G))))
assert (gbtest_eq (X, uint8 (full (G))))

X = uint16 (A)
G = GrB (X)
assert (gbtest_eq (X, full (uint16 (G))))
assert (gbtest_eq (X, uint16 (full (G))))

X = uint32 (A)
G = GrB (X)
assert (gbtest_eq (X, full (uint32 (G))))
assert (gbtest_eq (X, uint32 (full (G))))

X = uint64 (A)
G = GrB (X)
full (G)
assert (gbtest_eq (X, full (uint64 (G))))
assert (gbtest_eq (X, uint64 (full (G))))

fprintf ('gbtest11: all tests passed\n') ;

