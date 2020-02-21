function gbtest61
%GBTEST61 test GrB.laplacian

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;
n = 10 ;
A = sprand (n, n, 0.4) ;

S = tril (A, -1) ;
S = S+S' ;
G = GrB (S) ;

L0 = laplacian (graph (S, 'OmitSelfLoops')) ;

L1 = GrB.laplacian (S) ;
L2 = GrB.laplacian (G) ;
L3 = GrB.laplacian (G, 'double', 'check') ;

assert (isequal (L0, L1)) ;
assert (isequal (L0, L2)) ;
assert (isequal (L0, L3)) ;

G = GrB (G, 'by row') ;

L2 = GrB.laplacian (G) ;
L3 = GrB.laplacian (G, 'double', 'check') ;

assert (isequal (L0, L2)) ;
assert (isequal (L0, L3)) ;

types = { 'double', 'single', 'int8', 'int16', 'int32', 'int64' } ;
for k = 1:6
    type = types {k} ;
    L2 = GrB.laplacian (G, type) ;
    assert (isequal (GrB.type (L2), type)) ;
    assert (isequal (L0, double (L2))) ;
end


fprintf ('gbtest61: all tests passed\n') ;

