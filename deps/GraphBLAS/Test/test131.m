function test131
%TEST131 test GrB_Matrix_clear

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\ntest131: GrB_Matrix_clear\n') ;

rng ('default') ;

A = sparse (rand (4)) ;
C = GB_mex_clear (A) ;
S = sparse (4,4) ;
assert (isequal (S, C.matrix)) ;

Ahyper.matrix = A ;
Ahyper.is_hyper = true ;

C = GB_mex_clear (Ahyper) ;
assert (isequal (S, C.matrix)) ;

A = sparse (rand (4,1)) ;
C = GB_mex_clear (A) ;
S = sparse (4,1) ;
assert (isequal (S, C.matrix)) ;

fprintf ('\ntest131: all tests passed\n') ;

