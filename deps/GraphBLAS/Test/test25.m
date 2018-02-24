function test25
%TEST25 test GxB_select

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\nquick GxB_select tests\n') ;

[mult_ops unary_ops add_ops classes semirings] = GB_spec_opsall ;

select_ops = { 'tril', 'triu', 'diag', 'offdiag', 'nonzero' } ;

rng ('default') ;

m = 10 ;
n = 6 ;
dt = struct ('inp0', 'tran') ;

for k1 = 1:length(classes)
    aclass = classes {k1} ;
    fprintf ('%s: ', aclass) ;

    A = GB_spec_random (m, n, 0.3, 100, aclass) ;
    A.matrix (:,1) = rand (m,1) ;
    A.pattern (:,1) = true (m,1) ;
    Cin = GB_spec_random (m, n, 0.3, 100, aclass) ;
    B = GB_spec_random (n, m, 0.3, 100, aclass) ;
    cin = cast (0, aclass) ;
    Mask = (sprand (m, n, 0.5) ~= 0) ;

    for k2 = 1:length(select_ops)
        op = select_ops {k2} ;
        fprintf ('%s ', op) ;

        for k = [-m:n]

            % no mask
            C1 = GB_spec_select (Cin, [], [], op, A, k, []) ;
            C2 = GB_mex_select  (Cin, [], [], op, A, k, []) ;
            GB_spec_compare (C1, C2) ;

            % no mask, with accum
            C1 = GB_spec_select (Cin, [], 'plus', op, A, k, []) ;
            C2 = GB_mex_select  (Cin, [], 'plus', op, A, k, []) ;
            GB_spec_compare (C1, C2) ;

            % with mask
            C1 = GB_spec_select (Cin, Mask, [], op, A, k, []) ;
            C2 = GB_mex_select  (Cin, Mask, [], op, A, k, []) ;
            GB_spec_compare (C1, C2) ;

            % with mask and accum
            C1 = GB_spec_select (Cin, Mask, 'plus', op, A, k, []) ;
            C2 = GB_mex_select  (Cin, Mask, 'plus', op, A, k, []) ;
            GB_spec_compare (C1, C2) ;

            % no mask, transpose
            C1 = GB_spec_select (Cin, [], [], op, B, k, dt) ;
            C2 = GB_mex_select  (Cin, [], [], op, B, k, dt) ;
            GB_spec_compare (C1, C2) ;

            % no mask, with accum, transpose
            C1 = GB_spec_select (Cin, [], 'plus', op, B, k, dt) ;
            C2 = GB_mex_select  (Cin, [], 'plus', op, B, k, dt) ;
            GB_spec_compare (C1, C2) ;

            % with mask, transpose
            C1 = GB_spec_select (Cin, Mask, [], op, B, k, dt) ;
            C2 = GB_mex_select  (Cin, Mask, [], op, B, k, dt) ;
            GB_spec_compare (C1, C2) ;

            % with mask and accum, transpose
            C1 = GB_spec_select (Cin, Mask, 'plus', op, B, k, dt) ;
            C2 = GB_mex_select  (Cin, Mask, 'plus', op, B, k, dt) ;
            GB_spec_compare (C1, C2) ;

        end
    end
    fprintf ('\n') ;
end
fprintf ('\ntest25: all tests passed\n') ;


