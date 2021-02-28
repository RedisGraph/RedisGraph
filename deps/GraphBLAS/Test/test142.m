function test142
%TEST142 test GrB_assign for dense matrices

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[bin_ops, ~, ~, classes, ~, ~] = GB_spec_opsall ;

fprintf ('test142 ------------ GrB_assign with dense matrices\n') ;

m = 10 ;
n = 12 ;

rng ('default') ;

M = sprand (m, n, 0.5) ;
Amat = sparse (100 * rand (m,n)) ;
Bmat = sparse (100 * sprand (m,n, 0.5)) ;
Cmat = sparse (100 * rand (m,n)) ;
Smat = sparse (m,n) ;
Xmat = sparse (pi) ;
desc.mask = 'structural' ;
drep.outp = 'replace' ;

A.matrix = Amat ; A.class = 'see below' ;
B.matrix = Bmat ; B.class = 'see below' ;
C.matrix = Cmat ; C.class = 'see below' ;
S.matrix = Smat ; S.class = 'see below' ;
X.matrix = Xmat ; X.class = 'see below' ;
Bmask = logical (Bmat) ;

for k1 = 1:length (classes)
    clas = classes {k1}  ;
    fprintf ('%s', clas) ;

    A.class = clas ;
    B.class = clas ;
    X.class = clas ;

    for k3 = 1:2

        if (k3 == 1)
            C.class = 'logical' ;
            S.class = 'logical' ;
        else
            C.class = clas ;
            S.class = clas ;
        end

        %---------------------------------------
        % C<M> = A where A is dense
        %---------------------------------------

        C0 = GB_spec_assign (C, M, [ ], A, [ ], [ ], [ ], false) ;
        C1 = GB_mex_assign  (C, M, [ ], A, [ ], [ ], [ ]) ;
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % C<M> = B where B is sparse
        %---------------------------------------

        C0 = GB_spec_assign (C, M, [ ], B, [ ], [ ], [ ], false) ;
        C1 = GB_mex_assign  (C, M, [ ], B, [ ], [ ], [ ]) ;
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % C<M> = A where A is dense and C starts empty
        %---------------------------------------

        C0 = GB_spec_assign (S, M, [ ], A, [ ], [ ], desc, false) ;
        C1 = GB_mex_assign  (S, M, [ ], A, [ ], [ ], desc) ;
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % C<B> = B where B is sparse
        %---------------------------------------

        C0 = GB_spec_assign (C, Bmask, [ ], B, [ ], [ ], desc, false) ;
        C1 = GB_mex_assign_alias_mask (C, B, desc) ;
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % C<M> = x where C is dense
        %---------------------------------------

        C0 = GB_spec_assign (C, M, [ ], X, [ ], [ ], [ ], true) ;
        C1 = GB_mex_assign  (C, M, [ ], X, [ ], [ ], [ ]) ;
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % C<M> = x where C is dense
        %---------------------------------------

        C0 = GB_spec_assign (C, M, [ ], X, [ ], [ ], desc, true) ;
        C1 = GB_mex_assign  (C, M, [ ], X, [ ], [ ], desc) ;
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % C<M,struct> = x
        %---------------------------------------

        C0 = GB_spec_assign (S, M, [ ], X, [ ], [ ], desc, true) ;
        C1 = GB_mex_assign  (S, M, [ ], X, [ ], [ ], desc) ;
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % C = x
        %---------------------------------------

        C0 = GB_spec_assign (S, [ ], [ ], X, [ ], [ ], [ ], true) ;
        C1 = GB_mex_assign  (S, [ ], [ ], X, [ ], [ ], [ ]) ;
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % with accum operators
        %---------------------------------------

        for k2 = 1:length(bin_ops)
            binop = bin_ops {k2}  ;

            op.opname = binop ;
            op.opclass = clas ;
            fprintf ('.') ;

            %---------------------------------------
            % C += A where A is dense
            %---------------------------------------

            C0 = GB_spec_assign (C, [ ], op, A, [ ], [ ], [ ], false) ;
            C1 = GB_mex_assign  (C, [ ], op, A, [ ], [ ], [ ]) ;
            GB_spec_compare (C0, C1) ;

            %---------------------------------------
            % C += B where B is sparse
            %---------------------------------------

            C0 = GB_spec_assign (C, [ ], op, B, [ ], [ ], [ ], false) ;
            C1 = GB_mex_assign  (C, [ ], op, B, [ ], [ ], [ ]) ;
            GB_spec_compare (C0, C1) ;

            %---------------------------------------
            % C += x
            %---------------------------------------

            C0 = GB_spec_assign (C, [ ], op, X, [ ], [ ], [ ], true) ;
            C1 = GB_mex_assign  (C, [ ], op, X, [ ], [ ], [ ]) ;
            GB_spec_compare (C0, C1) ;

            %---------------------------------------
            % C<replace> += x
            %---------------------------------------

            C0 = GB_spec_assign (C, [ ], op, X, [ ], [ ], drep, true) ;
            C1 = GB_mex_subassign  (C, [ ], op, X, [ ], [ ], drep) ;
            GB_spec_compare (C0, C1) ;

        end
    end
end

fprintf ('\ntest142: all tests passed\n') ;

