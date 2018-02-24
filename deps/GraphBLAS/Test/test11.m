function test11
%TEST11 test GrB_*_extractTuples

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n ------------ testing GrB_extractTuples\n') ;

[accum_ops unary_ops add_ops classes] = GB_spec_opsall ;


% class of the output X
for k1 = 1:length (classes)
    xclass = classes {k1}  ;
    fprintf ('\n%s', xclass) ;

    % class of the matrix A
    for k2 = 1:length (classes)
        aclass = classes {k2}  ;

        % create a matrix
        for m = [1 10 25]
            for n = [1 10 25]
                fprintf ('.') ;
                clear A
                A.matrix = sprandn (m, n, 0.1) ;
                A.class = aclass ;

                clear B
                B.matrix = sprandn (m*n, 1, 0.1) ;
                B.class = aclass ;

                [I1, J1, X1] = GB_mex_extractTuples  (A, xclass) ;
                [I2, J2, X2] = GB_spec_extractTuples (A, xclass) ;

                assert (isequal (I1, I2)) ;
                assert (isequal (J1, J2)) ;
                assert (isequal (X1, X2)) ;

                [I1, J1, X1] = GB_mex_extractTuples  (B, xclass) ;
                [I2, J2, X2] = GB_spec_extractTuples (B, xclass) ;

                assert (isequal (I1, I2)) ;
                assert (isequal (J1, J2)) ;
                assert (isequal (X1, X2)) ;

            end
        end
    end
end

fprintf ('\ntest11: all tests passed\n') ;

