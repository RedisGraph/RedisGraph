function test17
%TEST17 test GrB_*_extractElement

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n ------------ testing GrB_extractElement\n') ;

[~, ~, ~, classes, ~, ~] = GB_spec_opsall ;

rng ('default') ;

% class of the output X
for k1 = 4 % 1:length (classes)
    xclass = classes {k1}  ;
    fprintf ('\n%s', xclass) ;

    % class of the matrix A
    for k2 = 3 % 1:length (classes)
        aclass = classes {k2}  ;

        % create a matrix
        for m = [1 10] % [1 10 25 50]
            for n = [1 10] % [1 10 25 50]
                fprintf ('.') ;
                clear A
                A.matrix = 100 * sprandn (m, n, 0.1) ;
                A.class = aclass ;

                clear B
                B.matrix = 100 * sprandn (m*n, 1, 0.1) ;
                B.class = aclass ;

                for A_is_hyper = 0:1
                for A_is_csc   = 0:1
                A.is_hyper = A_is_hyper ;
                A.is_csc   = A_is_csc   ;

                for i = 0:m-1
                    for j = 0:n-1
                        x1 = GB_mex_Matrix_extractElement  (A, uint64(i), uint64(j), xclass) ;
                        x2 = GB_spec_Matrix_extractElement (A, i, j, xclass) ;
                        assert (isequal (x1,x2))
                    end
                end

                end
                end

                for i = 0:(m*n)-1
                    x1 = GB_mex_Vector_extractElement  (B, uint64(i), xclass) ;
                    x2 = GB_spec_Vector_extractElement (B, uint64(i), xclass) ;
                    assert (isequal (x1,x2))
                end

            end
        end
    end
end

fprintf ('\ntest17: all tests passed\n') ;

