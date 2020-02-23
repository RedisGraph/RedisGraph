function test141
%TEST141 test GrB_eWiseAdd (all types and operators) for dense matrices

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[bin_ops, ~, ~, classes, ~, ~] = GB_spec_opsall ;

fprintf ('test141 ------------ GrB_eWiseAdd with dense matrices\n') ;

m = 5 ;
n = 5 ;

rng ('default') ;

M = sprand (m, n, 0.5) ;
Amat = sparse (100 * rand (m,n)) ;
Bmat = sparse (100 * rand (m,n)) ;
Cmat = sparse (100 * rand (m,n)) ;
Emat = sprand (m, n, 0.5) ;
Smat = sparse (m,n) ;
desc.mask = 'structural' ;

A.matrix = Amat ; A.class = 'see below' ;
B.matrix = Bmat ; B.class = 'see below' ;
C.matrix = Cmat ; C.class = 'see below' ;
S.matrix = Smat ; S.class = 'see below' ;
E.matrix = Emat ; E.class = 'see below' ;

for k2 = 1:length(bin_ops)
    binop = bin_ops {k2}  ;
    fprintf ('%s', binop) ;

    for k1 = 1:length (classes)
        clas = classes {k1}  ;

        op.opname = binop ;
        op.opclass = clas ;
        fprintf ('.') ;

        A.class = clas ;
        B.class = clas ;
        E.class = clas ;

        if (k2 > 20)
            % eq, ne, gt, lt, ge, le
            S.class = 'logical' ;
            C.class = 'logical' ;
        else
            S.class = clas ;
            C.class = clas ;
        end

        %---------------------------------------
        % C = A+B
        %---------------------------------------

        C0 = GB_spec_eWiseAdd_Matrix (S, [ ], [ ], op, A, B, [ ]) ;
        C1 = GB_mex_eWiseAdd_Matrix  (S, [ ], [ ], op, A, B, [ ]) ;
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % C<M> = A+B, both A and B dense
        %---------------------------------------

        C0 = GB_spec_eWiseAdd_Matrix (S, M, [ ], op, A, B, desc) ;
        C1 = GB_mex_eWiseAdd_Matrix  (S, M, [ ], op, A, B, desc) ;
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % C<M> = A+E, A dense, E sparse
        %---------------------------------------

        C0 = GB_spec_eWiseAdd_Matrix (S, M, [ ], op, A, E, desc) ;
        C1 = GB_mex_eWiseAdd_Matrix  (S, M, [ ], op, A, E, desc) ;
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % C<M> = E+A, A dense, E sparse
        %---------------------------------------

        C0 = GB_spec_eWiseAdd_Matrix (S, M, [ ], op, E, A, desc) ;
        C1 = GB_mex_eWiseAdd_Matrix  (S, M, [ ], op, E, A, desc) ;
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % C += A+B
        %---------------------------------------

        C0 = GB_spec_eWiseAdd_Matrix (C, [ ], op, op, A, B, [ ]) ;
        C1 = GB_mex_eWiseAdd_Matrix  (C, [ ], op, op, A, B, [ ]) ;
        GB_spec_compare (C0, C1) ;

    end
end

fprintf ('\ntest141: all tests passed\n') ;

