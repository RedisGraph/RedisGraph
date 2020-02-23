%TT test eWiseMult and A+B

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

tic ;
C = A + B ;
t = toc ;
fprintf ('MATLAB C=A+B time: %g\n\n', t) ;

for k = 1:40
    nthreads_set (k) ;
    tic ;
    C2 = GB_mex_AplusB (A,B,'plus') ;
    ts (k) = toc ;
    fprintf ('threads %d time %10.4f speedup %10.2f\n', k, ts(k), ts(1)/ts(k)) ;
end

tic ;
C = A .* B ;
t = toc ;
fprintf ('MATLAB C=A.*B time: %g\n\n', t) ;

[m n] = size (A) ;
Z = sparse (m,n) ;

for k = 1:40
    nthreads_set (k) ;
    tic ;
    C2 = GB_mex_eWiseMult_Matrix (Z, [ ], [ ], 'times', A,B) ;
    ts (k) = toc ;
    fprintf ('threads %d time %10.4f speedup %10.2f\n', k, ts(k), ts(1)/ts(k)) ;
end

err = norm (C - C2.matrix, 1)
