function gbtest59
%GBTEST59 test end

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

A = rand (4,7) ;
G = GrB (A) ;

A = A (2:end, 3:end) ;
G = G (2:end, 3:end) ;
assert (isequal (G, A)) ;

A = A (2:2:end, 3:2:end) ;
G = G (2:2:end, 3:2:end) ;
assert (isequal (G, A)) ;

A = rand (7, 1) ;
G = GrB (A) ;

A = A (2:2:end) ;
G = G (2:2:end) ;
assert (isequal (G, A)) ;

fprintf ('gbtest59: all tests passed\n') ;

