function test14
%TEST14 test GrB_reduce

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\nreduce to column and scalar\n') ;

[mult_ops unary_ops add_ops classes semirings] = GB_spec_opsall ;

rng ('default') ;

m = 8 ;
n = 4 ;
dt = struct ('inp0', 'tran') ;

for k1 = 1:length(classes)
    aclass = classes {k1} ;
    fprintf ('.') ;
    A = GB_spec_random (m, n, 0.3, 100, aclass) ;
    B = GB_spec_random (n, m, 0.3, 100, aclass) ;
    w = GB_spec_random (m, 1, 0.3, 100, aclass) ;
    cin = cast (0, aclass) ;
    mask = GB_random_mask (m, 1, 0.5, true, false) ;

    if (isequal (aclass, 'logical'))
        ops = {'or', 'and', 'xor', 'eq'} ;
    else
        ops = {'min', 'max', 'plus', 'times'} ;
    end

    if (isequal (aclass, 'double'))
        hrange = [0 1] ;
        crange = [0 1] ;
    else
        hrange = 0 ;
        crange = 1 ;
    end

    for A_is_hyper = 0:1
    for A_is_csc   = 0:1

    A.is_csc    = A_is_csc ; A.is_hyper    = A_is_hyper ;
    B.is_csc    = A_is_csc ; B.is_hyper    = A_is_hyper ;

    for k2 = 1:length(ops)
        op = ops {k2} ;

        % no mask
        w1 = GB_spec_reduce_to_vector (w, [], [], op, A, []) ;
        w2 = GB_mex_reduce_to_vector  (w, [], [], op, A, []) ;
        GB_spec_compare (w1, w2) ;

        % no mask, with accum
        w1 = GB_spec_reduce_to_vector (w, [], 'plus', op, A, []) ;
        w2 = GB_mex_reduce_to_vector  (w, [], 'plus', op, A, []) ;
        GB_spec_compare (w1, w2) ;

        % with mask
        w1 = GB_spec_reduce_to_vector (w, mask, [], op, A, []) ;
        w2 = GB_mex_reduce_to_vector  (w, mask, [], op, A, []) ;
        GB_spec_compare (w1, w2) ;

        % with mask and accum
        w1 = GB_spec_reduce_to_vector (w, mask, 'plus', op, A, []) ;
        w2 = GB_mex_reduce_to_vector  (w, mask, 'plus', op, A, []) ;
        GB_spec_compare (w1, w2) ;

        % no mask, transpose
        w1 = GB_spec_reduce_to_vector (w, [], [], op, B, dt) ;
        w2 = GB_mex_reduce_to_vector  (w, [], [], op, B, dt) ;
        GB_spec_compare (w1, w2) ;

        % no mask, with accum, transpose
        w1 = GB_spec_reduce_to_vector (w, [], 'plus', op, B, dt) ;
        w2 = GB_mex_reduce_to_vector  (w, [], 'plus', op, B, dt) ;
        GB_spec_compare (w1, w2) ;

        % with mask, transpose
        w1 = GB_spec_reduce_to_vector (w, mask, [], op, B, dt) ;
        w2 = GB_mex_reduce_to_vector  (w, mask, [], op, B, dt) ;
        GB_spec_compare (w1, w2) ;

        % with mask and accum, transpose
        w1 = GB_spec_reduce_to_vector (w, mask, 'plus', op, B, dt) ;
        w2 = GB_mex_reduce_to_vector  (w, mask, 'plus', op, B, dt) ;
        GB_spec_compare (w1, w2) ;

        % GB_spec_reduce_to_scalar always operates column-wise,
        % but GrB_reduce operates in whatever order it is given:
        % by column if CSC or by row if CSR.  The result can vary
        % slightly because of different round off errors.  The
        % alternative would be to compare c1 and c2 within round-off error.

        A_hack = A ;
        if (~A.is_csc && ...
            (isequal (aclass, 'single') || isequal (aclass, 'double')))
            A_hack.matrix = A_hack.matrix' ;
            A_hack.pattern = A_hack.pattern' ;
            A_hack.is_csc = true ;
        end

        % to scalar
        c1 = GB_spec_reduce_to_scalar (cin, [ ], op, A_hack) ;
        c2 = GB_mex_reduce_to_scalar  (cin, [ ], op, A) ;
        assert (isequal (c1, c2)) ;

        % to scalar, with accum
        c1 = GB_spec_reduce_to_scalar (cin, 'plus', op, A_hack) ;
        c2 = GB_mex_reduce_to_scalar  (cin, 'plus', op, A) ;
        assert (isequal (c1, c2)) ;

    end
    end
    end
end

fprintf ('\ntest14: all tests passed\n') ;

