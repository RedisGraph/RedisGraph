function testc6
%TESTC6 test complex apply

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng 'default'

[complex_binary complex_unary] = GB_user_opsall ;

dr  = struct ('outp', 'replace') ;
dtr = struct ('outp', 'replace', 'inp0', 'tran') ;

seed = 1 ;
for m = [1 5 10 50 100 ]
    for n = [ 1 5 10  50 100 ]

        seed = seed + 1 ;
        A = GB_mex_random (m, n, 10*(m+n), 1, seed) ;
        D = GB_mex_random (n, m, 10*(m+n), 1, seed) ;
        seed = seed + 1 ;
        C = GB_mex_random (m, n, 10*(m+n), 1, seed) ;
        seed = seed + 1 ;
        B = GB_mex_random (m, n, 10*(m+n), 0, seed) ;
        E = GB_mex_random (n, m, 10*(m+n), 0, seed) ;
        a = pi + 1i ;
        b = 42 ;

        % test unary ops with complex x,z
        for k = 1:7
            op = complex_unary {k} ;
            C1 = GB_mex_op (op, a, '',1) ;
            [C2 tol] = GB_user_op (op, a) ;
            GB_user_compare (C1, C2, tol) ;
            C1 = GB_mex_apply (C, [], [], op, A, dr) ;
            [i j x1] = find (C1.matrix) ;
            x1 = complex (x1) ;
            [i j s] = find (A) ;
            x2 = GB_user_op (op, complex (s)) ;
            x2 = complex (x2) ;
            GB_user_compare (x1, x2, tol) ;
        end

        % test unary ops with complex x,z, array transposed
        for k = 1:7
            op = complex_unary {k} ;
            C1 = GB_mex_apply (C, [], [], op, D, dtr) ;
            [i j x1] = find (C1.matrix) ;
            x1 = complex (x1) ;
            [i j s] = find (D.') ;
            x2 = GB_user_op (op, complex (s)) ;
            x2 = complex (x2) ;
            GB_user_compare (x1, x2, true) ;
        end

        % test unary ops with complex x, real z
        for k = 8:11
            op = complex_unary {k} ;
            C1 = GB_mex_op (op, a, '',1) ;
            [C2 tol] = GB_user_op (op, a) ;
            GB_user_compare (C1, C2, tol) ;
            C1 = GB_mex_apply (B, [], [], op, A, dr) ;
            [i j x1] = find (C1.matrix) ;
            x1 = complex (x1) ;
            [i j s] = find (A) ;
            x2 = GB_user_op (op, complex (s)) ;
            x2 = complex (x2) ;
            GB_user_compare (x1, x2, tol) ;
        end

        % test unary ops with complex x, real z, array transposed
        for k = 8:11
            op = complex_unary {k} ;
            C1 = GB_mex_apply (B, [], [], op, D, dtr) ;
            [i j x1] = find (C1.matrix) ;
            x1 = complex (x1) ;
            [i j s] = find (D.') ;
            x2 = GB_user_op (op, complex (s)) ;
            x2 = complex (x2) ;
            GB_user_compare (x1, x2, true) ;
        end

        % test unary ops with real x, complex z
        for k = 12:13
            op = complex_unary {k} ;
            C1 = GB_mex_op (op, b, '',1) ;
            [C2 tol] = GB_user_op (op, b) ;
            GB_user_compare (C1, C2, tol) ;
            C1 = GB_mex_apply (C, [], [], op, B, dr) ;
            [i j x1] = find (C1.matrix) ;
            x1 = complex (x1) ;
            [i j s] = find (B) ;
            x2 = GB_user_op (op, s) ;
            x2 = complex (x2) ;
            GB_user_compare (x1, x2, tol) ;
        end

        % test unary ops with real x, complex z, array transposed
        for k = 12:13
            C1 = GB_mex_apply (C, [], [], op, E, dtr) ;
            [i j x1] = find (C1.matrix) ;
            x1 = complex (x1) ;
            [i j s] = find (E.') ;
            x2 = GB_user_op (op, s) ;
            x2 = complex (x2) ;
            GB_user_compare (x1, x2, true) ;
        end
    end
end

fprintf ('testc6: all complex apply C<Mask>=op(A) tests passed\n') ;

