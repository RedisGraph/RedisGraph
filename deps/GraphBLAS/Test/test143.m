function test143
%TEST143 test special cases for C<!M>=A*B and C<M>=A*B

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('test143 ----------------------------- A*B special cases\n') ;

rng ('default') ;

n = 3000 ;
d = 0.001 ;
A = sprand (n, n, d) ;

semiring.add = 'plus' ;
semiring.multiply = 'times' ;
semiring.class = 'double' ;

% coarse Gustavson tasks, C<!M>=A*B, C(:,j) very sparse compared to M(:,j)
S = sparse (n, n) ;
M = logical (sprand (n, n, 0.01)) ;
M (:,1) = 1 ;
B = sprand (n, n, d) ;
C2 = GB_mex_mxm (S, M, [ ], semiring, A, B, struct ('mask', 'comp')) ;
C = (A*B) .* double (~M) ;
assert (nnz (C) > 0) ;
err = norm (C - C2.matrix, 1) ;
assert (err < 1e-12) ;

%----------------------------------------
desc = struct ('axb', 'hash', 'mask', 'comp') ;
%----------------------------------------

% coarse hash tasks, C<!M>=A*B
S = sparse (n, n) ;
M = logical (sprand (n, n, 0.01)) ;
B = sprand (n, n, d) ;
C2 = GB_mex_mxm (S, M, [ ], semiring, A, B, desc) ;
C = (A*B) .* double (~M) ;
assert (nnz (C) > 0) ;
err = norm (C - C2.matrix, 1) ;
assert (err < 1e-12) ;

% fine hash tasks, C<!M>=A*B
S = sparse (n, 1) ;
M = logical (sprand (n, 1, 0.01)) ;
B = sprand (n, 1, d) ;
C2 = GB_mex_mxm (S, M, [ ], semiring, A, B, desc) ;
C = (A*B) .* double (~M) ;
assert (nnz (C) > 0) ;
err = norm (C - C2.matrix, 1) ;
assert (err < 1e-12) ;

%----------------------------------------
desc = struct ('axb', 'hash') ;
%----------------------------------------

% coarse hash tasks, C<M>=A*B
S = sparse (n, n) ;
M = logical (sprand (n, n, 0.01)) ;
B = sprand (n, n, d) ;
C2 = GB_mex_mxm (S, M, [ ], semiring, A, B, desc) ;
C = (A*B) .* double (M) ;
assert (nnz (C) > 0) ;
err = norm (C - C2.matrix, 1) ;
assert (err < 1e-12) ;

% fine hash tasks, C<M>=A*B
S = sparse (n, 1) ;
M = logical (sprand (n, 1, 0.01)) ;
B = sprand (n, 1, d) ;
M (1:3) = 1 ;
A (1:3,1:3) = rand (3) ;
B (1:3) = rand (3,1) ;
C = (A*B) .* double (M) ;
assert (nnz (C) > 0) ;
C2 = GB_mex_mxm (S, M, [ ], semiring, A, B, desc) ;
err = norm (C - C2.matrix, 1) ;
assert (err < 1e-12) ;

%----------------------------------------
m = 10e6 ;
A = sprand (m, n, d) ;
[save save_chunk] = nthreads_get ;
nthreads_set (4, 1) ;
%----------------------------------------

% fine hash tasks, C=A*B
S = sparse (m, 1) ;
B = sprand (n, 1, d) ;
B (1:100, 1) = rand (100, 1) ;
C = (A*B) ;
assert (nnz (C) > 0) ;
C2 = GB_mex_mxm (S, [ ], [ ], semiring, A, B, desc) ;
err = norm (C - C2.matrix, 1) ;
assert (err < 1e-12) ;

nthreads_set (save, save_chunk) ;
fprintf ('\ntest143: all tests passed\n') ;

