function C = int16 (G)
%INT16 cast a GraphBLAS matrix to MATLAB full int16 matrix.
% C = int16 (G) typecasts the GrB matrix G to a MATLAB full int16 matrix.
% The result C is full since MATLAB does not support sparse int16
% matrices.
%
% To typecast the matrix G to a GraphBLAS sparse int16 matrix instead,
% use C = GrB (G, 'int16').
%
% See also GrB, double, complex, single, logical, int8, int32, int64,
% uint8, uint16, uint32, and uint64.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = gbfull (G.opaque, 'int16') ;

