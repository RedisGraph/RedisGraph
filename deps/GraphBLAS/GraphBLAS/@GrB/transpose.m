function C = transpose (G)
%TRANSPOSE C = G.', array transpose of a GraphBLAS matrix.
%
% See also GrB.trans, ctranspose.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = GrB.trans (G) ;

