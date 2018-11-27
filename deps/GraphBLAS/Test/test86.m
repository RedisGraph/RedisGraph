function test86
%TEST86 performance test of of GrB_Matrix_extract

Prob = ssget (2662)

A = Prob.A ;
n = size (A,1) ;

for inc = [1:10 16 64 128 256 1024 100000 1e6 2e6]
    fprintf ('C = A (1:%7d:n, 1:%7d:n) ', inc, inc) ;
    tic
    C = A (1:inc:n, 1:inc:n) ;
    t1 = toc ;
    fprintf ('MATLAB %12g ', t1) ;
    I.begin = 0 ;
    I.inc = inc ;
    I.end = n-1 ;
    [cm cn] = size (C) ;
    S = sparse (cm, cn) ;
    tic
    C2 = GB_mex_Matrix_extract (S, [ ], [ ], A, I, I) ;
    t2 = gbresults ;
    assert (isequal (C, C2.matrix)) ;
    fprintf ('GraphBLAS %12g speedup %10g\n', t2, t1/t2) ;
end

fprintf ('\n') ;

for hi = [1:10 16 64 128 256 1024 100000 1e6 2e6]
    fprintf ('C = A (1:%7d, 1:%7d) ', hi, hi) ;
    tic
    C = A (1:hi, 1:hi) ;
    t1 = toc ;
    fprintf ('MATLAB %12g ', t1) ;
    I.begin = 0 ;
    I.inc = 1 ;
    I.end = hi-1 ;
    [cm cn] = size (C) ;
    S = sparse (cm, cn) ;
    tic
    C2 = GB_mex_Matrix_extract (S, [ ], [ ], A, I, I) ;
    t2 = gbresults ;
    assert (isequal (C, C2.matrix)) ;
    fprintf ('GraphBLAS %12g speedup %10g\n', t2, t1/t2) ;
end

fprintf ('\n') ;

for lo = [1:10 16 64 128 256 1024 100000 1e6 2e6]
    hi = lo + 10000 ;
    fprintf ('C = A (%7d:%7d, %7d:%7d) ', lo, hi, lo, hi) ;
    tic
    C = A (lo:hi, lo:hi) ;
    t1 = toc ;
    fprintf ('MATLAB %12g ', t1) ;
    I.begin = lo-1 ;
    I.inc = 1 ;
    I.end = hi-1 ;
    [cm cn] = size (C) ;
    S = sparse (cm, cn) ;
    tic
    C2 = GB_mex_Matrix_extract (S, [ ], [ ], A, I, I) ;
    t2 = gbresults ;
    assert (isequal (C, C2.matrix)) ;
    fprintf ('GraphBLAS %12g speedup %10g\n', t2, t1/t2) ;
end

fprintf ('\n') ;

for lo = [1:10 16 64 128 256 1024 100000 1e6 2e6]
    hi = lo + 10000 ;
    fprintf ('C = A (%7d:-1:%7d, %7d:-1:%7d) ', hi, lo, hi, lo) ;
    tic
    C = A (hi:-1:lo, hi:-1:lo) ;
    t1 = toc ;
    fprintf ('MATLAB %12g ', t1) ;
    I.begin = hi-1 ;
    I.inc = -1 ;
    I.end = lo-1 ;
    [cm cn] = size (C) ;
    S = sparse (cm, cn) ;
    tic
    C2 = GB_mex_Matrix_extract (S, [ ], [ ], A, I, I) ;
    t2 = gbresults ;
    assert (isequal (C, C2.matrix)) ;
    fprintf ('GraphBLAS %12g speedup %10g\n', t2, t1/t2) ;
end

fprintf ('\n') ;

for inc = [1:10 16 64 128 256 1024 100000 1e6 2e6]
    fprintf ('C = A (n:%7d:1, n:%7d:1) ', -inc, -inc) ;
    tic
    C = A (n:(-inc):1, n:(-inc):1) ;
    t1 = toc ;
    fprintf ('MATLAB %12g ', t1) ;
    I.begin = n-1 ;
    I.inc = -inc ;
    I.end = 0 ;
    [cm cn] = size (C) ;
    S = sparse (cm, cn) ;
    tic
    C2 = GB_mex_Matrix_extract (S, [ ], [ ], A, I, I) ;
    t2 = gbresults ;
    assert (isequal (C, C2.matrix)) ;
    fprintf ('GraphBLAS %12g speedup %10g\n', t2, t1/t2) ;
end


