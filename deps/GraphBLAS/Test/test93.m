function test93
%TEST93 test dpagerank and ipagerank

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;
addpath ('../Demo/MATLAB') ;

for n = [10 100 1000 1e4 1e5 ] % 1e6]

    fprintf ('\n--------------n: %d\n', n) ;
    nz = 8*n ;
    d = nz / n^2 ;
    A = sprand (n, n, d) ;
    A = spones (A + speye (n)) ;
%   A = spones (A) ;
%   A (:,1) = 0 ;
%   A (1,:) = 0 ;

    tic
    [r1, i1] = dpagerank (A) ;
    t1 = toc ;

    [r2, i2] = GB_mex_dpagerank (A) ;
    t2 = grbresults ;

    % [i1' i2']

    % results won't be identical because of different random number generators
    mismatch = length (find (i1 ~= i2)) ;
    e = norm (r1 - r2) / norm (r1) ;
    fprintf ('i1 i2 mismatch: %d\n', mismatch) ;
    fprintf ('r1-r2 = %g\n', e) ;
    fprintf ('time: MATLAB %g GraphBLAS %g speedup %g\n', t1, t2, t1/t2) ;
    % if (n <= 1e5)
    %     assert (mismatch < 100) ;
    %    end
    % assert (e < 1e-6) ;

    tic
    [r4, i4, iter4] = dpagerank2 (A) ;
    t4 = toc ;
    fprintf ('\nWith stopping criterion, iter: %d\n', iter4) ;

    C.matrix = A ;
    C.class = 'logical' ;
    C.is_csc = false ;

    [r3, i3, iter3] = GB_mex_dpagerank (C, 0) ;
    t3 = grbresults ;
    fprintf ('GraphBLAS method 2: time %g speedup %g iters %g\n', ...
        t3, t4/t3, iter3) ;
    e = norm (r2 - r3) / norm (r3) ;
    % assert (e < 1e-12) ;
    % assert (isequal (i3, i2)) ;
    mismatch = length (find (i1 ~= i3)) ;

    fprintf ('\ndpagerank2 version:\n') ;
    fprintf ('i1 i3 mismatch: %d\n', mismatch) ;
    fprintf ('r1-r3 = %g\n', e) ;

    mismatch = length (find (i3 ~= i4)) ;
    fprintf ('\ndpagerank2 version:\n') ;
    fprintf ('i3 i4 mismatch: %d\n', mismatch) ;
    fprintf ('r3-r4 = %g\n', e) ;

    % test the integer versions
    tic
    [ir1, ii1] = ipagerank (A) ;
    ti1 = toc ;

    [ir2, ii2] = GB_mex_ipagerank (A) ;
    ti2 = grbresults ;

    % [ii1' ii2']

    ir1 = ir1 / norm (ir1) ;
    ir2 = ir2 / norm (ir2) ;

    % results won't be identical because of different random number generators
    mismatch = length (find (ii1 ~= ii2)) ;
    e = norm (ir1 - ir2) / norm (ir1) ;
    fprintf ('i1 i2 mismatch: %d\n', mismatch) ;
    fprintf ('r1-r2 = %g\n', e) ;
    fprintf ('time: MATLAB %g GraphBLAS %g speedup %g\n', ti1, ti2, ti1/ti2) ;
    if (n < 1e4)
        assert (mismatch < n/10) ;
        assert (e < 1e-4) ;
    end

end

fprintf ('test93: all tests passed\n') ;

