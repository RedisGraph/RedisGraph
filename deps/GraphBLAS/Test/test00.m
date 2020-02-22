function test00
%TEST00 test GB_mex_mis

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\ntest00: MIS\n') ;
rng ('default') ;

for n = 1:10
    for d = 0:.1:1
        for k = 1:100
            % A must be symmetric, with no diagonal
            A = sprand (n, n, d) ;
            A = A+A' ;
            A = tril (A, -1) ;
            A = A+A' ;
            % find a maximal independent set
            s = GB_mex_mis (A) ;
            p = find (s == 1) ;
            S = A (p,p) ;
            % make sure it's independent
            assert (nnz (S) == 0)
        end
    end
end

fprintf ('\ntest00: all tests passed\n') ;

