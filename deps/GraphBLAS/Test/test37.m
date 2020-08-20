function test37
%TEST37 performance test of qsort

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n------------ testing GB_mex_qsort performance\n') ;

% [save save_chunk] = nthreads_get ;
% chunk = 4096 ;
% nthreads = feature ('numcores') ;
% nthreads_set (nthreads, chunk) ;

rng ('default')  ;

for n = [1 10 19 20 21 100 1000 1e5 1e6 1e7 1e8]

    n
    ntrials = 2 ;
    for trials = 1:2
        I = int64 (10 * n * rand (1,n)) ;

        tic
        J1 = GB_mex_qsort (I) ;
        t1 = toc ;
        t1b = grbresults ;

        tic
        J4 = sort (I) ;
        t4 = toc ;

        assert (isequal (J1, J4)) ;

        fprintf ('q: %12.6f %12.6f MATLAB: %12.6f', t1, t1b, t4);
        fprintf (' [ %8.4f]\n', t1/t4) ;

        I = J4 ;

        tic
        J1 = GB_mex_qsort (I) ;
        t1 = toc ;
        t1b = grbresults ;

        tic
        J4 = sort (I) ;
        t4 = toc ;

        fprintf ('q: %12.6f %12.6f MATLAB: %12.6f', t1, t1b, t4);
        fprintf (' [ %8.4f] already sorted\n', t1/t4) ;

    end
end

% nthreads_set (save, save_chunk) ;

fprintf ('\ntest37: all tests passed\n') ;

