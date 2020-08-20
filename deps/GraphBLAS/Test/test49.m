function test49
%TEST49 performance test of GrB_mxm (dot product method, A'*B)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[save save_chunk] = nthreads_get ;
chunk = 4096 ;
nthreads = feature ('numcores') ;
nthreads_set (nthreads, chunk) ;

d = struct ('inp0', 'tran', 'axb', 'dot') ;

rng ('default') ;
k = 1e6 ;

semiring.multiply = 'times' ;
semiring.add = 'plus' ;
semiring.class = 'double' ;

A1 = sprand (k, 16, 10e6 / (k*16)) ;
B1 = sprand (k, 16, 10e6 / (k*16)) ;

for m = 1:4
    for n = 1:4

        A = A1 (:, 1:m) ;
        B = B1 (:, 1:n) ;

        W = sparse (m, n) ;

        tic ;
        C = A'*B ;
        t1 = toc  ;

        tic ;
        C2 = GB_mex_mxm (W, [], [], semiring, A, B, d) ;
        % t2 = toc ;
        t2 = grbresults ;

        e = norm (C - C2.matrix, 1) ;
        fprintf (...
       'm %3d n %3d MATLAB: %10.5g  GrB: %10.5g  speedup %10.2f  err: %g\n', ...
           m, n, t1, t2, t1/t2, e) ;
    end
end

nthreads_set (save, save_chunk) ;

fprintf ('\ntest49: all tests passed\n') ;

