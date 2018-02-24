function test50
%TEST50 test AxB numeric and symbolic

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n----------------------------- GB_mex_AxB numeric and symbolic\n') ;

% Prob = ssget (2662) ;
Prob = ssget (936)
A = Prob.A ;
A(1,2) = 443 ;

for trial = 1:2

    if (trial == 1)
        B = A ;
    else
        % make it rectangular
        [m k] = size (A) ;
        A = A (:,1:k-1) ;
        [m k] = size (A) ;
        nz = nnz (A) ;
        n = m+1 ;
        B = sprand (k, n, nz / (k*n)) ;
    end

    fprintf ('\n---------------matlab: C=A*B\n') ;
    tic ;
    C = (A*B) ;
    toc
    C0 = cast (C, 'logical') ;
    cnorm = norm (C, 1) ;
    fprintf ('nnz(C) %d density %g mincol %d maxcol %d norm %g\n', ...
        nnz (C0), nnz (C0) / prod (size (C0)), ...
        full (min (sum (spones (C0)))), ...
        full (max (sum (spones (C0)))), cnorm) ;
    % figure (1)
    % subplot (1,3,1) ; spy (A) ;
    % subplot (1,3,2) ; spy (B) ;
    % subplot (1,3,3) ; spy (C) ;

    fprintf ('\n--- C=A*B qsort, no transpose, symbolic:\n') ;
    tic ;
    C1 = GB_mex_AxB_symbolic (A, B, false, false, false) ;
    toc
    assert (isequal (C1, C0)) ;
    clear C1

    fprintf ('\n--- C=(B''*C'')'' 3 transpose, symbolic:\n') ;
    tic ;
    C1 = GB_mex_AxB_symbolic (B, A, true, true, true) ;
    toc
    assert (isequal (C1, C0)) ;
    clear C1

    fprintf ('numerical matrix multiply:\n') ;
    tic
    S = GB_mex_AxB (A, B) ;
    toc
    assert (spok (S*1) == 1) ;
    err = norm (C-S,1) / cnorm ;
    fprintf ('err %g\n', err) ;
    assert (isequal (C, S)) ;

    fprintf ('\n---------------matlab: C=(A*B)''\n') ;
    tic ;
    C = (A*B)' ;
    toc
    C0 = cast (C, 'logical') ;

    fprintf ('\n--- C=(A*B)'' no sort needed, transpose, symbolic\n') ;
    tic ;
    C1 = GB_mex_AxB_symbolic (A, B, false, false, true) ;
    toc
    assert (isequal (C1, C0)) ;
    clear C1

    fprintf ('\n--- C=B''*A'' qsort, no transpose needed, symbolic:\n') ;
    tic ;
    C1 = GB_mex_AxB_symbolic (B, A, true, true, false) ;
    toc
    assert (isequal (C1, C0)) ;
    clear C1

end

fprintf ('\ntest50: all tests passed\n') ;

