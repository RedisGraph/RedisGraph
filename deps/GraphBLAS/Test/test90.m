function test90
%TEST90 test AxB with user-defined semirings: plus_rdiv and plus_rdiv2

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n -------------- A*B plus_rdiv (user-defined semiring)\n') ;

    % 1001: Gustavson
    % 1002: heap
    % 1003: dot
    % 1004: hash
    % 1005: saxpy

rng ('default') ;

for N = [10 100] % 1000]

    N

    % create the problem
    A = sprand (4*N, 5*N, 0.01) ;
    B = sprand (5*N, 3*N, 0.01) ;

    [i j x] = find (A) ;
    [m n] = size (A) ;
    X = sparse (i, j, 1./x, m, n) ;
    clear i j x

    fprintf ('\nmatlab:  ') ;
    tic
    C1 = X*B ;
    toc

    fprintf ('GrB num: ') ;
    tic
    C2 = GB_mex_rdiv (A, B) ;
    toc
    t = grbresults ; fprintf ('GB time %g\n', t) ;
    assert (norm (C1-C2,1) / norm (C1,1) < 1e-10) ;

    for method = 1001:1005
        fprintf ('method: %d\n', method) ;
        cprint = (N <= 10) ;
        tic
        C2 = GB_mex_rdiv (A, B, method, cprint) ;
        toc
        t = grbresults ; fprintf ('GB time %g\n', t) ;
        assert (norm (C1-C2,1) / norm (C1,1) < 1e-10) ;
    end

    % try rdiv2, which typecasts B to single precision first
    fprintf ('GrB num: ') ;
    tic
    C3 = GB_mex_rdiv2 (A, B) ;
    toc
    assert (norm (C1-C3,1) / norm (C1,1) < 1e-5) ;

    [i j x] = find (B) ;
    [m n] = size (B) ;
    Y = sparse (i, j, 1./x, m, n) ;

    fprintf ('\nmatlab:  ') ;
    tic
    C4 = A*Y ;
    toc

    % try rdiv2, with flipxy, which inverts A instead of B
    fprintf ('GrB num: ') ;
    tic
    C5 = GB_mex_rdiv2 (A, B, false, false, 0, 1) ;
    toc
    assert (norm (C4-C5,1) / norm (C4,1) < 1e-5) ;

    %--------------------------------------------------------------------------
    fprintf ('\nextensive tests:\n') ;

    A = sprand (n, n, 0.01) ;
    B = sprand (n, n, 0.01) ;
    [i j x] = find (A) ;
    X = sparse (i, j, 1./x, n, n) ;
    [i j x] = find (B) ;
    Y = sparse (i, j, 1./x, n, n) ;

    flipxy = 0 ;
    at = 0 ; 
    bt = 0 ;
    fprintf ('\nmatlab:\n') ;
    tic
    C0 = X*B ;
    toc
    for method = 1001:1005
        fprintf ('method %d\n', method) ;
        tic
        C5 = GB_mex_rdiv2 (A, B, at, bt, method, flipxy) ;
        % toc
        t = grbresults ; fprintf ('GB time %g\n', t) ;
        assert (norm (C0-C5,1) / norm (C5,1) < 1e-5) ;
    end

    flipxy = 0 ;
    at = 1 ; 
    bt = 0 ;
    fprintf ('\nmatlab:\n') ;
    tic
    C0 = X'*B ;
    toc
    for method = 1001:1005
        fprintf ('method %d\n', method) ;
        tic
        C5 = GB_mex_rdiv2 (A, B, at, bt, method, flipxy) ;
        % toc
        t = grbresults ; fprintf ('GB time %g\n', t) ;
        assert (norm (C0-C5,1) / norm (C5,1) < 1e-5) ;
    end

    flipxy = 0 ;
    at = 0 ; 
    bt = 1 ;
    fprintf ('\nmatlab:\n') ;
    tic
    C0 = X*B' ;
    toc
    for method = 1001:1005
        fprintf ('method %d\n', method) ;
        tic
        C5 = GB_mex_rdiv2 (A, B, at, bt, method, flipxy) ;
        % toc
        t = grbresults ; fprintf ('GB time %g\n', t) ;
        assert (norm (C0-C5,1) / norm (C5,1) < 1e-5) ;
    end

    flipxy = 0 ;
    at = 1 ; 
    bt = 1 ;
    fprintf ('\nmatlab:\n') ;
    tic
    C0 = X'*B' ;
    toc
    for method = 1001:1005
        fprintf ('method %d\n', method) ;
        tic
        C5 = GB_mex_rdiv2 (A, B, at, bt, method, flipxy) ;
        % toc
        t = grbresults ; fprintf ('GB time %g\n', t) ;
        assert (norm (C0-C5,1) / norm (C5,1) < 1e-5) ;
    end

    flipxy = 1 ;
    at = 0 ; 
    bt = 0 ;
    fprintf ('\nmatlab:\n') ;
    tic
    C0 = A*Y ;
    toc
    for method = 1001:1005
        fprintf ('method %d\n', method) ;
        tic
        C5 = GB_mex_rdiv2 (A, B, at, bt, method, flipxy) ;
        % toc
        t = grbresults ; fprintf ('GB time %g\n', t) ;
        assert (norm (C0-C5,1) / norm (C5,1) < 1e-5) ;
    end

    flipxy = 1 ;
    at = 1 ; 
    bt = 0 ;
    fprintf ('\nmatlab:\n') ;
    tic
    C0 = A'*Y ;
    toc
    for method = 1001:1005
        fprintf ('method %d\n', method) ;
        tic
        C5 = GB_mex_rdiv2 (A, B, at, bt, method, flipxy) ;
        % toc
        t = grbresults ; fprintf ('GB time %g\n', t) ;
        assert (norm (C0-C5,1) / norm (C5,1) < 1e-5) ;
    end

    flipxy = 1 ;
    at = 0 ; 
    bt = 1 ;
    fprintf ('\nmatlab:\n') ;
    tic
    C0 = A*Y' ;
    toc
    for method = 1001:1005
        fprintf ('method %d\n', method) ;
        tic
        C5 = GB_mex_rdiv2 (A, B, at, bt, method, flipxy) ;
        % toc
        t = grbresults ; fprintf ('GB time %g\n', t) ;
        assert (norm (C0-C5,1) / norm (C5,1) < 1e-5) ;
    end

    flipxy = 1 ;
    at = 1 ; 
    bt = 1 ;
    fprintf ('\nmatlab:\n') ;
    tic
    C0 = A'*Y' ;
    toc
    for method = 1001:1005
        fprintf ('method %d\n', method) ;
        tic
        C5 = GB_mex_rdiv2 (A, B, at, bt, method, flipxy) ;
        % toc
        t = grbresults ; fprintf ('GB time %g\n', t) ;
        assert (norm (C0-C5,1) / norm (C5,1) < 1e-5) ;
    end

end
fprintf ('\ntest90: all tests passed\n') ;

