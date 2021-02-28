function C = spfun (fun, G)
%SPFUN Apply function to the entries of a GraphBLAS matrix.
% C = spfun (fun, G) evaluates the function fun on the entries of G.
%
% See also GrB.apply.

% FUTURE: this would be much faster as a mexFunction, but calling feval
% from inside a mexFunction would not be trivial (perhaps not possible).

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[m, n] = size (G) ;
desc.base = 'zero-based' ;
[i, j, x] = GrB.extracttuples (G, desc) ;
x = feval (fun, x) ;
C = GrB.build (i, j, x, m, n, '1st', GrB.type (x), desc) ;

