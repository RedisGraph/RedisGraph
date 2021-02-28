function C = uminus (G)
%UMINUS negate a GraphBLAS sparse matrix.
% C = -G negates the entries of a GraphBLAS matrix.
%
% See also GrB.apply, GrB/minus, GrB/uplus.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = GrB.apply ('-', G) ;

