function test36
%TEST36 performance test of matrix subref

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\ntest36 --------------------- performance of GB_Matrix_subref\n') ;

rng ('default') ;
n = 100e6 ;
fprintf ('-------------------------- column vector (%d-by-1):\n', n) ;
V = sprand (n, 1, 0.01) ;
J = uint64 (0) ;
[i j x] = find (V) ;
nz = length (i) ;


fprintf (' V(:,1): nnz (V) = %d\n', nnz (V)) ;
    tic
    C0 = V (:,1) ;
    t0 = toc ;
    tic
    C1 = GB_mex_Matrix_subref (V, [ ], [ ]) ;
    t1 = toc ;
    assert (isequal (C0, C1)) ;
    fprintf ('MATLAB %0.6f GrB: %0.6f  speedup %g\n', t0, t1, t0/t1) ;

fprintf (' V(50e6:80e6,1) explicit list:\n') ;
I = uint64 (50e6:80e6) ;
I1 = I+1 ;
    tic
    C0 = V (I1,1) ;
    t0 = toc ;
    tic
    C1 = GB_mex_Matrix_subref (V, I, [ ]) ;
    t1 = toc ;
    assert (isequal (C0, C1)) ;
    fprintf ('MATLAB %0.6f GrB: %0.6f  speedup %g\n', t0, t1, t0/t1) ;

fprintf (' V(50e6:80e6,1) colon:\n') ;
clear I
I.begin = 50e6-1 ; I.inc = 1 ; I.end = 80e6-1  ;
% I1 = I+1 ;
    tic
    C0 = V (50e6:80e6,1) ;
    t0 = toc ;
    tic
    C1 = GB_mex_Matrix_subref (V, I, [ ]) ;
    t1 = toc ;
    assert (isequal (C0, C1)) ;
    fprintf ('MATLAB %0.6f GrB: %0.6f  speedup %g\n', t0, t1, t0/t1) ;

I1 = i (floor (nz/2)) ;
I = uint64 (I1)-1 ;
fprintf (' V(%d,1):\n', I1) ;

    C0 = V (I1,1) ;
    tic
    C0 = V (I1,1) ;
    t0 = toc ;
    C1 = GB_mex_Matrix_subref (V, I, J) ;
    tic
    C1 = GB_mex_Matrix_subref (V, I, J) ;
    t1 = toc ;
    assert (isequal (C0, C1)) ;
    fprintf ('MATLAB %0.6f GrB: %0.6f  speedup %g\n', t0, t1, t0/t1) ;

    tic
    C0 = V (I1,1) ;
    t0 = toc ;
    tic
    C1 = GB_mex_Matrix_subref (V, I, J) ;
    t1 = toc ;
    assert (isequal (C0, C1)) ;
    fprintf ('MATLAB %0.6f GrB: %0.6f  speedup %g\n', t0, t1, t0/t1) ;


fprintf (' V( 100 entries ,1):\n') ;
p = randperm (nz) ;
I1 = i (p (1:100)) ;
I = uint64 (I1)-1 ;

    tic
    C0 = V (I1,1) ;
    t0 = toc ;
    tic
    C1 = GB_mex_Matrix_subref (V, I, [ ] ) ;
    t1 = toc ;
    assert (isequal (C0, C1)) ;
    fprintf ('MATLAB %0.6f GrB: %0.6f  speedup %g\n', t0, t1, t0/t1) ;


fprintf (' V( 100 entries ,1:4):\n') ;
V = [V V V V] ;

    tic
    C0 = V (I1,:) ;
    t0 = toc ;
    tic
    C1 = GB_mex_Matrix_subref (V, I, [ ]) ;
    t1 = toc ;
    assert (isequal (C0, C1)) ;
    fprintf ('MATLAB %0.6f GrB: %0.6f  speedup %g\n', t0, t1, t0/t1) ;


fprintf ('many single entries:\n') ;
V = V (:,1) ;
p = randperm (n) ;
x = 0 ;
tic
    for k = 1:1e5
        x = x + V (p(k)) ;
    end
t0 = toc ;

global GraphBLAS_results

% don't include the mexFunction overhead
p = uint64 (p) - 1 ;
y = 0 ;
t1 = 0 ;
tic
    for k = 1:1e5
        y = y + GB_mex_Matrix_subref (V, p(k), [ ]) ;
        t1 = t1 + gbresults ;
        GraphBLAS_results (1) = 0 ;
    end
t1 = toc ;
fprintf ('MATLAB %0.6f GrB: %0.6f  speedup %g\n', t0, t1, t0/t1) ;
assert (isequal (x,y))

fprintf ('\ntest36: all tests passed\n') ;

