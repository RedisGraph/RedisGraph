function test137
%TEST137 GrB_eWiseMult with FIRST and SECOND operators

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('test137: GrB_eWiseMult with FIRST and SECOND operators\n') ;

rng ('default') ;

n = 100 ;
A = sprand (n, n, 0.1) ;
Z = sprand (n, n, 0.1) + 1i * sprand (n, n, 0.1) ;
S = spones (Z) ;
C = sparse (n, n) ;

try
    % this is an error
    C1 = GB_mex_eWiseMult_Matrix (C, [ ], [ ], 'times', A, Z, [ ]) ;
    assert (false) ;
catch
    assert (true) ;
end

C0 = A .* S ;
C1 = GB_mex_eWiseMult_first (C, [ ], [ ], [ ], A, Z, [ ]) ;
assert (isequal (C0, C1.matrix)) ;

C0 = S .* A ;
C1 = GB_mex_eWiseMult_second (C, [ ], [ ], [ ], Z, A, [ ]) ;
assert (isequal (C0, C1.matrix)) ;

fprintf ('test137: all tests passed\n') ;
