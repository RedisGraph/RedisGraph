function C = uint16 (G)
%UINT16 cast a GraphBLAS matrix to MATLAB full uint16 matrix.
% C = uint16 (G) typecasts the GrB matrix G to a MATLAB full uint16
% matrix.  The result C is full since MATLAB does not support sparse
% uint16 matrices.
%
% To typecast the matrix G to a GraphBLAS sparse uint16 matrix instead,
% use C = GrB (G, 'uint16').
%
% See also GrB, double, complex, single, logical, int8, int16, int32,
% int64, uint8, uint32, and uint64.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = gbfull (G.opaque, 'uint16') ;

