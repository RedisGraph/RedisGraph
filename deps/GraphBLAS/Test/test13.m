function test13
%TEST13 test GrB_tranpsose

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

cinclass  = 'int16' ;

A.matrix = 50 * sparse (rand (2)) ;
A.matrix (1,2) = 0  ;
A.class = 'logical' ;

Cin.matrix = 50 * sparse (rand (2))  ;
Cin.class = 'int16' ;

accum.opname = '' ;
accum.opclass = 'logical'

C = GB_mex_transpose  (Cin, [ ], accum, A, [ ]) ;
assert (spok (C.matrix*1) == 1) ;
S = GB_spec_transpose (Cin, [ ], accum, A, [ ]) ;

assert (isequal (full (double (C.matrix)), double (S.matrix))) ;

A =   GB_spec_matrix (A) ;
Cin = GB_spec_matrix (Cin) ;
Cmatrix = full (C.matrix) ;
Smatrix = full (S.matrix) ;

assert (isequal (C.class, cinclass)) ;
assert (isequal (C.class, S.class)) ;
if (~(isequalwithequalnans (full (double (C.matrix)), ...
    double (S.matrix))))
    assert (false)
end

fprintf ('\ntest13: all tests passed\n') ;

