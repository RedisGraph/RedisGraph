function test245
%TEST245 test colscale (A*D) and rowscale (D*B) with complex types

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

rng ('default') ;

n = 30 ;
A = GB_spec_random (n, n, 0.05, 256, 'double complex') ;

D.matrix = speye (n) ;
D.class = 'double complex' ;
D.pattern = logical (spones (D.matrix)) ;

dnn = struct ;
dtn = struct ('inp0', 'tran') ;
dnt = struct ('inp1', 'tran') ;
dtt = struct ('inp0', 'tran', 'inp1', 'tran') ;

Cin.matrix = sparse (n,n) ;
Cin.pattern = logical (sparse (n,n)) ;
Cin.class = 'double complex' ;

semiring.add = 'plus' ;
semiring.class = 'double complex' ;
semiring.multiply = 'times' ;

tol = 1e-12 ;

% GrB.burble (1) ;

for k = [false true]
    GB_builtin_complex_set (k) ;

    C1 = A.matrix*D.matrix ;
    C2 = GB_mex_mxm  (Cin, [ ], [ ], semiring, A, D, [ ]) ;
    GB_spec_compare (C1, C2, 0, tol) ;

    C1 = (A.matrix).'*(D.matrix).' ;
    C2 = GB_mex_mxm  (Cin, [ ], [ ], semiring, A, D, dtt) ;
    GB_spec_compare (C1, C2, 0, tol) ;

    C1 = (A.matrix)*(D.matrix).' ;
    C2 = GB_mex_mxm  (Cin, [ ], [ ], semiring, A, D, dnt) ;
    GB_spec_compare (C1, C2, 0, tol) ;

    C1 = (A.matrix).'*(D.matrix) ;
    C2 = GB_mex_mxm  (Cin, [ ], [ ], semiring, A, D, dtn) ;
    GB_spec_compare (C1, C2, 0, tol) ;


    C1 = (D.matrix)*(A.matrix) ;
    C2 = GB_mex_mxm  (Cin, [ ], [ ], semiring, D, A, [ ]) ;
    GB_spec_compare (C1, C2, 0, tol) ;

    C1 = (D.matrix).'*(A.matrix).' ;
    C2 = GB_mex_mxm  (Cin, [ ], [ ], semiring, D, A, dtt) ;
    GB_spec_compare (C1, C2, 0, tol) ;

    C1 = (D.matrix)*(A.matrix).' ;
    C2 = GB_mex_mxm  (Cin, [ ], [ ], semiring, D, A, dnt) ;
    GB_spec_compare (C1, C2, 0, tol) ;

    C1 = (D.matrix).'*(A.matrix) ;
    C2 = GB_mex_mxm  (Cin, [ ], [ ], semiring, D, A, dtn) ;
    GB_spec_compare (C1, C2, 0, tol) ;

end

% GrB.burble (0) ;

GB_builtin_complex_set (true) ;

fprintf ('\ntest245: all tests passed\n') ;

