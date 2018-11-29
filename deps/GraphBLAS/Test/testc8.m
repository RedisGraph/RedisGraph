function testc8
%TESTC8 test complex eWiseAdd and eWiseMult

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default')
seed = 1 ;
for m = [1 5 10 100]
    for n = [1 5 10 100]
        
        for trials = 1:100

            A = GB_mex_random (m, n, 10*(m+n), 1, seed) ; seed = seed + 1 ;
            B = GB_mex_random (m, n, 10*(m+n), 1, seed) ; seed = seed + 1 ;
            S = GB_mex_complex (sparse (m,n)) ;

            C1 = GB_mex_complex (A + B) ;
            C2 = GB_mex_eWiseAdd_Matrix (S, [], [], 'plus', A, B, []) ;
            assert (isequal (C1, C2.matrix)) ;

            if (n == 1)
                C2 = GB_mex_eWiseAdd_Vector (S, [], [], 'plus', A, B, []) ;
                assert (isequal (C1, C2.matrix)) ;
            end

            C1 = GB_mex_complex (A .* B) ;
            C2 = GB_mex_eWiseMult_Matrix (S, [], [], 'times', A, B, []) ;
            % drop explicit zeros from C2.matrix:
            assert (isequal (C1, 1*C2.matrix)) ;

            if (n == 1)
                C2 = GB_mex_eWiseMult_Vector (S, [], [], 'times', A, B, []) ;
                assert (isequal (C1, 1*C2.matrix)) ;
            end
        end
    end
end

fprintf ('testc8: all complex eWise tests passed\n') ;

