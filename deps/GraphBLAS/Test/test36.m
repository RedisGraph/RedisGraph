function test36
%TEST36 test matrix subref

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n---------------------------- performance of GB_Matrix_subref\n') ;

rng ('default') ;
fprintf ('-------------------------- column vector:\n') ;
n = 100e6 ;
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

fprintf (' V(50e6:80e6,1):\n') ;
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

p = uint64 (p) - 1 ;
y = 0 ;
tic
    for k = 1:1e5
        y = y + GB_mex_Matrix_subref (V, p(k), [ ]) ;
    end
t1 = toc ;
fprintf ('MATLAB %0.6f GrB: %0.6f  speedup %g\n', t0, t1, t0/t1) ;
assert (isequal (x,y))

fprintf ('\ntest36: all tests passed\n') ;

