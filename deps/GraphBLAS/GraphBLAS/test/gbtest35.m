function gbtest35
%GBTEST35 test reshape

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

rng ('default')

for m = 0:6
    for n = 0:10
        A = rand (m, n) ;
        G = GrB (A) ;
        mn = m*n ;
        if (mn == 0)
            C1 = reshape (A, n, m) ;
            C2 = reshape (G, n, m) ;
            assert (gbtest_eq (C1, C2)) ;
        else
            f = factor (mn) ;
            for k = 1:length (f)
                S = nchoosek (f, k) ;
                for i = 1:size(S,1)
                    m2 = prod (S (i,:)) ;
                    n2 = mn / m2 ;
                    C1 = reshape (A, m2, n2) ;
                    C2 = reshape (G, m2, n2) ;
                    assert (gbtest_eq (C1, C2)) ;
                    C1 = reshape (A, [m2 n2]) ;
                    C2 = reshape (G, [m2 n2]) ;
                    assert (gbtest_eq (C1, C2)) ;
                end
            end
        end
    end
end

fprintf ('gbtest35: all tests passed\n') ;
