function test124
%TEST124 GrB_extract, trigger case 6

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('test124: GrB_extract, trigger case 6\n') ;

rng ('default') ;

m = 1000 ;
n = 1000 ;
d = 1e6 / (m*n ) ;
A = sprand (m, n, d) ;
A (:,n) = sprand (n, 1, 0.8) ;
huge = 1e9 ;
A (huge,1) = 1 ;
m = huge ;
A (3,n) = 999 ;

I = [3 4 1] ;
C = A (I,:) ;

I0 = uint64 (I) - 1 ;
S = sparse (length (I), n) ;

C2 = GB_mex_Matrix_extract (S, [ ], [ ], A, I0, [ ], [ ]) ;
assert (isequal (C, C2.matrix)) ;

fprintf ('test124: all tests passed\n') ;
