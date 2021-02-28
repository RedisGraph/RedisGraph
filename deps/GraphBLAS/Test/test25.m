function test25
%TEST25 test GxB_select

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\ntest25: GxB_select tests\n') ;

[~, ~, ~, classes, ~, select_ops] = GB_spec_opsall ;

rng ('default') ;

m = 10 ;
n = 6 ;
dt = struct ('inp0', 'tran') ;

for k1 = 1:length(classes)
    aclass = classes {k1} ;
    fprintf ('%s: ', aclass) ;

    for A_is_hyper = 0:1
    for A_is_csc   = 0:1
    for C_is_hyper = 0:1
    for C_is_csc   = 0:1
    for M_is_hyper = 0:1
    for M_is_csc   = 0:1

    if (A_is_hyper)
        ha = 1 ;
    else
        ha = 0 ;
    end

    if (C_is_hyper)
        hc = 1 ;
    else
        hc = 0 ;
    end

    if (M_is_hyper)
        hm = 1 ;
    else
        hm = 0 ;
    end

    A = GB_spec_random (m, n, 0.3, 100, aclass, A_is_csc, A_is_hyper, ha) ;
    A.matrix (:,1) = rand (m,1) ;
    A.pattern (:,1) = true (m,1) ;
    Cin = GB_spec_random (m, n, 0.3, 100, aclass, C_is_csc, C_is_hyper, hc) ;
    B = GB_spec_random (n, m, 0.3, 100, aclass, A_is_csc, A_is_hyper, ha) ;
    cin = cast (0, aclass) ;
    % Mask = (sprand (m, n, 0.5) ~= 0) ;
    Mask = GB_random_mask (m, n, 0.5, M_is_csc, M_is_hyper) ;
    Mask.hyper_ratio = hm ;

    fprintf ('.') ;

    for k2 = 1:length(select_ops)
        op = select_ops {k2} ;
        % fprintf ('%s ', op) ;

        for k = -m:3:n % Was: [-m:n]

            % no mask
            C1 = GB_spec_select (Cin, [], [], op, A, k, []) ;
            C2 = GB_mex_select  (Cin, [], [], op, A, k, [], 'test') ;
            GB_spec_compare (C1, C2) ;

            % no mask, with accum
            C1 = GB_spec_select (Cin, [], 'plus', op, A, k, []) ;
            C2 = GB_mex_select  (Cin, [], 'plus', op, A, k, [], 'test') ;
            GB_spec_compare (C1, C2) ;

            % with mask
            C1 = GB_spec_select (Cin, Mask, [], op, A, k, []) ;
            C2 = GB_mex_select  (Cin, Mask, [], op, A, k, [], 'test') ;
            GB_spec_compare (C1, C2) ;

            % with mask and accum
            C1 = GB_spec_select (Cin, Mask, 'plus', op, A, k, []) ;
            C2 = GB_mex_select  (Cin, Mask, 'plus', op, A, k, [], 'test') ;
            GB_spec_compare (C1, C2) ;

            % no mask, transpose
            C1 = GB_spec_select (Cin, [], [], op, B, k, dt) ;
            C2 = GB_mex_select  (Cin, [], [], op, B, k, dt, 'test') ;
            GB_spec_compare (C1, C2) ;

            % no mask, with accum, transpose
            C1 = GB_spec_select (Cin, [], 'plus', op, B, k, dt) ;
            C2 = GB_mex_select  (Cin, [], 'plus', op, B, k, dt, 'test') ;
            GB_spec_compare (C1, C2) ;

            % with mask, transpose
            C1 = GB_spec_select (Cin, Mask, [], op, B, k, dt) ;
            C2 = GB_mex_select  (Cin, Mask, [], op, B, k, dt, 'test') ;
            GB_spec_compare (C1, C2) ;

            % with mask and accum, transpose
            C1 = GB_spec_select (Cin, Mask, 'plus', op, B, k, dt) ;
            C2 = GB_mex_select  (Cin, Mask, 'plus', op, B, k, dt, 'test') ;
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
fprintf ('\ntest25: all tests passed\n') ;


