function C = mtimes (A, B)
%MTIMES sparse matrix-matrix multiplication over the standard semiring.
% C=A*B multiples two matrices using the standard '+.*' semiring, If the
% type of A and B differ, the type of A is used.  That is, C=A*B is the
% same as C = GrB.mxm (['+.*' GrB.type(A)], A, B).  If either A or B are
% scalars, C=A*B is the same as C=A.*B.
%
% The input matrices may be either GraphBLAS and/or MATLAB matrices, in
% any combination.  C is returned as a GraphBLAS matrix.
%
% See also GrB.mxm, GrB.emult, times.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (isscalar (A) || isscalar (B))
    C = A .* B ;
else
    C = GrB.mxm (A, '+.*', B) ;
end

