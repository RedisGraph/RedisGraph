function test231
%TEST231 test GrB_select with idxunp

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

[~, ~, ~, types, ~, ~, idxunops] = GB_spec_opsall ;
ops = idxunops ;
types = types.all ;

fprintf ('\n--- testing select with idxunops\n') ;
rng ('default') ;

desc.inp0 = 'tran' ;

n_operators = 0 ;
for k2 = 1:length(ops)
    opname = ops {k2} ;
    fprintf ('\n%-10s ', opname) ;

    for k1 = 1:length (types)
    type = types {k1} ;

    % create the op
    clear op
    op.opname = opname ;
    op.optype = type ;

    [is_idxunop, ztype] = GB_spec_is_idxunop (opname, type) ;
    if (~is_idxunop)
        continue ;
    end

    n_operators = n_operators + 1 ;

    for m = [1 4] % [ 1 10 ]% 100]
    for n = [1 4] % [1 10 ]% 100]
    for hi = [1 5] % [-1:2:5 ]
    for lo = [-1 0] % [-3:2:5 ]
    Amat = (hi*sprand (m,n,0.8)-lo) .* sprand (m,n,0.5) ;
    Cmat = sparse (m, n) ;
    fprintf ('.') ;

    C.matrix = Cmat ;
    C.class = ztype ;

    CT.matrix = Cmat' ;
    CT.class = ztype ;

    A.matrix = Amat ;
    A.class = type ;

    B.matrix = spones (Amat) ;
    B.class = type ;
    B.iso = true ;

    for ythunk = -3:3
    y.matrix = ythunk ;
    y.class = type ;

    for how = 0:1
    for csc = 0:1

    A.is_csc = csc ;
    C.is_csc = csc ;
    CT.is_csc = csc ;

    for sparsity = [1 2 4]
        A.sparsity = sparsity ;

        C1 = GB_mex_select_idxunop  (C, [ ], [ ], op, how, A, y, [ ]) ;
        C2 = GB_spec_select_idxunop (C, [ ], [ ], op,      A, y, [ ]) ;
        GB_spec_compare (C1, C2) ;

        C1 = GB_mex_select_idxunop  (C, [ ], [ ], op, how, B, y, [ ]) ;
        C2 = GB_spec_select_idxunop (C, [ ], [ ], op,      B, y, [ ]) ;
        GB_spec_compare (C1, C2) ;

        C1 = GB_mex_select_idxunop  (CT, [ ], [ ], op, how, A, y, desc) ;
        C2 = GB_spec_select_idxunop (CT, [ ], [ ], op,      A, y, desc) ;
        GB_spec_compare (C1, C2) ;

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

fprintf ('\nNumber of built-in GraphBLAS idxunops: %d\n',  n_operators) ;
fprintf ('\ntest231: all tests passed\n') ;

