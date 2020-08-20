function C = uint64 (G)
%UINT64 cast a GraphBLAS matrix to MATLAB full uint64 matrix.
% C = uint64 (G) typecasts the GrB matrix G to a MATLAB full uint64
% matrix.  The result C is full since MATLAB does not support sparse
% uint64 matrices.
%
% To typecast the matrix G to a GraphBLAS sparse uint64 matrix instead,
% use C = GrB (G, 'uint64').
%
% See also GrB, double, complex, single, logical, int8, int16, int32,
% int64, uint8, uint16, and uint32.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = gbfull (G.opaque, 'uint64') ;

