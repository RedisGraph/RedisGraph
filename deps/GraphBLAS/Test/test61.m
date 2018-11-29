function test61
%TEST61 performance test of GrB_eMult

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n----------------------------- eMult performance tests\n') ;

Prob = ssget (2662)
A = Prob.A ;
[m n] = size (A) ;

B = A ;
A (:, 1:5) = 44 ;

S = sparse (m,n) ;
fprintf ('\n\nm: %d n %d nnz(A) %d\n', m, n,  nnz (A)) ;

d = nnz (A) / prod (size (A)) ;

    tic
    C = A .*B ;
    t1 = toc ;

    tic
    C2 = GB_mex_eWiseMult_Matrix (S, [ ], [ ], 'times', A, B, [ ]) ;
    t2 = toc ;

    fprintf (...
    'd %10.6g nnz(C) %8d MATLAB %10.6f GB %10.6f  speedup %10.4f\n',...
        d, nnz (C), t1, t2, t1/t2) ;


A = sparse (rand (5000)) ;
[m n] = size (A) ;
S = sparse (m,n) ;
fprintf ('\n\nm: %d n %d nnz(A) %d\n', m, n,  nnz (A)) ;

for d = [0.00001:0.00001:0.0001 0.0002:0.0001: 0.001 0.002:.001:0.01 0.02:0.01:.1 1]

    B = sprandn (m, n, d) ;

    tic
    C = A .*B ;
    t1 = toc ;

    tic
    C2 = GB_mex_eWiseMult_Matrix (S, [ ], [ ], 'times', A, B, [ ]) ;
    t2 = toc ;

    fprintf (...
    'd %10.6g nnz(C) %8d MATLAB %10.6f GB %10.6f  speedup %10.4f\n',...
        d, nnz (C), t1, t2, t1/t2) ;

    assert (isequal (C, C2.matrix)) ;
end

fprintf ('\ntest61: all tests passed\n') ;

