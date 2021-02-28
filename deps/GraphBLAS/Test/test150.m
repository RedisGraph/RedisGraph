function test150
%TEST150 test GrB_mxm with typecasting and zombies (dot3)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('test150: ------- GrB_mxm with typecasting and zombies (dot3)\n') ;

[~, ~, ~, classes, ~, ~] = GB_spec_opsall ;

semiring.add = 'plus' ;
semiring.multiply = 'times' ;
semiring.class = 'double' ;
identity = 0 ;

dnn = struct ( 'axb', 'dot') ;

m = 8 ;
n = 5 ;
s = 4 ;
density = 0.1 ;

for k6 = 1:length (classes)
    aclas = classes {k6} ;
    fprintf ('%s ', aclas) ;

    A = GB_spec_random (m, s, density, 100, aclas) ;
    B = GB_spec_random (s, n, density, 100, aclas) ;
    M = GB_random_mask(m,n,0.2) ;

    clear C
    C.matrix = sparse (m,n) ;
    C.class = 'int32' ;
    C.pattern = false (m,n) ;

    C0 = GB_spec_mxm (C, M, [ ], semiring, A, B, dnn);
    C1 = GB_mex_mxm  (C, M, [ ], semiring, A, B, dnn);
    GB_spec_compare (C0, C1, identity) ;

end

fprintf ('\ntest150: all tests passed\n') ;

