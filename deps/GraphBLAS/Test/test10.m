function test10
%TEST10 test GrB_apply

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\nquick GrB_apply tests\n') ;

[mult_ops unary_ops add_ops classes semirings] = GB_spec_opsall ;

rng ('default') ;

m = 8 ;
n = 4 ;
dt = struct ('inp0', 'tran') ;
dr = struct ('outp', 'replace') ;

for k1 = 1:length(classes)
    aclass = classes {k1} ;
    fprintf ('%s: ', aclass) ;

    A = GB_spec_random (m, n, 0.3, 100, aclass) ;
    Cin = GB_spec_random (m, n, 0.3, 100, aclass) ;
    B = GB_spec_random (n, m, 0.3, 100, aclass) ;
    cin = cast (0, aclass) ;
    Mask = (sprand (m, n, 0.5) ~= 0) ;

    for k2 = 1:length(unary_ops)
        op.opname = unary_ops {k2} ;
        fprintf ('%s ', op.opname) ;

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

            % with C == mask, and outp = replace
            C1 = GB_spec_apply (Cin, Cin.pattern, [], op, A, dr) ;
            C2 = GB_mex_apply2 (Cin,              [], op, A, dr) ;
            GB_spec_compare (C1, C2) ;

            % with C == mask and accum, and outp = replace
            C1 = GB_spec_apply (Cin, Cin.pattern, 'plus', op, A, dr) ;
            C2 = GB_mex_apply2 (Cin,              'plus', op, A, dr) ;
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
    fprintf ('\n') ;
end

fprintf ('\ntest10: all tests passed\n') ;

