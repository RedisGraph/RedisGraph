function test215
%TEST215 test C<M>=A'*B (dot2, ANY_PAIR semiring)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

rng ('default') ;

GrB.burble (1) ;

n = 100 ;
Cin = sparse (n, n) ;

M.matrix = logical (sprand (n, n, 0.05)) ;
M.sparsity = 4 ;    % M bitmap
desc = struct ('mask', 'structural complement', 'inp0', 'tran') ;

A.matrix = sprandn (n, n, 0.05) ;
A.class  = 'double' ;
A.sparsity = 2 ;    % A sparse

B.matrix = sprandn (n, n, 0.05) ;
B.class  = 'double' ;
B.sparsity = 4 ;    % B bitmap

semiring.add = 'any' ;          % ANY monoid
semiring.multiply = 'pair' ;
semiring.class = 'double' ;

C1 = GB_mex_mxm  (Cin, M, [ ], semiring, A, B, desc) ;
C2 = GB_spec_mxm (Cin, M, [ ], semiring, A, B, desc) ;
GB_spec_compare (C1, C2) ;

GrB.burble (0) ;
fprintf ('\ntest215: all tests passed\n') ;

