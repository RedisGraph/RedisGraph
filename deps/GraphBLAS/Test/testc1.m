function testc1
%TESTC1 test complex operators

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng 'default'

a = GB_mex_op ('min', complex(1), 1i, 1)  ;
b = min (complex (1), 1i)  ;
assert (isequal (a, complex (b))) ;

a = GB_mex_op ('max', complex(1), -1i, 1)  ;
b = max (complex (1), -1i)  ;
assert (isequal (a, b)) ;

A = sparse (rand (2) + 1i * rand (2))  ;

C = GB_mex_dump (A,0) ;

B = sparse (rand (2) + 1i * rand (2))  ;

A = full (A) ;
B = full (B) ;

C1 = GB_mex_op ('plus', A, B, 1) ;
C2 = A+B ;

assert (isequal (C1,C2)) ;

E = rand (2) ;
F = rand (2) ;

C1 = GB_mex_op ('complex', E, F, 1) ;
C2 = complex (E,F) ;
assert (isequal (C1,C2)) ;

[complex_binary complex_unary] = GB_user_opsall ;

A (2,1) = B (2,1) ;

% create some complex test matrices

for m = [1 5 10 50 100 ]
    for n = [ 1 5 10  50 100 ]

        for akind = 1:5
            switch (akind)
                case 1
                    A = complex (zeros (m,n), 0) ;
                case 2
                    A = complex (ones (m,n), 0) ;
                case 3
                    A = complex (-ones (m,n), ones(m,n)) ;
                case 4
                    x = full (sprand(m,n,0.3)) ;
                    y = full (sprand(m,n,0.3)) ;
                    A = complex (x,y) ;
                case 5
                    A = complex (rand (m,n), rand (m,n)) ;
                end

            % test unary ops with complex x
            for k = 1:11
                op = complex_unary {k} ;
                C1 = GB_mex_op (op, A, '',1) ;
                [C2 tol] = GB_user_op (op, A) ;
                GB_user_compare (C1, C2, tol) ;
            end

            % test unary ops with real x
            for k = 12:13
                op = complex_unary {k} ;
                C1 = GB_mex_op (op, real (A), '',1) ;
                [C2 tol] = GB_user_op (op, real (A)) ;
                GB_user_compare (C1, C2, tol) ;
            end

            for bkind = 1:6
                switch (bkind)
                    case 1
                        B = complex (zeros (m,n), 0) ;
                    case 2
                        B = complex (ones (m,n), 0) ;
                    case 3
                        B = complex (-ones (m,n), ones(m,n)) ;
                    case 4
                        x = full (sprand(m,n,0.3)) ;
                        y = full (sprand(m,n,0.3)) ;
                        B = complex (x,y) ;
                    case 5
                        B = complex (rand (m,n), rand (m,n)) ;
                    case 6
                        B = A ;
                    end

                % test all but the last one, 'complex', which requires
                % x,y real
                for k = 1:length (complex_binary)-1
                    op = complex_binary {k} ;
                    C1 = GB_mex_op (op, A, B, 1) ;
                    [C2 tol] = GB_user_op (op, A, B) ;
                    GB_user_compare (C1, C2, tol) ;
                end

                % test complex(A,B)
                C1 = GB_mex_op  ('complex', real (A), real (B), 1) ;
                C2 = GB_user_op ('complex', real (A), real (B)) ;
                assert (isequal (C1, C2)) 

            end
        end
    end
end

fprintf ('testc1: all complex operator tests passed\n') ;

