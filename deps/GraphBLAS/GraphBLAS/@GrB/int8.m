function C = int8 (G)
%INT8 cast a GraphBLAS matrix to MATLAB full int8 matrix.
% C = int8 (G) typecasts the GrB matrix G to a MATLAB full int8 matrix.
% The result C is full since MATLAB does not support sparse int8
% matrices.
%
% To typecast the matrix G to a GraphBLAS sparse int8 matrix instead, use
% C = GrB (G, 'int8').
%
% See also GrB, double, complex, single, logical, int8, int16, int32,
% int64, uint8, uint16, uint32, and uint64.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = gbfull (G.opaque, 'int8') ;

