function test39
%TEST39 test GrB_transpose performance

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n performance tests : GrB_transpose \n') ;
rng ('default') ;

Prob = ssget (939)
A = Prob.A ;
[m n] = size (A) ;
Cin = sprandn (n, m, 0.000001) ;
A (1,2) =1 ;

fprintf ('\n===============================================================n') ;
fprintf ('\nC = Cin + A''\n') ;
tic
C1 = Cin + A' ;
toc

% C = Cin + A'
tic
C = GB_mex_transpose (Cin, [ ], 'plus', A) ;
toc
assert (isequal (C1, C.matrix)) ;

fprintf ('\n===============================================================n') ;
fprintf ('\nC = (single) A\n') ;
clear Cin
Cin.matrix = sparse (n, m) ;
Cin.class = 'single'

tic
C1 = A' ;
toc

% C = A'
tic
C2 = GB_mex_transpose (Cin, [ ], '', A) ;
toc

[I1, J1, X1] = find (C1) ;
[I2, J2, X2] = find (C2.matrix) ;
clear C2

assert (isequal (I1, I2)) ;
assert (isequal (J1, J2)) ;
assert (isequal (single(X1), X2)) ;

fprintf ('\n===============================================================n') ;
fprintf ('\nC = A + B\n') ;

B = sprandn (m, n, 0.00001) ;

tic
C1 = A + B ;
toc

D = struct ('inp0', 'tran') ;

tic
C2 = GB_mex_transpose (A, [ ], 'plus', B, D) ;
toc
assert (isequal (C1, C2.matrix)) ;

tic
C2 = GB_mex_transpose (B, [ ], 'plus', A, D) ;
toc
assert (isequal (C1, C2.matrix)) ;

clear Cin
Cin = sparse (m,n) ;
tic
C3 = GB_mex_eWiseAdd_Matrix (Cin, [ ], '', 'plus', A, B) ;
toc
assert (isequal (C1, C3.matrix)) ;

tic
C4 = GB_mex_AplusB (A, B, 'plus') ;
toc
assert (isequal (C1, C4)) ;

tic
C4 = GB_mex_AplusB (B, A, 'plus') ;
toc
assert (isequal (C1, C4)) ;

fprintf ('\n===============================================================n') ;
fprintf ('\nC = Cin + A + B\n') ;

Cin = sprandn (m, n, 0.0001) ;

tic
C1 = Cin + A + B ;
toc

tic
C1 = (Cin + A) + B ;
toc

tic
C1 = Cin + (A + B) ;
toc

tic
C1 = (Cin + B) + A ;
toc

tic
C3 = GB_mex_eWiseAdd_Matrix (Cin, [ ], 'plus', 'plus', A, B) ;
toc

assert (isequal (C1, C3.matrix)) ;

tic
C4 = GB_mex_AplusB (A, B, 'plus') ;
C4 = GB_mex_AplusB (C4, Cin, 'plus') ;
toc
assert (isequal (C1, C4)) 

tic
C4 = GB_mex_AplusB (Cin, A, 'plus') ;
C4 = GB_mex_AplusB (C4, B, 'plus') ;
toc
assert (isequal (C1, C4)) ;;

tic
C4 = GB_mex_AplusB (Cin, B, 'plus') ;
C4 = GB_mex_AplusB (C4, A, 'plus') ;
toc
assert (isequal (C1, C4)) ;;

fprintf ('\ntest39: all tests passed\n') ;

