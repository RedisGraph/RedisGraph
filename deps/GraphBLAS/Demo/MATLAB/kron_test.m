%KRON_TEST test kron_demo.m

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

clear
A = sprand (10, 20, 0.1) ;
A (10,20) = 42 ;
B = sprand (5,  4, 0.1) ;
B (5,4) = 9 ;
[C err] = kron_demo (A,B) ;
fprintf ('Kron demo passed, err: %g\n', err) ;

