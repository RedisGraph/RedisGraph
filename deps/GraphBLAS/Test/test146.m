function test146
%TEST146 test C<M,struct> = scalar

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('test146 --------------------- C<M,struct> = scalar\n') ;

m = 5 ;
n = 4 ;

M = logical (sprand (m, n, 0.5)) ;

C1 = GB_mex_expand (M, pi) 
C2 = sparse (m, n) ;
C2 (M) = pi 
assert (isequal (C2, C1.matrix))

z = 1 + 1i ;
C1 = GB_mex_expand (M, z) 
C2 = sparse (m, n) ;
C2 (M) = z 
assert (isequal (C2, C1.matrix))

C1 = GB_mex_expand (M, true) 
C2 = logical (sparse (m, n)) ;
C2 (M) = true
assert (isequal (C2, logical (C1.matrix)))

fprintf ('test146: all tests passed\n') ;

