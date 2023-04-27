function gbtest44
%GBTEST44 test subsasgn, mtimes, plus, false, ...

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

rng ('default') ;

A = rand (4,1) ;
G = GrB (A) ;
G (1) = pi ;
A (1) = pi ;
assert (isequal (A, G)) ;

A = pi ;
G = GrB (pi) ;
C = A*G ;
assert (isequal (C, pi^2))

A = pi ;
B = rand (2) ;
G = GrB (B) ;
A
B
G
A*B
A*G
assert (isequal (A*B, A*G))
assert (isequal (A.*B, A.*G))
assert (isequal (B.*A, G.*A))

C = A + B ;
H = A + G ;
assert (isequal (C, H))

G = false ([3 4], 'like', H) ;
C = sparse (false ([3 4])) ;
assert (isequal (C, G))

G = false (3, 4, 'like', H) ;
C = sparse (false (3, 4)) ;
assert (isequal (C, G))

G = true ([3 4], 'like', H) ;
C = sparse (true ([3 4])) ;
assert (isequal (C, G))

G = true (3, 4, 'like', H) ;
C = sparse (true (3, 4)) ;
assert (isequal (C, G))

fprintf ('gbtest44: all tests passed\n') ;

