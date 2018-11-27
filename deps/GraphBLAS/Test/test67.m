function test67
%TEST67 test GrB_apply

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n---------------------------- quick test of GrB_apply\n') ;

A = sparse (rand (4,3)) ;
C = sparse (rand (4,3)) ;

C0 = GB_mex_apply (C, [ ], '', 'ainv', A) ;
C1 = -A ;
assert (isequal (full (C0.matrix), C1))

% compile time; requires User/my_scale.m4 to be present
ok = true ;
try
    % value of my_scalar in GB_mex_apply is 2
    C0 = GB_mex_apply (C, [ ], '', 'my_scale', A) ;
    C1 = 2*A ;
    ok = isequal (full (C0.matrix), C1) ;
    fprintf ('user compile-time my_scale.m4 is available\n') ;
catch
    fprintf ('user compile-time my_scale.m4 not available\n') ;
end
assert (ok) ;

C0 = GB_mex_apply (C, [ ], 'plus', 'identity', A) ;
C1 = C + A ;
assert (isequal (full (C0.matrix), C1))

C0 = GB_mex_apply (C, [ ], 'times', 'minv', A) ;
C1 = C .* (1./ A) ;
assert (isequal (full (C0.matrix), C1))

d = struct ('inp0', 'tran') ;
C0 = GB_mex_apply (C', [ ], 'times', 'ainv', A, d) ;
C1 = C' .* (-A') ;
assert (isequal (full (C0.matrix), C1))

A = sparse (zeros (4,3)) ;
C0 = GB_mex_apply (A', [ ], 'plus', 'ainv', A, d) ;
assert (nnz (C0.matrix) == 0)
C0 = GB_mex_apply (A, [ ], '', 'ainv', A) ;
assert (nnz (C0.matrix) == 0)

fprintf ('\ntest67: all tests passed\n') ;

