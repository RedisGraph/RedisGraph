function C = uint8 (G)
%UINT8 cast a GraphBLAS matrix to MATLAB full uint8 matrix.
% C = uint8 (G) typecasts the GrB matrix G to a MATLAB full uint8 matrix.
% The result C is full since MATLAB does not support sparse uint8
% matrices.
%
% To typecast the matrix G to a GraphBLAS sparse uint8 matrix instead,
% use C = GrB (G, 'uint8').
%
% See also GrB, double, complex, single, logical, int8, int16, int32,
% int64, uint16, uint32, and uint64.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = gbfull (G.opaque, 'uint8') ;

