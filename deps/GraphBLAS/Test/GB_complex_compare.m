function GB_complex_compare (C1, C2, tol)
%GB_COMPLEX_COMPARE compare GraphBLAS results for complex types
%
% compare two complex results, from GB_mex_op and GB_user_op

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (tol)
    if (any (any (isinf (C1))))
        assert (isequal (isinf (C1), isinf (C2)))
        C1 (isinf (C1)) = 1i ;
        C2 (isinf (C2)) = 1i ;
    end
    if (any (any (isnan (C1))))
        assert (isequal (isnan (C1), isnan (C2)))
        C1 (isnan (C1)) = 1i ;
        C2 (isnan (C2)) = 1i ;
    end
    assert (norm (C1 - C2,1) < 16 * eps (norm (C2,1)))
else
    assert (isequal (C1, C2)) 
end


