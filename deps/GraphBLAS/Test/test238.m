function test238
%TEST238 test GrB_mxm (dot4)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

GrB.burble (0) ;
rng ('default') ;

desc.inp0 = 'tran' ;

n = 32 ;

for k = [1 2 4 8 32]

    A = GB_spec_random (n, n, 0.3) ;
    B = GB_spec_random (n, k, 0.3) ;
    B.sparsity = 4 ;    % bitmap
    F = GB_spec_random (n, k, inf) ;
    F.sparsity = 8 ;    % full
    F.iso = false ;

    for iso = [0 1]

        accum.opname = 'plus' ;
        accum.optype = 'double' ;
        % fprintf ('\n----------------- accum:\n') ;
        % GB_mex_binaryop (accum) ;

        % fprintf ('\n----------------- semiring:\n') ;
        semiring.multiply = 'times' ;
        semiring.add = 'plus' ;
        semiring.class = 'double' ;
        % GB_mex_semiring (semiring, 3) ;

        tol = 1e-12 ;

        % fprintf ('\n----------------- F += S*B:\n\n') ;
        A.sparsity = 2 ;
        C1 = GB_mex_mxm  (F, [ ], accum, semiring, A, B, desc) ;
        C2 = GB_spec_mxm (F, [ ], accum, semiring, A, B, desc) ;
        GB_spec_compare (C1, C2, tol) ;

        % fprintf ('\n----------------- F += H*B:\n\n') ;
        A.sparsity = 1 ;
        C1 = GB_mex_mxm  (F, [ ], accum, semiring, A, B, desc) ;
        C2 = GB_spec_mxm (F, [ ], accum, semiring, A, B, desc) ;
        GB_spec_compare (C1, C2, tol) ;

        B = GB_spec_random (n, k, inf) ;
        B.sparsity = 8 ;    % full

        % fprintf ('\n----------------- F += S*F:\n\n') ;
        A.sparsity = 2 ;
        C1 = GB_mex_mxm  (F, [ ], accum, semiring, A, B, desc) ;
        C2 = GB_spec_mxm (F, [ ], accum, semiring, A, B, desc) ;
        GB_spec_compare (C1, C2, tol) ;

        % fprintf ('\n----------------- F += H*F:\n\n') ;
        A.sparsity = 1 ;
        C1 = GB_mex_mxm  (F, [ ], accum, semiring, A, B, desc) ;
        C2 = GB_spec_mxm (F, [ ], accum, semiring, A, B, desc) ;
        GB_spec_compare (C1, C2, tol) ;

        F.matrix = pi * ones (n,k) ;
        F.iso = true ;
    end
end

GrB.burble (0) ;
fprintf ('test238: all tests passed\n') ;

