function C = minus (A, B)
%MINUS sparse matrix subtraction, C = A-B.
% C = A-B subtracts the two matrices A and B.  If A and B are matrices,
% the pattern of C is the set union of A and B.  If one of A or B is a
% scalar, the scalar is expanded into a dense matrix the size of the
% other matrix, and the result is a dense matrix.  If the type of A and B
% differ, the type of A is used, as: C = A - GrB (B, GrB.type (A)).
%
% The input matrices may be either GraphBLAS and/or MATLAB matrices, in
% any combination.  C is returned as a GraphBLAS matrix.
%
% See also GrB.eadd, plus, uminus.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = A + (-B) ;

