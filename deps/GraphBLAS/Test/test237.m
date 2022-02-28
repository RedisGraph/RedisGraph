function test237
%TEST237 test GrB_mxm (saxpy4)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

GrB.burble (0) ;
rng ('default') ;

n = 32 ;

for k = [1 2 4 8 32]

    A = GB_spec_random (n, n, 0.3) ;
    B = GB_spec_random (n, k, 0.3) ;
    B.sparsity = 4 ;    % bitmap
    F = GB_spec_random (n, k, inf) ;
    F.sparsity = 8 ;    % full

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
    C1 = GB_mex_mxm  (F, [ ], accum, semiring, A, B, [ ]) ;
    C2 = GB_spec_mxm (F, [ ], accum, semiring, A, B, [ ]) ;
    GB_spec_compare (C1, C2, tol) ;

    % fprintf ('\n----------------- F += H*B:\n\n') ;
    A.sparsity = 1 ;
    C1 = GB_mex_mxm  (F, [ ], accum, semiring, A, B, [ ]) ;
    C2 = GB_spec_mxm (F, [ ], accum, semiring, A, B, [ ]) ;
    GB_spec_compare (C1, C2, tol) ;

    B = GB_spec_random (n, k, inf) ;
    B.sparsity = 8 ;    % full

    % fprintf ('\n----------------- F += S*F:\n\n') ;
    A.sparsity = 2 ;
    C1 = GB_mex_mxm  (F, [ ], accum, semiring, A, B, [ ]) ;
    C2 = GB_spec_mxm (F, [ ], accum, semiring, A, B, [ ]) ;
    GB_spec_compare (C1, C2, tol) ;

    % fprintf ('\n----------------- F += H*F:\n\n') ;
    A.sparsity = 1 ;
    C1 = GB_mex_mxm  (F, [ ], accum, semiring, A, B, [ ]) ;
    C2 = GB_spec_mxm (F, [ ], accum, semiring, A, B, [ ]) ;
    GB_spec_compare (C1, C2, tol) ;
end

% k = 1 with a sparser A matrix

    A = GB_spec_random (n, n, 0.05) ;
    B = GB_spec_random (n, 1, 0.3) ;
    B.sparsity = 4 ;    % bitmap
    F = GB_spec_random (n, 1, inf) ;
    F.sparsity = 8 ;    % full

    % fprintf ('\n----------------- F += S*F:\n\n') ;
    A.sparsity = 2 ;
    C1 = GB_mex_mxm  (F, [ ], accum, semiring, A, B, [ ]) ;
    C2 = GB_spec_mxm (F, [ ], accum, semiring, A, B, [ ]) ;
    GB_spec_compare (C1, C2, tol) ;

GrB.burble (0) ;
fprintf ('test237: all tests passed\n') ;

