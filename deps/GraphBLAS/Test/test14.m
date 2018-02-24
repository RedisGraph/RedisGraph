function test14
%TEST14 test GrB_reduce

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
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
    mask = (sprand (m, 1, 0.5) ~= 0) ;

    if (isequal (aclass, 'logical'))
        ops = {'or', 'and', 'xor', 'eq'} ;
    else
        ops = {'min', 'max', 'plus', 'times'} ;
    end

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

        % to scalar
        c1 = GB_spec_reduce_to_scalar (cin, [ ], op, A) ;
        c2 = GB_mex_reduce_to_scalar  (cin, [ ], op, A) ;
        assert (isequal (c1, c2)) ;

        % to scalar, with accum
        c1 = GB_spec_reduce_to_scalar (cin, 'plus', op, A) ;
        c2 = GB_mex_reduce_to_scalar  (cin, 'plus', op, A) ;
        assert (isequal (c1, c2)) ;

    end
end

fprintf ('\ntest14: all tests passed\n') ;

