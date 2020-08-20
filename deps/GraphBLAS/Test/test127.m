function test127
%TEST127 test GrB_eWiseAdd and GrB_eWiseMult (all types and operators)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[bin_ops, ~, ~, classes, ~, ~] = GB_spec_opsall ;

fprintf ('test127 ------------tests of GrB_eWiseAdd and eWiseMult (all ops)\n') ;

m = 5 ;
n = 5 ;

rng ('default') ;

dnn = struct ;
dtn = struct ( 'inp0', 'tran' ) ;
dnt = struct ( 'inp1', 'tran' ) ;
dtt = struct ( 'inp0', 'tran', 'inp1', 'tran' ) ;

Amat = sparse (100 * sprandn (m,n, 0.8)) ;
Bmat = sparse (100 * sprandn (m,n, 0.8)) ;
Cmat = sparse (100 * sprandn (m,n, 0.8)) ;
w = sparse (100 * sprandn (m,1, 0.8)) ;
uvec = sparse (100 * sprandn (m,1, 0.8)) ;
vvec = sparse (100 * sprandn (m,1, 0.8)) ;
Maskmat = sprandn (m,n,0.9) ~= 0 ;
maskvec = sprandn (m,1,0.9) ~= 0 ;

% create a very sparse matrix mask
Maskmat2 = sparse (m,n) ;
T = Amat .* Bmat ;
[i j x] = find (T) ;
if (length (i) > 0)
    Maskmat2 (i(1), j(1)) = 1 ;
end
T = (Amat ~= 0) & (Bmat == 0) ;
[i j x] = find (T) ;
if (length (i) > 0)
    Maskmat2 (i(1), j(1)) = 1 ;
end
T = (Amat == 0) & (Bmat ~= 0) ;
[i j x] = find (T) ;
if (length (i) > 0)
    Maskmat2 (i(1), j(1)) = 1 ;
end
clear T i j x

% create a very sparse vector mask
maskvec2 = sparse (m,1) ;
T = uvec .* vvec ;
[i j x] = find (T) ;
if (length (i) > 0)
    maskvec2 (i(1), j(1)) = 1 ;
end
T = (uvec ~= 0) & (vvec == 0) ;
[i j x] = find (T) ;
if (length (i) > 0)
    maskvec2 (i(1), j(1)) = 1 ;
end
T = (uvec == 0) & (vvec ~= 0) ;
[i j x] = find (T) ;
if (length (i) > 0)
    maskvec2 (i(1), j(1)) = 1 ;
end
clear T i j x

ATmat = Amat' ;
BTmat = Bmat' ;

for k1 = 1:length (classes)
    clas = classes {k1}  ;

    for k2 = 1:length(bin_ops)
        binop = bin_ops {k2}  ;

        op.opname = binop ;
        op.opclass = clas ;
        fprintf ('.') ;

        for A_is_hyper = 0 % 0:1
        for A_is_csc   = 0 % 0:1
        for B_is_hyper = 0 % 0:1
        for B_is_csc   = 0 % 0:1
        for C_is_hyper = 0 % 0:1
        for C_is_csc   = 0 % 0:1

        for native = 1 % 0:1

        clear A
        A.matrix = Amat ;
        A.is_hyper = A_is_hyper ;
        A.is_csc   = A_is_csc   ;
        if (native)
            A.class = op.opclass ;
        end

        clear AT
        AT.matrix = ATmat ;
        AT.is_hyper = A_is_hyper ;
        AT.is_csc   = A_is_csc   ;
        if (native)
            AT.class = op.opclass ;
        end

        clear B
        B.matrix = Bmat ;
        B.is_hyper = B_is_hyper ;
        B.is_csc   = B_is_csc   ;
        if (native)
            B.class = op.opclass ;
        end

        clear BT
        BT.matrix = BTmat ;
        BT.is_hyper = B_is_hyper ;
        BT.is_csc   = B_is_csc   ;
        if (native)
            BT.class = op.opclass ;
        end

        clear C
        C.matrix = Cmat ;
        C.is_hyper = C_is_hyper ;
        C.is_csc   = C_is_csc   ;

        clear u
        u.matrix = uvec ;
        u.is_csc = true ;
        if (native)
            u.class = op.opclass ;
        end

        clear v
        v.matrix = vvec ;
        v.is_csc = true ;
        if (native)
            v.class = op.opclass ;
        end

        %---------------------------------------
        % A+B
        %---------------------------------------

        C0 = GB_spec_eWiseAdd_Matrix ...
            (C, [ ], [ ], op, A, B, dnn);
        C1 = GB_mex_eWiseAdd_Matrix ...
            (C, [ ], [ ], op, A, B, dnn);
        GB_spec_compare (C0, C1) ;

        w0 = GB_spec_eWiseAdd_Vector ...
            (w, [ ], [ ], op, u, v, dnn);
        w1 = GB_mex_eWiseAdd_Vector ...
            (w, [ ], [ ], op, u, v, dnn);
        GB_spec_compare (w0, w1) ;

        %---------------------------------------
        % A'+B
        %---------------------------------------

        C0 = GB_spec_eWiseAdd_Matrix ...
            (C, [ ], [ ], op, AT, B, dtn);
        C1 = GB_mex_eWiseAdd_Matrix ...
            (C, [ ], [ ], op, AT, B, dtn);
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % A+B'
        %---------------------------------------

        C0 = GB_spec_eWiseAdd_Matrix ...
            (C, [ ], [ ], op, A, BT, dnt);
        C1 = GB_mex_eWiseAdd_Matrix ...
            (C, [ ], [ ], op, A, BT, dnt);
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % A'+B'
        %---------------------------------------

        C0 = GB_spec_eWiseAdd_Matrix ...
            (C, [ ], [ ], op, AT, BT, dtt);
        C1 = GB_mex_eWiseAdd_Matrix ...
            (C, [ ], [ ], op, AT, BT, dtt);
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % A.*B
        %---------------------------------------

        C0 = GB_spec_eWiseMult_Matrix ...
            (C, [ ], [ ], op, A, B, dnn);
        C1 = GB_mex_eWiseMult_Matrix ...
            (C, [ ], [ ], op, A, B, dnn);
        GB_spec_compare (C0, C1) ;

        w0 = GB_spec_eWiseMult_Vector ...
            (w, [ ], [ ], op, u, v, dnn);
        w1 = GB_mex_eWiseMult_Vector ...
            (w, [ ], [ ], op, u, v, dnn);
        GB_spec_compare (w0, w1) ;

        %---------------------------------------
        % A'.*B
        %---------------------------------------

        C0 = GB_spec_eWiseMult_Matrix ...
            (C, [ ], [ ], op, AT, B, dtn);
        C1 = GB_mex_eWiseMult_Matrix ...
            (C, [ ], [ ], op, AT, B, dtn);
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % A.*B'
        %---------------------------------------

        C0 = GB_spec_eWiseMult_Matrix ...
            (C, [ ], [ ], op, A, BT, dnt);
        C1 = GB_mex_eWiseMult_Matrix ...
            (C, [ ], [ ], op, A, BT, dnt);
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % A'.*B'
        %---------------------------------------

        C0 = GB_spec_eWiseMult_Matrix ...
            (C, [ ], [ ], op, AT, BT, dtt);
        C1 = GB_mex_eWiseMult_Matrix ...
            (C, [ ], [ ], op, AT, BT, dtt);
        GB_spec_compare (C0, C1) ;

        %-----------------------------------------------
        % with mask
        %-----------------------------------------------

        for M_is_very_sparse = 0 % 0:1
        for M_is_hyper = 0 % 0:1
        for M_is_csc   = 0 % 0:1

        clear Mask mask
        if (M_is_very_sparse)
            Mask.matrix = Maskmat2 ;
            mask.matrix = maskvec2 ;
        else
            Mask.matrix = Maskmat ;
            mask.matrix = maskvec ;
        end
        Mask.is_hyper = M_is_hyper ;
        Mask.is_csc   = M_is_csc   ;
        mask.is_csc = true ;

        %---------------------------------------
        % A+B, with mask
        %---------------------------------------

        C0 = GB_spec_eWiseAdd_Matrix ...
            (C, Mask, [ ], op, A, B, dnn);
        C1 = GB_mex_eWiseAdd_Matrix ...
            (C, Mask, [ ], op, A, B, dnn);
        GB_spec_compare (C0, C1) ;

        w0 = GB_spec_eWiseAdd_Vector ...
            (w, mask, [ ], op, u, v, dnn);
        w1 = GB_mex_eWiseAdd_Vector ...
            (w, mask, [ ], op, u, v, dnn);
        GB_spec_compare (w0, w1) ;

        %---------------------------------------
        % A'+B, with mask
        %---------------------------------------

        C0 = GB_spec_eWiseAdd_Matrix ...
            (C, Mask, [ ], op, AT, B, dtn);
        C1 = GB_mex_eWiseAdd_Matrix ...
            (C, Mask, [ ], op, AT, B, dtn);
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % A+B', with mask
        %---------------------------------------

        C0 = GB_spec_eWiseAdd_Matrix ...
            (C, Mask, [ ], op, A, BT, dnt);
        C1 = GB_mex_eWiseAdd_Matrix ...
            (C, Mask, [ ], op, A, BT, dnt);
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % A'+B', with mask
        %---------------------------------------

        C0 = GB_spec_eWiseAdd_Matrix ...
            (C, Mask, [ ], op, AT, BT, dtt);
        C1 = GB_mex_eWiseAdd_Matrix ...
            (C, Mask, [ ], op, AT, BT, dtt);
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % A.*B, with mask
        %---------------------------------------

        C0 = GB_spec_eWiseMult_Matrix ...
            (C, Mask, [ ], op, A, B, dnn);
        C1 = GB_mex_eWiseMult_Matrix ...
            (C, Mask, [ ], op, A, B, dnn);
        GB_spec_compare (C0, C1) ;

        w0 = GB_spec_eWiseMult_Vector ...
            (w, mask, [ ], op, u, v, dnn);
        w1 = GB_mex_eWiseMult_Vector ...
            (w, mask, [ ], op, u, v, dnn);
        GB_spec_compare (w0, w1) ;

        %---------------------------------------
        % A'.*B, with mask
        %---------------------------------------

        C0 = GB_spec_eWiseMult_Matrix ...
            (C, Mask, [ ], op, AT, B, dtn);
        C1 = GB_mex_eWiseMult_Matrix ...
            (C, Mask, [ ], op, AT, B, dtn);
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % A.*B', with mask
        %---------------------------------------

        C0 = GB_spec_eWiseMult_Matrix ...
            (C, Mask, [ ], op, A, BT, dnt);
        C1 = GB_mex_eWiseMult_Matrix ...
            (C, Mask, [ ], op, A, BT, dnt);
        GB_spec_compare (C0, C1) ;

        %---------------------------------------
        % A'.*B', with mask
        %---------------------------------------

        C0 = GB_spec_eWiseMult_Matrix ...
            (C, Mask, [ ], op, AT, BT, dtt);
        C1 = GB_mex_eWiseMult_Matrix ...
            (C, Mask, [ ], op, AT, BT, dtt);
        GB_spec_compare (C0, C1) ;


        end
        end
        end
        end
        end
        end
        end
        end
        end
        end
    end
end

fprintf ('\ntest127: all tests passed\n') ;

