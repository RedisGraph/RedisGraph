function test10
%TEST10 test GrB_apply

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\nquick GrB_apply tests\n') ;

[~, unary_ops, ~, classes, ~, ~] = GB_spec_opsall ;

rng ('default') ;

m = 8 ;
n = 4 ;
dt = struct ('inp0', 'tran') ;
dr = struct ('outp', 'replace') ;

for k1 = 1:length(classes)
    aclass = classes {k1} ;
    fprintf ('%s: ', aclass) ;

    A   = GB_spec_random (m, n, 0.3, 100, aclass) ;
    Cin = GB_spec_random (m, n, 0.3, 100, aclass) ;
    B   = GB_spec_random (n, m, 0.3, 100, aclass) ;
    cin = cast (0, aclass) ;
    Mask = GB_random_mask (m, n, 0.5, true, false) ;

    if (isequal (aclass, 'double'))
        hrange = [0 1] ;
        crange = [0 1] ;
    else
        hrange = 0 ;
        crange = 1 ;
    end

    for A_is_hyper = hrange
    for A_is_csc   = crange
    for C_is_hyper = hrange
    for C_is_csc   = crange
    for M_is_hyper = hrange
    for M_is_csc   = crange

    A.is_csc    = A_is_csc ; A.is_hyper    = A_is_hyper ;
    Cin.is_csc  = C_is_csc ; Cin.is_hyper  = C_is_hyper ;
    B.is_csc    = A_is_csc ; B.is_hyper    = A_is_hyper ;
    Mask.is_csc = M_is_csc ; Mask.is_hyper = M_is_hyper ;

    for k2 = 1:length(unary_ops)
        op.opname = unary_ops {k2} ;
        % fprintf ('%s ', op.opname) ;
        fprintf ('.') ;

        for k3 = 1:length(classes)
            op.opclass = classes {k3} ;

            % no mask
            C1 = GB_spec_apply (Cin, [], [], op, A, []) ;
            C2 = GB_mex_apply  (Cin, [], [], op, A, []) ;
            GB_spec_compare (C1, C2) ;

            % no mask, with accum
            C1 = GB_spec_apply (Cin, [], 'plus', op, A, []) ;
            C2 = GB_mex_apply  (Cin, [], 'plus', op, A, []) ;
            GB_spec_compare (C1, C2) ;

            % with mask
            C1 = GB_spec_apply (Cin, Mask, [], op, A, []) ;
            C2 = GB_mex_apply  (Cin, Mask, [], op, A, []) ;
            GB_spec_compare (C1, C2) ;

            % with mask and accum
            C1 = GB_spec_apply (Cin, Mask, 'plus', op, A, []) ;
            C2 = GB_mex_apply  (Cin, Mask, 'plus', op, A, []) ;
            GB_spec_compare (C1, C2) ;

            Cmask = spones (GB_mex_cast (full (Cin.matrix), Cin.class)) ;

            % with C == mask, and outp = replace
            C1 = GB_spec_apply (Cin, Cmask, [], op, A, dr) ;
            C2 = GB_mex_apply2 (Cin,        [], op, A, dr) ;
            GB_spec_compare (C1, C2) ;

            % with C == mask and accum, and outp = replace
            C1 = GB_spec_apply (Cin, Cmask, 'plus', op, A, dr) ;
            C2 = GB_mex_apply2 (Cin,        'plus', op, A, dr) ;
            GB_spec_compare (C1, C2) ;

            % no mask, transpose
            C1 = GB_spec_apply (Cin, [], [], op, B, dt) ;
            C2 = GB_mex_apply  (Cin, [], [], op, B, dt) ;
            GB_spec_compare (C1, C2) ;

            % no mask, with accum, transpose
            C1 = GB_spec_apply (Cin, [], 'plus', op, B, dt) ;
            C2 = GB_mex_apply  (Cin, [], 'plus', op, B, dt) ;
            GB_spec_compare (C1, C2) ;

            % with mask, transpose
            C1 = GB_spec_apply (Cin, Mask, [], op, B, dt) ;
            C2 = GB_mex_apply  (Cin, Mask, [], op, B, dt) ;
            GB_spec_compare (C1, C2) ;

            % with mask and accum, transpose
            C1 = GB_spec_apply (Cin, Mask, 'plus', op, B, dt) ;
            C2 = GB_mex_apply  (Cin, Mask, 'plus', op, B, dt) ;
            GB_spec_compare (C1, C2) ;

        end
    end

    end
    end
    end
    end
    end
    end
    fprintf ('\n') ;

end

fprintf ('\ntest10: all tests passed\n') ;

