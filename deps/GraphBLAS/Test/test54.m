function test54
%TEST54 test AxB, qsort vs bucket sort

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n======================= qsort vs C transpose for C=A*B\n') ;

index = ssget ;
f = find (index.nrows == index.ncols) ;
[ignore i] = sort (index.ncols (f)) ;
f = f (i) ;
nmat = length (f) ;

T = zeros (nmat,8) ;
Nz = zeros (nmat,1) ;

for k = 1:nmat
    id = f (k) ;
    Prob = ssget (id, index) ;
    A = Prob.A ;
    if (~isreal (A))
        A = real (A) ;
    end
    B = A' ;
    n = size (A,1) ;
    if (max (sum (spones (B))) > 16*sqrt(n))
        continue ;
    end
    Nz (k) = nnz (A) ;

    % C = A*B or (B'*A')'
    tic
    C1 = GB_mex_AxB_symbolic (A, B, false, false, false) ;
    T(k,1) = toc ;
    tic
    C2 = GB_mex_AxB_symbolic (B, A, true, true, true) ;
    T(k,2) = toc ;
    assert (isequal (C1, C2)) ;

    % C = A'*B or (B'*A)'
    tic
    C1 = GB_mex_AxB_symbolic (A, B, true, false, false) ;
    T(k,3) = toc ;
    tic
    C2 = GB_mex_AxB_symbolic (B, A, true, false, true) ;
    T(k,4) = toc ;
    assert (isequal (C1, C2)) ;

    % C = A*B' or (B*A')'
    tic
    C1 = GB_mex_AxB_symbolic (A, B, false, true, false) ;
    T(k,5) = toc ;
    tic
    C2 = GB_mex_AxB_symbolic (B, A, false, true, true) ;
    T(k,6) = toc ;
    assert (isequal (C1, C2)) ;

    % C = A'*B' or (B*A)'
    tic
    C1 = GB_mex_AxB_symbolic (A, B, true, true, false) ;
    T(k,7) = toc ;
    tic
    C2 = GB_mex_AxB_symbolic (B, A, false, false, true) ;
    T(k,8) = toc ;
    assert (isequal (C1, C2)) ;

    fprintf ('%10.3f   %10.3f   %10.3f   %10.3f\n', ...
        T (k,1) ./ T (k,2), ...
        T (k,3) ./ T (k,4), ...
        T (k,5) ./ T (k,6), ...
        T (k,7) ./ T (k,8)) ;

    subplot (2,2,1) ;
    loglog (Nz (1:k), T (1:k,1) ./ T (1:k,2), 'o', [1 10e6], [1 1], 'k-') ;

    subplot (2,2,2) ;
    loglog (Nz (1:k), T (1:k,3) ./ T (1:k,4), 'o', [1 10e6], [1 1], 'k-') ;

    subplot (2,2,3) ;
    loglog (Nz (1:k), T (1:k,5) ./ T (1:k,6), 'o', [1 10e6], [1 1], 'k-') ;

    subplot (2,2,4) ;
    loglog (Nz (1:k), T (1:k,7) ./ T (1:k,8), 'o', [1 10e6], [1 1], 'k-') ;

    drawnow
end

fprintf ('\ntest54: all tests passed\n') ;

