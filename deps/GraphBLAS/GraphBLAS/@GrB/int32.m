function C = int32 (G)
%INT32 cast a GraphBLAS matrix to MATLAB full int32 matrix.
% C = int32 (G) typecasts the GrB matrix G to a MATLAB full int32 matrix.
% The result C is full since MATLAB does not support sparse int32
% matrices.
%
% To typecast the matrix G to a GraphBLAS sparse int32 matrix instead,
% use C = GrB (G, 'int32').
%
% See also GrB, double, complex, single, logical, int8, int16, int32,
% int64, uint8, uint16, uint32, and uint64.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = gbfull (G.opaque, 'int32') ;

