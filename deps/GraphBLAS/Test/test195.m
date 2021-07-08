function test195
%TEST195 test all variants of saxpy3

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

k = 3 ;
n = 4 ;
m = 200 ;
desc.axb   = 'hash' ;
desc_s.axb = 'hash' ; desc_s.mask = 'structural' ;
dnot.axb   = 'hash' ; dnot.mask = 'complement' ;
dnot_s.axb = 'hash' ; dnot_s.mask = 'structural complement' ;

semiring.multiply = 'times' ;
semiring.add = 'plus' ;
semiring.class = 'double' ;

for asparsity = [1 2 4 8]
    fprintf ('\nA: %s ', GB_sparsity (asparsity)) ;
    for bsparsity = [1 2 4 8]
        fprintf ('\n    B: %s ', GB_sparsity (bsparsity)) ;
        for msparsity = [1 2 4 8]
            fprintf ('\n        M: %s ', GB_sparsity (msparsity)) ;
            for da = [0.01 .1 inf]
                A = GB_spec_random (m, k, da) ; A.sparsity = asparsity ;
                for db = [0.01 .1 inf]
                    B = GB_spec_random (k, n, db) ; B.sparsity = bsparsity ;
                    for dm = [0.01 .1 inf]
                        fprintf ('.') ;
                        M = GB_spec_random (m, n, dm) ; M.sparsity = msparsity ;
                        M.matrix = spones (M.matrix) ;
                        C0 = sparse (n, n) ;

                        % C = A*B
                        C0 = A.matrix * B.matrix ;
                        C1 = GB_spec_mxm (C0, [ ], [ ], semiring, A, B, desc) ;
                        C2 = GB_mex_mxm  (C0, [ ], [ ], semiring, A, B, desc) ;
                        GB_spec_compare (C1, C2, 0, 1e-12) ;
                        err = norm (C0 - C2.matrix, 1) ;
                        assert (err < 1e-12) ;

                        % C<M> = A*B
                        C0 = (A.matrix * B.matrix) .* M.matrix ;
                        C1 = GB_spec_mxm (C0, M, [ ], semiring, A, B, desc) ;
                        C2 = GB_mex_mxm  (C0, M, [ ], semiring, A, B, desc) ;
                        GB_spec_compare (C1, C2, 0, 1e-12) ;
                        err = norm (C0 - C2.matrix, 1) ;
                        assert (err < 1e-12) ;

                        % C<!M> = A*B
                        C0 = (A.matrix * B.matrix) .* (1 - M.matrix) ;
                        C1 = GB_spec_mxm (C0, M, [ ], semiring, A, B, dnot) ;
                        C2 = GB_mex_mxm  (C0, M, [ ], semiring, A, B, dnot) ;
                        GB_spec_compare (C1, C2, 0, 1e-12) ;
                        err = norm (C0 - C2.matrix, 1) ;
                        assert (err < 1e-12) ;

                        % C<M,struct> = A*B
                        C0 = (A.matrix * B.matrix) .* M.matrix ;
                        C1 = GB_spec_mxm (C0, M, [ ], semiring, A, B, desc_s) ;
                        C2 = GB_mex_mxm  (C0, M, [ ], semiring, A, B, desc_s) ;
                        GB_spec_compare (C1, C2, 0, 1e-12) ;
                        err = norm (C0 - C2.matrix, 1) ;
                        assert (err < 1e-12) ;

                        % C<!M,struct> = A*B
                        C0 = (A.matrix * B.matrix) .* (1 - M.matrix) ;
                        C1 = GB_spec_mxm (C0, M, [ ], semiring, A, B, dnot_s) ;
                        C2 = GB_mex_mxm  (C0, M, [ ], semiring, A, B, dnot_s) ;
                        GB_spec_compare (C1, C2, 0, 1e-12) ;
                        err = norm (C0 - C2.matrix, 1) ;
                        assert (err < 1e-12) ;

                    end
                end
            end
        end
    end
end

fprintf ('\ntest195: all tests passed\n') ;
