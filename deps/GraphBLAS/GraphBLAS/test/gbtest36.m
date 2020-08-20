function gbtest36
%GBTEST36 test abs, sign

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;

types = gbtest_types ;
for k = 1:length (types)
    type = types {k} ;

    A = floor (100 * (rand (3, 3) - 0.5)) ;
    A (1,1) = 0 ;

    if (type (1) == 'u')
        A = max (A, 0) ;
    end
    G = GrB (A, type) ;
    B = cast (A, type) ;
    assert (gbtest_eq (B, G))

    H = abs (G) ;
    C = abs (B) ;
    assert (gbtest_eq (double (C), double (H)))

    H = sign (G) ;
    if (isequal (type, 'logical'))
        % sign (B) is not defined for MATLAB logical matrices
        C = B ;
    else
        C = sign (B) ;
    end
    assert (gbtest_eq (C, H))

end

fprintf ('gbtest36: all tests passed\n') ;

