function testca
%TESTCA test complex mxm, mxv, and vxm

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;
dnt = struct ('inp1', 'tran') ;
dtn = struct ('inp0', 'tran') ;
dtt = struct ('inp0', 'tran', 'inp1', 'tran') ;

seed = 1 ;
for m = [1 5 10 100]
    for n = [1 5 10 100]
        for k = [1 5 10 100]
            for trial = 1:30

                A = GB_mex_random (m, k, 10*(m+k), 1, seed) ; seed = seed + 1 ;
                B = GB_mex_random (k, n, 10*(k+n), 1, seed) ; seed = seed + 1 ;
                S = GB_mex_complex (sparse (m,n)) ;
                D = GB_mex_random (m, n, 10*(m+n), 1, seed) ; seed = seed + 1 ;

                C = A*B ;
                C2 = GB_mex_mxm (S, [], [], [], A, B, []) ;
                assert (isequal (C, C2.matrix)) ;

                if (n == 1)
                    C2 = GB_mex_mxv (S, [], [], [], A, B, []) ;
                    assert (isequal (C, C2.matrix)) ;

                    C2 = GB_mex_vxm (S, [], [], [], B, A.', []) ;
                    assert (isequal (C, C2.matrix)) ;
                end

                if (m > 1 && n > 1)
                    t = min (m,n) ;
                    Eye = speye (m,n) ;
                    C = A*B ;
                    C2 = GB_mex_mxm (S, Eye, [], [], A, B, []) ;
                    d = full (diag (C)) ;
                    d = GB_mex_complex (sparse (1:t, 1:t, d, m, n)) ;
                    assert (isequal (d, C2.matrix)) ;
                end

                C = D + A*B ;
                C2 = GB_mex_mxm (D, [], 'plus', [], A, B, []) ;
                assert (isequal (C, C2.matrix)) ;

                if (n == 1)
                    C2 = GB_mex_mxv (D, [], 'plus', [], A, B, []) ;
                    assert (isequal (C, C2.matrix)) ;
                end

                C2 = GB_mex_mxm (D, [], 'plus', [], A, B.', dnt)  ;
                assert (isequal (C, C2.matrix)) ;

                C2 = GB_mex_mxm (D, [], 'plus', [], A.', B, dtn)  ;
                assert (isequal (C, C2.matrix)) ;

                if (n == 1)
                    C2 = GB_mex_mxv (D, [], 'plus', [], A.', B, dtn)  ;
                    assert (isequal (C, C2.matrix)) ;
                end

                C2 = GB_mex_mxm (D, [], 'plus', [], A.', B.', dtt)  ;
                assert (isequal (C, C2.matrix)) ;

                M = spones (sprand (m, n, 0.5)) ;
                C2 = GB_mex_mxm (S, M, [ ], [], A.', B.', dtt)  ;
                C = (A*B) .* M ;
                assert (isequal (C, C2.matrix)) ;

            end
        end
    end
end

fprintf ('testca: all complex mxm, mxv, vxm tests passed\n') ;

