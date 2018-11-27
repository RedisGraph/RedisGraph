function test02
%TEST02 test GrB_*_dup

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[mult_ops unary_ops add_ops classes semirings] = GB_spec_opsall ;

rng ('default') ;
format long g

for k1 = 1:length (classes)
    aclass = classes {k1} ;

    for is_hyper = 0:1
        for is_csc = 0:1

            % create a random A
            A = GB_spec_random (4, 4, 0.8, 128, aclass, is_csc, is_hyper) ;
            A.matrix (1,1) = -1 ;
            A.pattern (1,1) = true ;
            A_matrix = full (A.matrix) ;
            A_pattern = full (A.pattern) ;
            assert (spok (1*A.matrix) == 1) ;
            assert (spok (A.pattern) == 1) ;

            for k2 = 1:length (classes)
                cclass = classes {k2} ;
                % typecast to class C

                C = GB_mex_dup (A, cclass) ;
                C_matrix = full (C.matrix) ;
                C_pattern = full (spones (C.matrix)) ;
                assert (spok (1*C.matrix) == 1) ;

                if (k1 == k2)
                    % also try another method
                    assert (isequal (A_pattern, C_pattern)) ;
                    assert (isequal (A.class, C.class)) ;

                    C2 = GB_mex_dup (A, cclass, 1) ;
                    C2_matrix = full (C2.matrix) ;
                    C2_pattern = full (spones (C2.matrix)) ;
                    assert (isequal (C, C2))  ;
                    assert (spok (1*C2.matrix) == 1) ;
                end

            end
        end
    end
end

% duplicate a complex matrix (can't be typecasted)
A = GB_mex_random (4, 4, 10, 1) ;
assert (spok (1*A) == 1) ;

C = GB_mex_dup (A) ;
C_matrix = full (C.matrix) ;
assert (isequal (A, C.matrix))  ;
assert (spok (1*C.matrix) == 1) ;

C = GB_mex_dup (A, 'double', 1) ;
C_matrix = full (C.matrix) ;
assert (isequal (A, C.matrix))  ;
assert (spok (1*C.matrix) == 1) ;

format

fprintf ('test02: all typecast and copy tests passed\n') ;

