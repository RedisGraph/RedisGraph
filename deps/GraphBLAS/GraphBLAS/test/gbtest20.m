function gbtest20
%GBTEST20 test bandwidth, isdiag, ceil, floor, round, fix

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;
for trial = 1:10
    fprintf ('.') ;
    for m = 0:10
        for n = 0:10
            A = 100 * sprand (m, n, 0.5) ;
            G = GrB (A) ;
            [lo1, hi1] = bandwidth (A) ;
            [lo2, hi2] = bandwidth (G) ;
            assert (isequal (lo1, lo2)) ;
            assert (isequal (hi1, hi2)) ;
            d1 = isdiag (A) ;
            d2 = isdiag (G) ;
            assert (isequal (d1, d2)) ;

            assert (gbtest_eq (ceil  (A), ceil  (G))) ;
            assert (gbtest_eq (floor (A), floor (G))) ;
            assert (gbtest_eq (round (A), round (G))) ;
            assert (gbtest_eq (fix   (A), fix   (G))) ;
        end
    end
end

fprintf ('\ngbtest20: all tests passed\n') ;

