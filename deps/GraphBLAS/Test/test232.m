function test232
%TEST232 test assign with GrB_Scalar

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

[~, ~, ~, types, ~, ~, ~] = GB_spec_opsall ;
types = types.all ;

fprintf ('\n--- testing assign with GrB_Scalar\n') ;
rng ('default') ;

defaults = [ ] ;

for k1 = 1:length (types)
    type = types {k1} ;

    % create the accum
    accum.opname = 'plus' ;
    accum.optype = type ;

    m = 10 ;
    I1 = [1 3 5] ;
    I0 = uint64 (I1) - 1 ;
    for n = [1 9]
        if (n == 1)
            J1 = 1 ;
        else
            J1 = [1 2 5] ;
        end
        J0 = uint64 (J1) - 1 ;

        fprintf ('.') ;

        C = GB_spec_random (m, n, 0.8, 100, type) ;
        S1.matrix = sparse (1) ;
        S1.class = type ;
        S0.matrix = sparse (0) ;
        S0.class = type ;

        C1 = GB_mex_assign_scalar (C, [ ], accum, S1, I0, J0, [ ]) ;
        C2 = GB_spec_assign (C, [ ], accum, S1, I1, J1, [ ], 1) ;
        GB_spec_compare (C1, C2) ;

        C1 = GB_mex_assign_scalar (C, [ ], accum, S0, I0, J0, [ ]) ;
        C2 = GB_spec_assign (C, [ ], accum, S0, I1, J1, [ ], 1) ;
        GB_spec_compare (C1, C2) ;

        C1 = GB_mex_subassign_scalar (C, [ ], accum, S1, I0, J0, [ ]) ;
        C2 = GB_spec_subassign (C, [ ], accum, S1, I1, J1, [ ], 1) ;
        GB_spec_compare (C1, C2) ;

        C1 = GB_mex_subassign_scalar (C, [ ], accum, S0, I0, J0, [ ]) ;
        C2 = GB_spec_subassign (C, [ ], accum, S0, I1, J1, [ ], 1) ;
        GB_spec_compare (C1, C2) ;

        C1 = GB_mex_assign_scalar (C, [ ], [ ], S1, I0, J0, [ ]) ;
        C2 = GB_spec_assign (C, [ ], [ ], S1, I1, J1, [ ], 1) ;
        GB_spec_compare (C1, C2) ;

        C1 = GB_mex_assign_scalar (C, [ ], [ ], S0, I0, J0, [ ]) ;
        C2 = GB_spec_assign (C, [ ], [ ], S0, I1, J1, [ ], 1) ;
        GB_spec_compare (C1, C2) ;

        C1 = GB_mex_subassign_scalar (C, [ ], [ ], S1, I0, J0, [ ]) ;
        C2 = GB_spec_subassign (C, [ ], [ ], S1, I1, J1, [ ], 1) ;
        GB_spec_compare (C1, C2) ;

        C1 = GB_mex_subassign_scalar (C, [ ], [ ], S0, I0, J0, [ ]) ;
        C2 = GB_spec_subassign (C, [ ], [ ], S0, I1, J1, [ ], 1) ;
        GB_spec_compare (C1, C2) ;

    end
end

fprintf ('\ntest232: all tests passed\n') ;

