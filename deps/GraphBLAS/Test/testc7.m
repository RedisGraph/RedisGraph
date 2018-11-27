function testc7
%TESTC7 test complex assign

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default')

dclear.outp = 'replace' ;
dclear.mask = 'scmp' ;

seed = 1 ;
for m = [1 5 10 50]
    for n = [1 5 10 50]
        seed = seed + 1 ;
        C = GB_mex_random (m, n, 10*(m+n), 1, seed) ;
        for ni = 1:m
            for nj = 1:n
                I = randperm (m, ni) ;
                J = randperm (n, nj) ;
                seed = seed + 1 ;
                A = GB_mex_random (ni, nj, 2*(ni+nj), 1, seed) ;
                seed = seed + 1 ;
                M = GB_mex_random (ni, nj, 4*(ni+nj), 0, seed) ;
                C1 = C ;
                C1 (I,J) = A ;

                I0 = uint64 (I-1) ;
                J0 = uint64 (J-1) ;

                C2 = GB_mex_subassign (C, [ ], [ ], A, I0, J0, []) ;
                assert (isequal (C1, C2.matrix)) ;

                [C3,c1] = GB_mex_subassign (C, M, [ ], A, I0, J0, [], 'plus') ;
                cin = complex (0,0) ;
                c2 = GB_mex_reduce_to_scalar (cin, '', 'plus', C3) ;
                assert (isequal (c1,c2)) ;

                C1 = C ;
                C1 (I,J) = C1 (I,J) + A ;

                C2 = GB_mex_subassign (C, [ ], 'plus', A, I0, J0, []) ;
                assert (isequal (C1, C2.matrix)) ;

            end
            fprintf ('.') ;
        end

        C = GB_mex_random (m, n, 100*(m*n), 1, seed) ; seed = seed + 1 ;
        M = GB_mex_random (m, n, 4*(ni+nj), 0, seed) ; seed = seed + 1 ;
        A = GB_mex_random (m, n, m+n, 1, seed) ;       seed = seed + 1 ;
        [C3,c1] = GB_mex_subassign (C, M, [ ], A, [ ], [ ], dclear, 'plus') ;
        cin = complex (0,0) ;
        c2 = GB_mex_reduce_to_scalar (cin, '', 'plus', C3) ;
        assert (isequal (c1,c2)) ;

        [C3,c1] = GB_mex_subassign (C, [ ], [ ], A, [ ], [ ], dclear, 'plus') ;
        cin = complex (0,0) ;
        c2 = GB_mex_reduce_to_scalar (cin, '', 'plus', C3) ;
        assert (isequal (c1,c2)) ;


    end
end

fprintf ('\ntestc7: all complex assign C(I,J)=A tests passed\n') ;

