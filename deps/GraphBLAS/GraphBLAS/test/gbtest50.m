function gbtest50
%GBTEST50 test GrB.ktruss and GrB.tricount

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;

load west0479 ; %#ok<*LOAD>
A = GrB.offdiag (west0479) ;
A = A+A' ;
C3a  = GrB.ktruss (A) ;
C3  = GrB.ktruss (A, 3) ;
assert (isequal (C3a, C3)) ;
C3  = GrB.ktruss (A, 3, 'check') ;
assert (isequal (C3a, C3)) ;

ntriangles = sum (C3, 'all') / 6 ;

C4a = GrB.ktruss (A, 4) ;
C4b = GrB.ktruss (C3, 4) ;          % this is faster
assert (isequal (C4a, C4b)) ;

nt2 = GrB.tricount (A) ;
assert (ntriangles == nt2) ;
assert (ntriangles == 235) ;

fprintf ('gbtest50: all tests passed\n') ;

