function test144
%TEST144 test GB_cumsum

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('test144 ---------------------- test GB_cumsum\n') ;

rng ('default') ;

n = 1e5 ;

c = int64 (50 * rand (1,n)) ;

for nthreads = 1:8

    % fprintf ('nthreads: %d\n', nthreads) ;

    for nmalloc = 0:2

        [p1, k1] = GB_mex_cumsum (c, nthreads, nmalloc) ;
        p = cumsum ([0 c]) ;
        k = sum (c ~= 0) ;
        assert (isequal (p, p1)) ;
        assert (k == k1) ;

        p1 = GB_mex_cumsum (c, nthreads, nmalloc) ;
        p = cumsum ([0 c]) ;
        assert (isequal (p, p1)) ;

    end
end

fprintf ('test144: all tests passed\n') ;

