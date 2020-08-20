function gbtest7
%GBTEST7 test GrB.build

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;

n = 5 ;
A = sprand (n, n, 0.5) ;
A (n,n) = 5 ;

[i, j, x] = find (A) ;
[m, n] = size (A) ;

G = GrB.build (i, j, x, m, n) ;
S = sparse   (i, j, x, m, n) ;
assert (gbtest_eq (S, G)) ;

d.kind = 'GrB' ;
G = GrB.build (i, j, x, m, n, d) ;
assert (gbtest_eq (S, G)) ;

d.kind = 'sparse' ;
G = GrB.build (i, j, x, m, n, d) ;
assert (gbtest_eq (S, G))

i0 = int64 (i) - 1 ;
j0 = int64 (j) - 1 ;

G = GrB.build (i0, j0, x, struct ('base', 'zero-based')) ;
assert (gbtest_eq (S, G)) ;

G = GrB.build (1:3, 1:3, [1 1 1]) ;
assert (gbtest_eq (speye (3), G)) ;

G = GrB.build (1, 1, [1 2 3]) ;
assert (isequal (sparse (6), G)) ;

G = GrB.build (1:3, 1:3, 1) ;
assert (isequal (speye (3), G)) ;

types = gbtest_types ;
for k = 1: length(types)
    type = types {k} ;
    G = GrB.build (1:3, 1:3, cast (1, type)) ;
    assert (isequal (speye (3), double (G))) ;
    assert (isequal (GrB.type (G), type)) ;
end

fprintf ('gbtest7: all tests passed\n') ;

