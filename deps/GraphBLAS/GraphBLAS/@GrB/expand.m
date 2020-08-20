function C = expand (scalar, S)
%GRB.EXPAND expand a scalar into a GraphBLAS matrix.
% C = GrB.expand (scalar, S) expands the scalar into a matrix with the
% same size and pattern as S, as C = scalar*spones(S).  C has the same
% type as the scalar.  The numerical values of S are ignored; only the
% pattern of S is used.  The inputs may be either GraphBLAS and/or
% MATLAB matrices/scalars, in any combination.  C is returned as a
% GraphBLAS matrix.
%
% See also GrB.assign.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% FUTURE: as much as possible, replace scalar expansion with binary operators
% used in a unary apply, when it becomes part of the C API.

[m, n] = size (S) ;
desc.mask = 'structure' ;
C = GrB.assign (GrB (m, n, GrB.type (S)), S, scalar, desc) ;

