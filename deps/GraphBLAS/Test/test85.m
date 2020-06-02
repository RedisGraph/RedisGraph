function test85
%TEST85 test GrB_transpose: 1-by-n with typecasting

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

A.matrix = sparse ([ 1 2 3 4]) ;
A.class  = 'single' ;

C.matrix = sparse (4,1) ;
C.class  = 'double' ;

C2 = GB_mex_transpose (C, [ ], [ ], A, [ ]) ;
assert (isequal (A.matrix', C2.matrix)) ;

fprintf ('\ntest85: all tests passed\n') ;

