function test147
%TEST147 test C<M>A*B with very sparse M

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('test147 ----------------------------- C<M>A*B with very sparse M\n') ;
rng ('default') ;

n = 1000 ;
A = sprand (n, n, 0.01) ;
A (:,1:300) = 1 ;
S = sparse (n, n) ;
M = spones (speye (n) + sparse (n, n, 1e-5)) ;

semiring.add = 'plus' ;
semiring.multiply = 'times' ;
semiring.class = 'double' ;

C1 = GB_mex_mxm (S, M, [ ], semiring, A, A, [ ]) ;
C2 = (A*A) .* M ;

assert (norm (C1.matrix - C2, 1) < 1e-12)

fprintf ('test147: all tests passed\n') ;

