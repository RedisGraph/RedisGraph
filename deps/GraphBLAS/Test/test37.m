function test37
%TEST37 test qsort

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n------------ testing GB_mex_qsort performance\n') ;

rng ('default')  ;

for n = [1 10 19 20 21 100 1000 1e5 1e6 1e7 1e8]

    n
    for trials = 1:10
        I = int64 (10 * n * rand (1,n)) ;

        tic
        J1 = GB_mex_qsort (I) ;
        t1 = toc ;
        t1b = gbresults ;

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
        t1b = gbresults ;

        tic
        J4 = sort (I) ;
        t4 = toc ;

        fprintf ('q: %12.6f %12.6f MATLAB: %12.6f', t1, t1b, t4);
        fprintf (' [ %8.4f] already sorted\n', t1/t4) ;

    end
end

fprintf ('\ntest37: all tests passed\n') ;

