function test128
%TEST128 test eWiseMult and eWiseAdd, special cases

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

fprintf ('\ntest128: test eWiseMult and eWiseAdd, special cases\n') ;
rng ('default') ;

m = 100 ;
n = 100 ;

Mmat = sparse (m,n) ;
Mmat (1,:) = 1 ;

Amat = sparse (rand (m,n)) ;
Amat (:, 2) = 0 ;
Amat (:, 5) = 0 ;
Amat (1, 5) = 1 ;
Amat (2, 6) = 0 ;

Bmat = sparse (rand (m,n)) ;
Bmat (:, 3:4) = 0 ;
Bmat (:, 6) = 0 ;
Bmat (1, 6) = 1 ;
Amat (2, 5) = 0 ;

clear M
M.matrix  = Mmat ;
M.pattern = logical (spones (Mmat)) ;
M.class = 'logical' ;

clear A
A.matrix  = Amat ;
A.pattern = logical (spones (Amat)) ;
A.class = 'double' ;

clear B
B.matrix  = Bmat ; 
B.pattern = logical (spones (Bmat)) ;
B.class = 'double' ;

S = sparse (m,n) ;
X = sparse (rand (m,n)) ;

for B_hyper = 0:1
    for A_hyper = 0:1
        A.is_hyper = A_hyper ;
        B.is_hyper = B_hyper ;

        for M_hyper = 0:1
            M.is_hyper = M_hyper ;

            C0 = Amat .* Bmat .* Mmat ;
            C1 = GB_spec_Matrix_eWiseMult (S, M, [ ], 'times', A, B, [ ]) ;
            C2 = GB_mex_Matrix_eWiseMult  (S, M, [ ], 'times', A, B, [ ]) ;
            C3 = GB_mex_Matrix_eWiseMult  (S, M, [ ], 'times', B, A, [ ]) ;
            GB_spec_compare (C1, C2) ;
            GB_spec_compare (C1, C3) ;
            assert (isequal (C0, C2.matrix)) ;

            C0 = (Amat + Bmat) .* Mmat ;
            C1 = GB_spec_Matrix_eWiseAdd (S, M, [ ], 'plus', A, B, [ ]) ;
            C2 = GB_mex_Matrix_eWiseAdd  (S, M, [ ], 'plus', A, B, [ ]) ;
            C3 = GB_mex_Matrix_eWiseAdd  (S, M, [ ], 'plus', B, A, [ ]) ;
            GB_spec_compare (C1, C2) ;
            GB_spec_compare (C1, C3) ;
            assert (isequal (C0, C2.matrix)) ;

            C1 = GB_spec_Matrix_eWiseMult (X, M, [ ], 'times', A, B, [ ]) ;
            C2 = GB_mex_Matrix_eWiseMult  (X, M, [ ], 'times', A, B, [ ]) ;
            GB_spec_compare (C1, C2) ;

            C1 = GB_spec_Matrix_eWiseAdd (X, M, [ ], 'plus', A, B, [ ]) ;
            C2 = GB_mex_Matrix_eWiseAdd  (X, M, [ ], 'plus', A, B, [ ]) ;
            GB_spec_compare (C1, C2) ;

        end

        C0 = Amat .* Bmat ;
        C1 = GB_spec_Matrix_eWiseMult (S, [ ], [ ], 'times', A, B, [ ]) ;
        C2 = GB_mex_Matrix_eWiseMult  (S, [ ], [ ], 'times', A, B, [ ]) ;
        C3 = GB_mex_Matrix_eWiseMult  (S, [ ], [ ], 'times', B, A, [ ]) ;
        GB_spec_compare (C1, C2) ;
        GB_spec_compare (C1, C3) ;
        assert (isequal (C0, C2.matrix)) ;

        C0 = Amat + Bmat ;
        C1 = GB_spec_Matrix_eWiseAdd (S, [ ], [ ], 'plus', A, B, [ ]) ;
        C2 = GB_mex_Matrix_eWiseAdd  (S, [ ], [ ], 'plus', A, B, [ ]) ;
        C3 = GB_mex_Matrix_eWiseAdd  (S, [ ], [ ], 'plus', B, A, [ ]) ;
        GB_spec_compare (C1, C2) ;
        GB_spec_compare (C1, C3) ;
        assert (isequal (C0, C2.matrix)) ;

        C1 = GB_spec_Matrix_eWiseMult (X, [ ], [ ], 'times', A, B, [ ]) ;
        C2 = GB_mex_Matrix_eWiseMult  (X, [ ], [ ], 'times', A, B, [ ]) ;
        GB_spec_compare (C1, C2) ;

        C1 = GB_spec_Matrix_eWiseAdd (X, [ ], [ ], 'plus', A, B, [ ]) ;
        C2 = GB_mex_Matrix_eWiseAdd  (X, [ ], [ ], 'plus', A, B, [ ]) ;
        GB_spec_compare (C1, C2) ;

    end
end

fprintf ('test128: all tests passed\n') ;
